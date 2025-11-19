#include "snmp_client.h"
#include <esphome/core/log.h>
#include <cstring>

static const char *TAG = "snmp_client";

// ----------------------------------------------------
// ASN.1 OID ENCODER
// ----------------------------------------------------
static int encode_oid(const char *oid_str, uint8_t *out)
{
  int out_len = 0;
  int numbers[32];
  int count = 0;

  // Parse dotted OID string -> int array
  const char *p = oid_str;
  while (*p) {
    numbers[count++] = atoi(p);
    p = strchr(p, '.');
    if (!p) break;
    p++;
  }

  if (count < 2) return -1;

  // First byte = 40 * X + Y
  out[out_len++] = numbers[0] * 40 + numbers[1];

  // Remaining numbers in base-128
  for (int i = 2; i < count; i++) {
    uint32_t val = numbers[i];
    uint8_t tmp[8];
    int t = 0;

    do {
      tmp[t++] = val & 0x7F;
      val >>= 7;
    } while (val > 0);

    for (int j = t - 1; j >= 0; j--) {
      uint8_t b = tmp[j];
      if (j != 0) b |= 0x80;
      out[out_len++] = b;
    }
  }

  return out_len;
}

// ----------------------------------------------------
// CLASS SnmpClient
// ----------------------------------------------------

SnmpClient::SnmpClient() {}

bool SnmpClient::begin(uint16_t local_port) {
  if (!udp_.begin(local_port)) {
    ESP_LOGE(TAG, "UDP begin(%d) failed!", local_port);
    return false;
  }
  ESP_LOGI(TAG, "UDP client started on port %d", local_port);
  return true;
}

// ----------------------------------------------------
// BUILD REAL SNMPv1 GET PACKET
// ----------------------------------------------------
int SnmpClient::build_snmp_get_packet(uint8_t *buf, int buf_size,
                                      const char *community, const char *oid)
{
  uint8_t oid_buf[64];
  int oid_len = encode_oid(oid, oid_buf);
  if (oid_len <= 0) return 0;

  int p = 0;

  buf[p++] = 0x30;          // SEQUENCE
  int len_pos = p++;        // placeholder
  int start = p;

  // Version = 0
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // Community
  buf[p++] = 0x04;
  buf[p++] = strlen(community);
  memcpy(&buf[p], community, strlen(community));
  p += strlen(community);

  // GET REQUEST PDU
  buf[p++] = 0xA0;
  int pdu_len_pos = p++;
  int pdu_start = p;

  // Request ID = 1
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x01;

  // Error status = 0
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // Error index = 0
  buf[p++] = 0x02; buf[p++] = 0x01; buf[p++] = 0x00;

  // Varbind list
  buf[p++] = 0x30;
  int vbl_len_pos = p++;
  int vbl_start = p;

  // Varbind
  buf[p++] = 0x30;
  int vb_len_pos = p++;
  int vb_start = p;

  // OID
  buf[p++] = 0x06;
  buf[p++] = oid_len;
  memcpy(&buf[p], oid_buf, oid_len);
  p += oid_len;

  // Value = NULL
  buf[p++] = 0x05;
  buf[p++] = 0x00;

  // Fill lengths
  buf[vb_len_pos] = p - vb_start;
  buf[vbl_len_pos] = p - vbl_start;
  buf[pdu_len_pos] = p - pdu_start;
  buf[len_pos]     = p - start;

  return p;
}

// ----------------------------------------------------
// SEND GET AND READ RESPONSE
// ----------------------------------------------------
bool SnmpClient::get(const char *host, const char *community, const char *oid, long *value)
{
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

  uint32_t deadline = millis() + 800;

  while (millis() < deadline) {
    int size = udp_.parsePacket();
    if (size > 0) {
      uint8_t buffer[256];
      int n = udp_.read(buffer, sizeof(buffer));

      if (n > 0) {
        // Na první verzi: jen najdeme INTEGER v odpovědi
        for (int i = 0; i < n - 2; i++) {
          if (buffer[i] == 0x02) {   // INTEGER
            int l = buffer[i+1];
            if (l >= 1 && l <= 4 && i+2+l <= n) {
              long v = 0;
              for (int j = 0; j < l; j++) {
                v = (v << 8) | buffer[i+2+j];
              }
              *value = v;
              return true;
            }
          }
        }

        ESP_LOGW(TAG, "SNMP response received but no INTEGER value found");
        return false;
      }
    }
  }

  ESP_LOGW(TAG, "SNMP timeout for host %s oid %s", host, oid);
  return false;
}
