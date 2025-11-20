#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>
#include <string>

class SnmpClient {
 public:
  SnmpClient();

  bool begin(uint16_t local_port = 161);

  // Jedno OID
  bool get(const char *host,
           const char *community,
           const char *oid,
           long *value);

  // Multi-OID GET – numerické hodnoty
  bool get_many(const char *host,
                const char *community,
                const char **oids,
                int num_oids,
                long *values);

  // Multi-OID GET – string (OCTET STRING)
  bool get_many_string(const char *host,
                       const char *community,
                       const char **oids,
                       int num_oids,
                       std::string *values);

 private:
  WiFiUDP udp_;

  int build_snmp_get_packet(uint8_t *buf, int buf_size,
                            const char *community,
                            const char *oid);

  // multi-OID builder
  int build_snmp_get_packet_multi(uint8_t *buf, int buf_size,
                                  const char *community,
                                  const char **oids,
                                  int num_oids);

  bool parse_snmp_response(uint8_t *buf, int len, long *value);

  // multi-OID parser (původní, teď už ho nepoužíváme)
  bool parse_snmp_response_multi(uint8_t *buf, int len,
                                 long *values,
                                 int num_oids);
};
