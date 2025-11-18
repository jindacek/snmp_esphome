#include "snmp_packet.h"
#include <string.h>

// Jednoduchý parser OID stringu "1.3.6.1.4.1.318..." na BER OID
static int encode_oid(const char *oid_str, uint8_t *out, int max_len) {
  int nums[20];
  int count = 0;
  const char *p = oid_str;

  while (*p && count < 20) {
    // přeskoč tečky
    if (*p == '.') {
      p++;
      continue;
    }
    int v = 0;
    bool has = false;
    while (*p >= '0' && *p <= '9') {
      has = true;
      v = v * 10 + (*p - '0');
      p++;
    }
    if (has) {
      nums[count++] = v;
    } else {
      p++;
    }
  }

  if (count < 2) return -1;

  int pos = 0;
  if (max_len <= 0) return -1;

  // první dva: 40*X + Y
  out[pos++] = (uint8_t)(40 * nums[0] + nums[1]);

  for (int i = 2; i < count && pos < max_len; i++) {
    int v = nums[i];
    // pro naše OID předpokládáme < 128
    if (v < 128) {
      out[pos++] = (uint8_t)v;
    } else {
      // jednoduché kódování base-128 (max 2 bajty)
      uint8_t tmp[5];
      int tpos = 0;
      while (v > 0 && tpos < 5) {
        tmp[tpos++] = (uint8_t)(v & 0x7F);
        v >>= 7;
      }
      for (int j = tpos - 1; j >= 0; j--) {
        uint8_t b = tmp[j];
        if (j != 0) b |= 0x80;
        out[pos++] = b;
        if (pos >= max_len) break;
      }
    }
  }

  return pos;
}

// pomocná funkce - zapíše INTEGER (kladný)
static int encode_integer(uint8_t *buf, long value) {
  uint8_t tmp[5];
  int len = 0;
  long v = value;
  if (v == 0) {
    buf[0] = 0;
    return 1;
  }
  while (v > 0 && len < 5) {
    tmp[len++] = (uint8_t)(v & 0xFF);
    v >>= 8;
  }
  // reverse
  for (int i = 0; i < len; i++) {
    buf[i] = tmp[len - 1 - i];
  }
  // pokud je nejvyšší bit 1, přidej 0x00
  if (buf[0] & 0x80) {
    // posuň
    for (int i = len; i > 0; i--) buf[i] = buf[i - 1];
    buf[0] = 0;
    len++;
  }
  return len;
}

