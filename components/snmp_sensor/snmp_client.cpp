#include "snmp_client.h"

bool SnmpClient::begin(uint16_t local_port) {
  return udp_.begin(local_port);
}

bool SnmpClient::get(const char *host,
                     const char *community,
                     const char *oid,
                     long *out_value) {
  IPAddress ip;
  if (!ip.fromString(host)) {
    return false;
  }

  uint8_t buffer[256];

  long request_id = (long) random(1, 0x7FFFFFFF);

  int len = snmp_build_get(buffer, sizeof(buffer), community, oid, request_id);
  if (len <= 0) {
    return false;
  }

  // pošli SNMP GET na port 161
  udp_.beginPacket(ip, 161);
  udp_.write(buffer, len);
  udp_.endPacket();

  unsigned long start = millis();
  while (millis() - start < 500) {  // timeout 500 ms
    int packet_len = udp_.parsePacket();
    if (packet_len > 0) {
      if (packet_len > (int)sizeof(buffer))
        packet_len = sizeof(buffer);
      udp_.read(buffer, packet_len);

      long value = 0;
      if (snmp_parse_int_response(buffer, packet_len, &value)) {
        *out_value = value;
        return true;
      } else {
        return false;
      }
    }
    delay(10);
  }

  return false;  // timeout
}
