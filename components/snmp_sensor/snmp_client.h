#pragma once

#include <Arduino.h>
#include <WiFiUdp.h>

class SnmpClient {
 public:
  SnmpClient();

  bool begin(uint16_t local_port = 161);

  // Jedno OID – stávající API (kvůli kompatibilitě)
  bool get(const char *host,
           const char *community,
           const char *oid,
           long *value);

  // Multi-OID GET
  bool get_many(const char *host,
                const char *community,
                const char **oids,
                int num_oids,
                long *values);

 private:
  WiFiUDP udp_;

  int build_snmp_get_packet(uint8_t *buf, int buf_size,
                            const char *community,
                            const char *oid);

  int build_snmp_get_packet_multi(uint8_t *buf, int buf_size,
                                  const char *community,
                                  const char **oids,
                                  int num_oids);

  bool parse_snmp_response(uint8_t *buf, int len, long *value);

  bool parse_snmp_response_multi(uint8_t *buf, int len,
                                 long *values,
                                 int num_oids);
};
