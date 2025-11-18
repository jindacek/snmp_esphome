#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include "snmp_packet.h"

class SnmpClient {
 public:
  bool begin(uint16_t local_port = 16100);
  bool get(const char *host,
           const char *community,
           const char *oid,
           long *out_value);

 private:
  WiFiUDP udp_;
};