int snmp_build_get(uint8_t *buf, int buf_size,
                   const char *community,
                   const char *oid,
                   long request_id) {
  if (buf_size < 100) return -1;

  uint8_t oid_bytes[64];
  int oid_len = encode_oid(oid, oid_bytes, sizeof(oid_bytes));
  if (oid_len <= 0) return -1;

  int pos = 0;

  // Rezervujeme místo: budeme stavět "zevnitř ven"
  // VarBind inner: name (OID) + value (NULL)
  uint8_t vb_inner[96];
  int vi = 0;

  // name: OBJECT IDENTIFIER (0x06)
  vb_inner[vi++] = 0x06;
  vb_inner[vi++] = (uint8_t)oid_len;
  memcpy(&vb_inner[vi], oid_bytes, oid_len);
  vi += oid_len;

  // value: NULL (0x05, length 0)
  vb_inner[vi++] = 0x05;
  vb_inner[vi++] = 0x00;

  // VarBind SEQUENCE (0x30)
  uint8_t varbind[128];
  int vb = 0;
  varbind[vb++] = 0x30;
  varbind[vb++] = (uint8_t)vi;
  memcpy(&varbind[vb], vb_inner, vi);
  vb += vi;

  // VarBindList SEQUENCE OF VarBind
  uint8_t vblist[160];
  int vl = 0;
  vblist[vl++] = 0x30;
  vblist[vl++] = (uint8_t)vb;
  memcpy(&vblist[vl], varbind, vb);
  vl += vb;

  // PDU (GetRequest-PDU tag 0xA0)
  uint8_t pdu_inner[192];
  int pi = 0;

  // request-id INTEGER
  {
    uint8_t intbuf[8];
    int ilen = encode_integer(intbuf, request_id);
    pdu_inner[pi++] = 0x02;
    pdu_inner[pi++] = (uint8_t)ilen;
    memcpy(&pdu_inner[pi], intbuf, ilen);
    pi += ilen;
  }
  // error-status INTEGER 0
  pdu_inner[pi++] = 0x02;
  pdu_inner[pi++] = 0x01;
  pdu_inner[pi++] = 0x00;
  // error-index INTEGER 0
  pdu_inner[pi++] = 0x02;
  pdu_inner[pi++] = 0x01;
  pdu_inner[pi++] = 0x00;
  // variable-bindings
  pdu_inner[pi++] = 0x30;
  pdu_inner[pi++] = (uint8_t)vl;
  memcpy(&pdu_inner[pi], vblist, vl);
  pi += vl;

  uint8_t pdu[256];
  int p = 0;
  pdu[p++] = 0xA0;
  pdu[p++] = (uint8_t)pi;
  memcpy(&pdu[p], pdu_inner, pi);
  p += pi;

  // SNMP Message: SEQUENCE { version, community, pdu }
  uint8_t msg_inner[300];
  int mi = 0;

  // version INTEGER 0
  msg_inner[mi++] = 0x02;
  msg_inner[mi++] = 0x01;
  msg_inner[mi++] = 0x00;

  // community OCTET STRING
  size_t comm_len = strlen(community);
  if (comm_len > 60) comm_len = 60;
  msg_inner[mi++] = 0x04;
  msg_inner[mi++] = (uint8_t)comm_len;
  memcpy(&msg_inner[mi], community, comm_len);
  mi += comm_len;

  // data: PDU
  msg_inner[mi++] = 0xA0;
  msg_inner[mi++] = (uint8_t)p;
  memcpy(&msg_inner[mi], pdu, p);
  mi += p;

  // Outer SEQUENCE
  int total = 0;
  buf[total++] = 0x30;             // SEQUENCE
  buf[total++] = (uint8_t)mi;      // délka
  memcpy(&buf[total], msg_inner, mi);
  total += mi;

  return total;
}

bool snmp_parse_int_response(uint8_t *buf, int len, long *out_value) {
  if (len < 20) return false;
  int i = 0;

  if (buf[i++] != 0x30) return false;  // SEQUENCE
  int msg_len = buf[i++];
  if (msg_len + 2 > len) return false;

  // version
  if (buf[i++] != 0x02) return false;
  int vlen = buf[i++];
  i += vlen;  // přeskoč

  // community
  if (buf[i++] != 0x04) return false;
  int clen = buf[i++];
  i += clen;

  // PDU - GetResponse očekáváme tag 0xA2
  uint8_t pdu_tag = buf[i++];
  if (pdu_tag != 0xA2 && pdu_tag != 0xA0) return false;
  int pdu_len = buf[i++];
  int pdu_end = i + pdu_len;
  if (pdu_end > len) return false;

  // request-id
  if (buf[i++] != 0x02) return false;
  int rid_len = buf[i++];
  i += rid_len;

  // error-status
  if (buf[i++] != 0x02) return false;
  int es_len = buf[i++];
  long err = 0;
  for (int k = 0; k < es_len; k++) {
    err = (err << 8) | buf[i++];
  }
  if (err != 0) return false;

  // error-index
  if (buf[i++] != 0x02) return false;
  int ei_len = buf[i++];
  i += ei_len;

  // varbind list
  if (buf[i++] != 0x30) return false;
  int vbl_len = buf[i++];
  if (i + vbl_len > pdu_end) return false;

  // first varbind
  if (buf[i++] != 0x30) return false;
  int vb_len = buf[i++];
  int vb_end = i + vb_len;
  if (vb_end > pdu_end) return false;

  // OID
  if (buf[i++] != 0x06) return false;
  int oid_len = buf[i++];
  i += oid_len;
  if (i >= vb_end) return false;

  // value
  uint8_t vtag = buf[i++];
  int vlen = buf[i++];
  if (vtag != 0x02) {
    // není INTEGER
    return false;
  }
  if (i + vlen > vb_end) return false;

  long value = 0;
  for (int k = 0; k < vlen; k++) {
    value = (value << 8) | buf[i++];
  }

  *out_value = value;
  return true;
}
