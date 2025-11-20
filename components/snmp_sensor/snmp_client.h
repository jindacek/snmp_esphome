#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

class SnmpClient {
public:
  SnmpClient();

  // Inicializace UDP klienta na lokálním portu (např. 50000)
  bool begin(uint16_t local_port = 50000);

  // Jednoduchý SNMPv1 GET
  // host      – IP adresa zařízení (např. "192.168.2.230")
  // community – "public"
  // oid       – např. "1.3.6.1.4.1.318.1.1.1.3.2.1.0"
  // value     – výstup (integer / unsigned)
  bool get(const char *host,
           const char *community,
           const char *oid,
           long *value);

private:
  WiFiUDP udp_;

  int build_snmp_get_packet(uint8_t *buf, int buf_size,
                            const char *community,
                            const char *oid_str);

  bool parse_snmp_response(uint8_t *buf, int len, long *value);
};
