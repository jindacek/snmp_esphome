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

int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community, const char *oid) {
  int p = 0;

  if (buf_size < 64) return 0;

  buf[p++] = 0x30;
  buf[p++] = 0x20;

  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;
  buf[p++] = 0x04;
  buf[p++] = strlen(community);
  memcpy(&buf[p], community, strlen(community));
  p += strlen(community);

  buf[p++] = 0xA0;
  buf[p++] = 0x14;

  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x01;

  buf[p++] = 0x04; buf[p++] = 0x00;

  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  buf[p++] = 0x30; buf[p++] = 0x0A;

  buf[p++] = 0x30; buf[p++] = 0x08;

  buf[p++] = 0x06;
  uint8_t oid_bytes[] = {1,3,6,1};
  buf[p++] = sizeof(oid_bytes);
  memcpy(&buf[p], oid_bytes, sizeof(oid_bytes));
  p += sizeof(oid_bytes);

  buf[p++] = 0x05;
  buf[p++] = 0x00;

  return p;
}

bool SnmpClient::parse_snmp_response(uint8_t *buf, int len, long *value) {
  if (len < 2) return false;

  for (int i = 0; i < len - 2; i++) {
    if (buf[i] == 0x02) {
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
