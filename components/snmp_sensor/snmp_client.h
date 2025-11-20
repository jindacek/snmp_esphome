#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include <string>


class SnmpClient {
public:
bool begin(uint16_t port);


bool get(const char *host,
const char *community,
const char *oid,
long *value);


bool get_many(const char *host,
const char *community,
const char **oids,
int count,
long *values);


bool get_many_string(const char *host,
const char *community,
const char **oids,
int count,
std::string *values);


private:
WiFiUDP udp_;


int encode_oid(const char *oid_str, uint8_t *out, int max_len);


int build_snmp_get_packet(uint8_t *buf, int buf_size,
const char *community,
const char *oid);


int build_snmp_get_packet_multi(uint8_t *buf, int buf_size,
const char *community,
const char **oids,
int count);
};
