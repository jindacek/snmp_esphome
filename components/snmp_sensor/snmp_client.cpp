#include "snmp_client.h"
#include <esphome/core/log.h>
#include <cstring>

static const char *TAG = "snmp_client";

SnmpClient::SnmpClient() {}

bool SnmpClient::begin(uint16_t local_port) {
  if (!udp_.begin(local_port)) {
    ESP_LOGE(TAG, "UDP begin(%d) failed!", local_port);
    return false;
  }
  ESP_LOGI(TAG, "UDP client started on port %d", local_port);
  return true;
}

bool SnmpClient::get(const char *host,
                     const char *community,
                     const char *oid,
                     long *value) {
  IPAddress ip;
  if (!ip.fromString(host)) {
    ESP_LOGE(TAG, "Invalid host: %s", host);
    return false;
  }

  uint8_t packet[256];
  int len = build_snmp_get_packet(packet, sizeof(packet), community, oid);
  if (len <= 0) {
    ESP_LOGE(TAG, "Failed to build SNMP GET packet");
    return false;
  }

  udp_.beginPacket(ip, 161);
  udp_.write(packet, len);
  udp_.endPacket();

  uint32_t deadline = millis() + 500;
  while (millis() < deadline) {
    int size = udp_.parsePacket();
    if (size > 0) {
      uint8_t buffer[256];
      int n = udp_.read(buffer, sizeof(buffer));
      if (n > 0) {
        return parse_snmp_response(buffer, n, value);
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}


// ---------------------------------------------------------
// Convert OID string "1.3.6.1.4..." → BER encoded bytes
// ---------------------------------------------------------
int encode_oid(const char *oid_str, uint8_t *output) {
  uint32_t parts[32];
  int count = 0;

  const char *p = oid_str;
  while (*p && count < 32) {
    parts[count++] = strtoul(p, (char **)&p, 10);
    if (*p == '.') p++;
  }

  if (count < 2) return 0;

  int out = 0;

  // first byte = 40 * first + second
  output[out++] = 40 * parts[0] + parts[1];

  // subsequent bytes encoded in base-128
  for (int i = 2; i < count; i++) {
    uint32_t v = parts[i];
    uint8_t stack[5];
    int sp = 0;

    do {
      stack[sp++] = v & 0x7F;
      v >>= 7;
    } while (v);

    for (int j = sp - 1; j >= 0; j--) {
      uint8_t b = stack[j];
      if (j != 0)
        b |= 0x80;
      output[out++] = b;
    }
  }

  return out;
}


// ---------------------------------------------------------
int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community,
                                      const char *oid_str) {
  int p = 0;

  uint8_t oid_encoded[64];
  int oid_len = encode_oid(oid_str, oid_encoded);
  if (oid_len <= 0) {
    ESP_LOGE(TAG, "OID encode failed for %s", oid_str);
    return 0;
  }

  // Sequence
  buf[p++] = 0x30; // SEQ
  buf[p++] = 0x00; // filled later

  int seq_start = p;

  // version
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // community
  buf[p++] = 0x04;
  buf[p++] = strlen(community);
  memcpy(&buf[p], community, strlen(community));
  p += strlen(community);

  // GET PDU
  buf[p++] = 0xA0;
  buf[p++] = 0x00; // fill later
  int pdu_start = p;

  // request-id
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x01;

  // error-status
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // error-index
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // varbind list
  buf[p++] = 0x30;
  buf[p++] = 0x00; // fill later
  int vbl_start = p;

  // varbind
  buf[p++] = 0x30;
  buf[p++] = 0x00; // fill later
  int vb_start = p;

  // OID
  buf[p++] = 0x06;
  buf[p++] = oid_len;
  memcpy(&buf[p], oid_encoded, oid_len);
  p += oid_len;

  // NULL
  buf[p++] = 0x05;
  buf[p++] = 0x00;

  // fix lengths
  buf[vb_start - 1] = p - vb_start;                           // varbind length
  buf[vbl_start - 1] = p - vbl_start - 2;                     // vbl length
  buf[pdu_start - 1] = p - pdu_start - 2;                     // PDU len
  buf[1] = p - 2;                                             // total SEQ len

  return p;
}


// ---------------------------------------------------------
bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  int i = 0;

  if (buf[i++] != 0x30) return false;
  int total = buf[i++];

  // find response PDU (A2)
  while (i < len && buf[i] != 0xA2) i++;
  if (i >= len) return false;
  i++; // skip tag

  int pdu_len = buf[i++];

  // skip request-id, error-status, error-index
  for (int k = 0; k < 3; k++) {
    if (buf[i++] != 0x02) return false;
    int l = buf[i++];
    i += l;
  }

  // varbind list
  if (buf[i++] != 0x30) return false;
  int vbl_len = buf[i++];
  int vbl_end = i + vbl_len;

  // varbind
  if (buf[i++] != 0x30) return false;
  int vb_len = buf[i++];
  int vb_end = i + vb_len;

  // skip OID
  if (buf[i++] != 0x06) return false;
  int oid_len = buf[i++];
  i += oid_len;

  uint8_t type = buf[i++]; // 0x41 Gauge32, 0x02 Integer
  int l = buf[i++];

  if (l <= 0 || l > 4 || i + l > len) return false;

  long v = 0;
  for (int j = 0; j < l; j++) {
    v = (v << 8) | buf[i + j];
  }

  *value = v;
  return true;
}
