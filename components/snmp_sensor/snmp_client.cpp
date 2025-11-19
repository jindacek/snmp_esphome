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

bool SnmpClient::get(const char *host, const char *community, const char *oid, long *value) {
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

  // SEND SNMP GET
  udp_.beginPacket(ip, 161);
  udp_.write(packet, len);
  udp_.endPacket();

  // WAIT FOR RESPONSE
  uint32_t deadline = millis() + 500;
  while (millis() < deadline) {
    int size = udp_.parsePacket();
    if (size > 0) {
      uint8_t buffer[256];
      int n = udp_.read(buffer, sizeof(buffer));
      if (n > 0) {
        bool ok = parse_snmp_response(buffer, n, value);
        if (!ok)
          ESP_LOGW(TAG, "Failed to parse SNMP response");
        return ok;
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}



// ---------------------------------------------------------
//  BUILD SNMP GET PACKET (minimalist SNMPv1 GET)
// ---------------------------------------------------------
int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community, const char *oid) {
  int p = 0;

  if (buf_size < 64) return 0;

  // SEQUENCE
  buf[p++] = 0x30;
  buf[p++] = 0x20;

  // version = 0
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // community
  buf[p++] = 0x04;
  buf[p++] = strlen(community);
  memcpy(&buf[p], community, strlen(community));
  p += strlen(community);

  // GET request PDU
  buf[p++] = 0xA0;
  buf[p++] = 0x14;

  // request id
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x01;

  // error status
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // error index
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // varbind list
  buf[p++] = 0x30; buf[p++] = 0x0A;

  // varbind
  buf[p++] = 0x30; buf[p++] = 0x08;

  // OID
  buf[p++] = 0x06;

  // HARD-CODED OID (zatím test) → změníme později podle tvojeho OID
  uint8_t oid_bytes[] = {1,3,6,1};
  buf[p++] = sizeof(oid_bytes);
  memcpy(&buf[p], oid_bytes, sizeof(oid_bytes));
  p += sizeof(oid_bytes);

  // NULL
  buf[p++] = 0x05; buf[p++] = 0x00;

  return p;
}



// ---------------------------------------------------------
//  PARSE SNMP RESPONSE – correct varbind decoding
// ---------------------------------------------------------
bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {

  int i = 0;

  // Expect SEQUENCE
  if (buf[i++] != 0x30) return false;
  i++; // skip total length

  // Locate GetResponse-PDU (A2)
  while (i < len && buf[i] != 0xA2) i++;
  if (i >= len) return false;

  i++; // skip tag
  i++; // skip PDU length

  // Skip: request-id, error-status, error-index
  for (int k = 0; k < 3; k++) {
    if (buf[i++] != 0x02) return false;  // INTEGER
    int l = buf[i++];
    i += l;
    if (i >= len) return false;
  }

  // Varbind-list
  if (buf[i++] != 0x30) return false;
  int vbl_len = buf[i++];
  int vbl_end = i + vbl_len;

  // Varbind
  if (buf[i++] != 0x30) return false;
  int vb_len = buf[i++];
  int vb_end = i + vb_len;

  // OID
  if (buf[i++] != 0x06) return false;
  int oid_len = buf[i++];
  i += oid_len;

  // VALUE (INTEGER / UNSIGNED)
  uint8_t type = buf[i++];   // 0x02 = INTEGER, 0x41 = Gauge32 / Unsigned
  int l = buf[i++];

  if (l <= 0 || l > 4 || i + l > len) return false;

  long v = 0;
  for (int j = 0; j < l; j++) {
    v = (v << 8) | buf[i + j];
  }

  *value = v;
  return true;
}
