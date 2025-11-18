#pragma once
#include <stdint.h>

bool snmp_parse_int_response(uint8_t *buf, int len, long *value);
