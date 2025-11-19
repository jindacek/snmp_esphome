#include "snmp_client.h"
#include <esphome/core/log.h>
#include <cstring>

static const char *TAG = "snmp_client";

/***************************************************
 * Constructor
 ***************************************************/
SnmpClient::SnmpClient() {}

/***************************************************
 * Initialize UDP
 ***************************************************/
bool SnmpClient::begin(uint16_t local_port) {
  if (!udp_.begin(local_port)) {
    ESP_LOGE(TAG, "UDP begin(%d) FAILED", local_port);
    return false;
  }
  ESP_LOGI(TAG, "UDP client started on port %d", local_port);
  return true;
}

/***************************************************
 * Encode OID string ("1.3.6.1....") → BER
 ***************************************************/
static int encode_oid(const char *oid_str, uint8_t *out) {
  uint32_t p[32];
  int count = 0;

  const char *s = oid_str;
  while (*s && count < 32) {
    p[count++] = strtoul(s, (char **)&s, 10);
    if (*s == '.') s++;
  }

  if (count < 2) return -1;

  int o = 0;

  out[o++] = 40 * p[0] + p[1];

  for (int i = 2; i < count; i++) {
    uint32_t v = p[i];
    uint8_t tmp[8];
    int tn = 0;

    do {
      tmp[tn++] = v & 0x7F;
      v >>= 7;
    } while (v);

    for (int j = tn - 1; j >= 0; j--) {
      uint8_t b = tmp[j];
      if (j != 0) b |= 0x80;
      out[o++] = b;
    }
  }

  return o;
}

/***************************************************
 * Build valid RFC-compliant SNMPv1 GET packet
 ***************************************************/
int SnmpClient::build_snmp_get_packet(uint8_t *b, int max,
                                      const char *community,
                                      const char *oid_str) {
  uint8_t oid[64];
  int oid_len = encode_oid(oid_str, oid);
  if (oid_len <= 0) return -1;

  int p = 0;

  // ROOT sequence
  b[p++] = 0x30; // seq
  int seq_len_pos = p++;
  
  // version: INTEGER 0
  b[p++] = 0x02; b[p++] = 0x01; b[p++] = 0x00;

  // community
  int clen = strlen(community);
  b[p++] = 0x04; b[p++] = clen;
  memcpy(&b[p], community, clen);
  p += clen;

  // GET REQUEST PDU (0xA0)
  b[p++] = 0xA0;
  int pdu_len_pos = p++;

  // request-id: INTEGER 1
  b[p++] = 0x02; b[p++] = 0x01; b[p++] = 0x01;

  // error-status
  b[p++] = 0x02; b[p++] = 0x01; b[p++] = 0x00;

  // error-index
  b[p++] = 0x02; b[p++] = 0x01; b[p++] = 0x00;

  // VarBind list (sequence)
  b[p++] = 0x30;
  int vbl_len_pos = p++;

  // VarBind entry
  b[p++] = 0x30;
  int vb_len_pos = p++;

  // OID
  b[p++] = 0x06;
  b[p++] = oid_len;
  memcpy(&b[p], oid, oid_len);
  p += oid_len;

  // NULL value
  b[p++] = 0x05;
  b[p++] = 0x00;

  // Fix lengths
  b[vb_len_pos] = p - vb_len_pos - 1;
  b[vbl_len_pos] = p - vbl_len_pos - 1;
  b[pdu_len_pos] = p - pdu_len_pos - 1;
  b[seq_len_pos] = p - seq_len_pos - 1;

  return p;
}

/***************************************************
 * Parse SNMP response → long value
 ***************************************************/
bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  if (len < 2) return false;

  int p = 0;

  if (buf[p++] != 0x30) return false;
  p++;  // skip seq len

  // skip version INTEGER
  if (buf[p++] != 0x02) return false;
  int vlen = buf[p++];
  p += vlen;

  // skip community STRING
  if (buf[p++] != 0x04) return false;
  int clen = buf[p++];
  p += clen;

  // response-PDU is 0xA2
  if (buf[p++] != 0xA2) return false;
  int pdulen = buf[p++];
  (void)pdulen;

  // skip request-id INTEGER
  if (buf[p++] != 0x02) return false;
  int ridlen = buf[p++];
  p += ridlen;

  // skip error-status
  if (buf[p++] != 0x02) return false;
  int eslen = buf[p++];
  p += eslen;

  // skip error-index
  if (buf[p++] != 0x02) return false;
  int eil = buf[p++];
  p += eil;

  // Varbind list
  if (buf[p++] != 0x30) return false;
  int vbl = buf[p++];
  (void)vbl;

  // Varbind entry
  if (buf[p++] != 0x30) return false;
  int vblen = buf[p++];
  (void)vblen;

  // OID
  if (buf[p++] != 0x06) return false;
  int oidlen = buf[p++];
  p += oidlen;

  // value type (INTEGER or GAUGE32 = 0x41)
  uint8_t type = buf[p++];
  int vlen2 = buf[p++];

  if (vlen2 <= 0 || vlen2 > 4) return false;

  long out = 0;
  for (int i = 0; i < vlen2; i++) {
    out = (out << 8) | buf[p + i];
  }

  *value = out;
  return true;
}

/***************************************************
 * SNMP GET
 ***************************************************/
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
    ESP_LOGE(TAG, "Packet build failed");
    return false;
  }

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
        return parse_snmp_response(resp, n, value);
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}
