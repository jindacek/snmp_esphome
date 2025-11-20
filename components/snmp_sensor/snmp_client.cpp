#include "snmp_client.h"
#include "esphome/core/log.h"
#include <cstring>

static const char *TAG = "snmp_client";

SnmpClient::SnmpClient() {}

bool SnmpClient::begin(uint16_t local_port) {
  if (!udp_.begin(local_port)) {
    ESP_LOGE(TAG, "UDP begin(%u) failed!", local_port);
    return false;
  }
  ESP_LOGI(TAG, "UDP client started on port %u", local_port);
  return true;
}

// ------------------------ OID ENCODE -----------------------------

// Převede "1.3.6.1.4.1.318.1.1.1.3.2.1.0" na BER OID
static int encode_oid(const char *oid_str, uint8_t *out, int max_len) {
  int nums[32];
  int count = 0;

  char buf[128];
  strncpy(buf, oid_str, sizeof(buf));
  buf[sizeof(buf) - 1] = 0;

  char *saveptr = nullptr;
  char *tok = strtok_r(buf, ".", &saveptr);
  while (tok && count < 32) {
    nums[count++] = atoi(tok);
    tok = strtok_r(nullptr, ".", &saveptr);
  }
  if (count < 2) return -1;

  int p = 0;
  if (p >= max_len) return -1;
  out[p++] = (uint8_t)(nums[0] * 40 + nums[1]);

  for (int i = 2; i < count; i++) {
    uint32_t v = (uint32_t) nums[i];
    uint8_t tmp[8];
    int tn = 0;

    do {
      tmp[tn++] = v & 0x7F;
      v >>= 7;
    } while (v && tn < 8);

    for (int j = tn - 1; j >= 0; j--) {
      uint8_t b = tmp[j];
      if (j != 0) b |= 0x80;
      if (p >= max_len) return -1;
      out[p++] = b;
    }
  }

  return p;
}

// ------------------------ SNMPv1 GET PDU – single OID -----------------------------

int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community,
                                      const char *oid_str) {
  const char *oids[1] = {oid_str};
  return build_snmp_get_packet_multi(buf, buf_size, community, oids, 1);
}

// ------------------------ SNMPv1 GET PDU – multi OID -----------------------------

int SnmpClient::build_snmp_get_packet_multi(uint8_t *buf, int buf_size,
                                            const char *community,
                                            const char **oids,
                                            int num_oids) {
  if (buf_size < 64 || num_oids <= 0) return -1;

  uint8_t *p = buf;

  // Outer SEQUENCE
  *p++ = 0x30;
  uint8_t *len_total = p++;

  // version INTEGER 0
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x00;

  // community
  size_t clen = strlen(community);
  if (clen > 60) return -1;  // hrubý limit
  *p++ = 0x04;
  *p++ = (uint8_t)clen;
  memcpy(p, community, clen);
  p += clen;

  // PDU: GetRequest = 0xA0
  *p++ = 0xA0;
  uint8_t *len_pdu = p++;

  // request-id INTEGER 1
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x01;

  // error-status
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x00;

  // error-index
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x00;

  // VarBind list (sequence)
  *p++ = 0x30;
  uint8_t *len_vbl = p++;

  uint8_t *vbl_start = p;
  (void)vbl_start;  // nepoužito, ale necháme to tu

  for (int idx = 0; idx < num_oids; idx++) {
    uint8_t oid[64];
    int oid_len = encode_oid(oids[idx], oid, sizeof(oid));
    if (oid_len <= 0) {
      ESP_LOGE(TAG, "encode_oid failed for %s", oids[idx]);
      return -1;
    }

    // VarBind
    *p++ = 0x30;
    uint8_t *len_vb = p++;

    // OID
    *p++ = 0x06;
    *p++ = (uint8_t) oid_len;
    memcpy(p, oid, oid_len);
    p += oid_len;

    // value = NULL
    *p++ = 0x05;
    *p++ = 0x00;

    *len_vb = (uint8_t)(p - (len_vb + 1));
  }

  *len_vbl   = (uint8_t)(p - (len_vbl + 1));
  *len_pdu   = (uint8_t)(p - (len_pdu + 1));
  *len_total = (uint8_t)(p - (len_total + 1));

  return (int)(p - buf);
}

