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

  // první dva: 40*X + Y
  out[p++] = (uint8_t)(nums[0] * 40 + nums[1]);

  // zbytek v base-128
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

// ------------------------ SNMPv1 GET PDU -----------------------------

int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community,
                                      const char *oid_str) {
  if (buf_size < 64) return -1;

  uint8_t oid[64];
  int oid_len = encode_oid(oid_str, oid, sizeof(oid));
  if (oid_len <= 0) {
    ESP_LOGE(TAG, "encode_oid failed for %s", oid_str);
    return -1;
  }

  uint8_t *p = buf;

  // Outer SEQUENCE
  *p++ = 0x30;
  uint8_t *len_total = p++;

  // version INTEGER 0
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x00;

  // community
  size_t clen = strlen(community);
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

  // Přepočet délek
  *len_vb    = (uint8_t)(p - (len_vb + 1));
  *len_vbl   = (uint8_t)(p - (len_vbl + 1));
  *len_pdu   = (uint8_t)(p - (len_pdu + 1));
  *len_total = (uint8_t)(p - (len_total + 1));

  return (int)(p - buf);
}

// ------------------------ RESPONSE PARSER -----------------------------

bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  if (len < 20) {
    ESP_LOGW(TAG, "SNMP resp too short len=%d", len);
    // radši hned fallback o kus níž
  } else {
    bool ok = false;
    int i = 0;

    // Zkusíme STRIKTNÍ PARSER podle SNMPv1 struktury
    do {
      // 1) Outer SEQUENCE
      if (buf[i++] != 0x30) break;
      if (i >= len) break;
      int len_total = buf[i++];  // ignorujeme, jen posouváme

      // 2) version INTEGER
      if (i + 2 > len) break;
      if (buf[i++] != 0x02) break;
      int l = buf[i++];
      if (i + l > len) break;
      i += l;

      // 3) community OCTET STRING
      if (i + 2 > len) break;
      if (buf[i++] != 0x04) break;
      l = buf[i++];
      if (i + l > len) break;
      i += l;

      // 4) PDU (GetResponse = 0xA2, ale tolerujeme 0xA0–0xA5)
      if (i + 2 > len) break;
      uint8_t pdu_type = buf[i++];
      if ((pdu_type & 0xA0) != 0xA0) break;
      int pdu_len = buf[i++];  // jen posun

      // 5) request-id INTEGER
      if (i + 2 > len) break;
      if (buf[i++] != 0x02) break;
      l = buf[i++];
      if (i + l > len) break;
      i += l;

      // 6) error-status INTEGER
      if (i + 2 > len) break;
      if (buf[i++] != 0x02) break;
      l = buf[i++];
      if (i + l > len) break;
      i += l;

      // 7) error-index INTEGER
      if (i + 2 > len) break;
      if (buf[i++] != 0x02) break;
      l = buf[i++];
      if (i + l > len) break;
      i += l;

      // 8) VarBindList SEQUENCE
      if (i + 2 > len) break;
      if (buf[i++] != 0x30) break;
      int vbl_len = buf[i++];  // ignorujeme

      // 9) VarBind SEQUENCE
      if (i + 2 > len) break;
      if (buf[i++] != 0x30) break;
      int vb_len = buf[i++];  // ignorujeme

      // 10) OID
      if (i + 2 > len) break;
      if (buf[i++] != 0x06) break;
      int oid_len = buf[i++];
      if (i + oid_len > len) break;
      i += oid_len;

      // 11) VALUE
      if (i + 2 > len) break;
      uint8_t type = buf[i++];
      int vlen = buf[i++];

      if (i + vlen > len) break;

      if (type == 0x05) {
        // NULL – hodnota neexistuje (třeba ten stav baterie)
        ESP_LOGW(TAG, "SNMP value is NULL (ASN_NULL)");
        return false;
      }

      if (!(type == 0x02 || type == 0x41 || type == 0x42)) {
        ESP_LOGW(TAG, "Unsupported ASN.1 type 0x%02X", type);
        break;
      }

      if (vlen <= 0 || vlen > 4) break;

      long v = 0;
      for (int j = 0; j < vlen; j++) {
        v = (v << 8) | buf[i + j];
      }
      *value = v;
      return true;

    } while (false);

    // když jsme se dostali sem, striktní parser selhal
    ESP_LOGW(TAG, "Strict SNMP parse failed, falling back to simple scan");
  }

  // ---------- FALLBACK: původní jednoduchý scanner ----------
  if (len < 2) return false;

  for (int i = 0; i < len - 2; i++) {
    uint8_t t = buf[i];
    if (t == 0x02 || t == 0x41 || t == 0x42) {
      int l = buf[i + 1];
      if (l <= 0 || l > 4 || i + 2 + l > len) continue;

      long v = 0;
      for (int j = 0; j < l; j++) {
        v = (v << 8) | buf[i + 2 + j];
      }
      *value = v;
      return true;
    }
  }

  ESP_LOGW(TAG, "No numeric ASN.1 value found in SNMP response");
  return false;
}



// ------------------------ PUBLIC GET -----------------------------

bool SnmpClient::get(const char *host,
                     const char *community,
                     const char *oid,
                     long *value) {
  IPAddress ip;
  if (!ip.fromString(host)) {
    ESP_LOGE(TAG, "Invalid host: %s", host);
    return false;
  }

  uint8_t packet[300];
  int plen = build_snmp_get_packet(packet, sizeof(packet), community, oid);
  if (plen <= 0) {
    ESP_LOGE(TAG, "SNMP packet build failed");
    return false;
  }

  // SNMP server port je 161
  udp_.beginPacket(ip, 161);
  udp_.write(packet, plen);
  udp_.endPacket();

  uint32_t deadline = millis() + 500;

  while (millis() < deadline) {
    int size = udp_.parsePacket();
    if (size > 0) {
      uint8_t resp[300];
      int n = udp_.read(resp, sizeof(resp));
      if (n > 0) {
        bool ok = parse_snmp_response(resp, n, value);
        if (!ok) {
          ESP_LOGW(TAG, "Failed to parse SNMP response");
        }
        return ok;
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}
