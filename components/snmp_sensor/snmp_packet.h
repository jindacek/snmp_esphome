#pragma once
#include <Arduino.h>

// Sestaví SNMPv1 GET požadavek do bufferu.
// Vrací délku paketu, nebo -1 při chybě.
int snmp_build_get(uint8_t *buf, int buf_size,
                   const char *community,
                   const char *oid,
                   long request_id);

// Z parsované SNMP odpovědi vytáhne první INTEGER hodnotu.
// Vrací true/false podle úspěchu, výsledek do *out_value.
bool snmp_parse_int_response(uint8_t *buf, int len, long *out_value);