// ------------------------ RESPONSE PARSER – single -----------------------------

bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  long tmp[1];
  bool ok = parse_snmp_response_multi(buf, len, tmp, 1);
  if (!ok) return false;
  *value = tmp[0];
  return true;
}

// ------------------------ RESPONSE PARSER – multi -----------------------------

bool SnmpClient::parse_snmp_response_multi(uint8_t *buf, int len,
                                           long *values,
                                           int num_oids) {
  if (len < 2 || num_oids <= 0) return false;

  int found = 0;

  // Jdeme odzadu – tím přeskočíme hlavičky (version, request-id, atd.)
  for (int i = len - 2; i >= 0 && found < num_oids; --i) {
    uint8_t t = buf[i];
    if (t == 0x02 || t == 0x41 || t == 0x42) {  // INTEGER, GAUGE, UNSIGNED
      int l = buf[i + 1];
      if (l <= 0 || l > 4 || i + 2 + l > len) continue;

      long v = 0;
      for (int j = 0; j < l; j++) {
        v = (v << 8) | buf[i + 2 + j];
      }

      // Plníme od konce → poslední nalezená hodnota = poslední OID
      int idx = num_oids - 1 - found;
      values[idx] = v;
      found++;
    }
  }

  if (found != num_oids) {
    ESP_LOGW(TAG, "Multi-parse: expected %d values, found %d",
             num_oids, found);
    return false;
  }

  return true;
}

// ------------------------ PUBLIC GET – single OID -----------------------------

bool SnmpClient::get(const char *host,
                     const char *community,
                     const char *oid,
                     long *value) {
  const char *oids[1] = {oid};
  long vals[1] = {0};

  bool ok = get_many(host, community, oids, 1, vals);
  if (!ok) return false;
  *value = vals[0];
  return true;
}

// ------------------------ PUBLIC GET – multi OID -----------------------------

bool SnmpClient::get_many(const char *host,
                          const char *community,
                          const char **oids,
                          int count,
                          long *values)
{
  // Reset output values
  for (int i = 0; i < count; i++) values[i] = -1;

  IPAddress ip;
  if (!ip.fromString(host)) return false;

  // --- Build REQUEST ---
  uint8_t packet[512];
  int plen = build_snmp_get_packet_multi(packet, sizeof(packet), community, oids, count);
  if (plen <= 0) return false;

  udp_.beginPacket(ip, 161);
  udp_.write(packet, plen);
  udp_.endPacket();

  uint32_t deadline = millis() + 500;
  uint8_t resp[512];

  while (millis() < deadline) {
    int size = udp_.parsePacket();
    if (!size) continue;

    int n = udp_.read(resp, sizeof(resp));
    if (n <= 0) continue;

    // --- MULTI PARSER ---
    int pos = 0;
    while (pos < n - 4) {

      // Hledáme OID tag (0x06)
      if (resp[pos] == 0x06) {
        int oid_len = resp[pos+1];
        const uint8_t *oid_ptr = &resp[pos+2];

        // VALUE TLV začíná hned za OID
        int val_tlv = pos + 2 + oid_len;
        // Skip NULL between OID and value
        if (resp[val_tlv] == 0x05 && resp[val_tlv+1] == 0x00) {
            val_tlv += 2;
        }
        uint8_t vtag = resp[val_tlv];
        uint8_t vlen = resp[val_tlv + 1];

        long val = 0;

        // Jen pro numeric ASN.1 types
        if (vtag == 0x02 || vtag == 0x41 || vtag == 0x42) {
          for (int j = 0; j < vlen; j++)
            val = (val << 8) | resp[val_tlv + 2 + j];
        }

        // Porovnání OID se seznamem
        for (int i = 0; i < count; i++) {
          uint8_t expected[64];
          int elen = encode_oid(oids[i], expected, sizeof(expected));

          if (elen == oid_len &&
              memcmp(expected, oid_ptr, oid_len) == 0)
          {
            values[i] = val;
          }
        }
      }

      pos++;
    }

    return true;
  }

  return false;
}


