#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

class SnmpClient {
public:
  SnmpClient();

  bool begin(uint16_t local_port = 161);

  bool get(const char *host,
           const char *community,
           const char *oid,
           long *value);

private:
  WiFiUDP udp_;

  int build_snmp_get_packet(uint8_t *buf, int buf_size,
                            const char *community,
                            const char *oid);

  bool parse_snmp_response(uint8_t *buf, int len, long *value);
};
