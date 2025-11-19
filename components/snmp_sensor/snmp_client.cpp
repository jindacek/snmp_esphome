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

  uint8_t packet[300];
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
      uint8_t buffer[300];
      int n = udp_.read(buffer, sizeof(buffer));
      if (n > 0) {
        return parse_snmp_response(buffer, n, value);
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}

// ------------------------ SNMP v1 GET PACKET -----------------------------

// Encode OID string (e.g. "1.3.6.1.4.1.318...") into ASN.1 BER format
static int encode_oid(const char *oid, uint8_t *out) {
  uint8_t temp[64];
  int tpos = 0;

  char oid_copy[128];
  strncpy(oid_copy, oid, sizeof(oid_copy));
  oid_copy[sizeof(oid_copy) - 1] = 0;

  char *tok = strtok(oid_copy, ".");
  int nums[32];
  int count = 0;

  while (tok && count < 32) {
    nums[count++] = atoi(tok);
    tok = strtok(nullptr, ".");
  }

  if (count < 2) return 0;

  temp[tpos++] = (uint8_t)(nums[0] * 40 + nums[1]);

  for (int i = 2; i < count; i++) {
    int v = nums[i];
    if (v < 128) {
      temp[tpos++] = (uint8_t)v;
    } else {
      temp[tpos++] = 0x80 | (v >> 7);
      temp[tpos++] = (uint8_t)(v & 0x7F);
    }
  }

  memcpy(out, temp, tpos);
  return tpos;
}

int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community, const char *oid) {
  uint8_t oid_encoded[64];
  int oid_len = encode_oid(oid, oid_encoded);
  if (oid_len <= 0) return 0;

  uint8_t *p = buf;

  // --- SEQUENCE (outer) ---
  *p++ = 0x30;  // SEQUENCE
  uint8_t *len_total = p++; // placeholder

  // Version
  *p++ = 0x02; *p++ = 0x01; *p++ = 0x00;

  // Community
  *p++ = 0x04;
  *p++ = strlen(community);
  memcpy(p, community, strlen(community));
  p += strlen(community);

  // PDU = GetRequest = A0
  *p++ = 0xA0;
  uint8_t *len_pdu = p++;  // placeholder

  // Request ID
  *p++ = 0x02; *p++ = 0x01; *p++ = 1;

  // Error status / index
  *p++ = 0x02; *p++ = 0x01; *p++ = 0;
  *p++ = 0x02; *p++ = 0x01; *p++ = 0;

  // Varbind list
  *p++ = 0x30;
  uint8_t *len_vbl = p++;

  // Varbind
  *p++ = 0x30;
  uint8_t *len_vb = p++;

  // OID
  *p++ = 0x06;
  *p++ = oid_len;
  memcpy(p, oid_encoded, oid_len);
  p += oid_len;

  // NULL value
  *p++ = 0x05;
  *p++ = 0x00;

  *len_vb = (uint8_t)(p - (len_vb + 1));
  *len_vbl = (uint8_t)(p - (len_vbl + 1));
  *len_pdu = (uint8_t)(p - (len_pdu + 1));
  *len_total = (uint8_t)(p - (len_total + 1));

  return p - buf;
}


// ------------------------ RESPONSE PARSER -----------------------------

bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  for (int i = 0; i < len - 5; i++) {
    if (buf[i] == 0x42) {   // ASN_UNSIGNED
      int l = buf[i+1];
      if (l > 0 && l <= 4 && i+2+l <= len) {
        long v = 0;
        for (int j = 0; j < l; j++) {
          v = (v << 8) | buf[i+2+j];
        }
        *value = v;
        return true;
      }
    }
  }
  return false;
}
