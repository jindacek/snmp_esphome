#include "snmp_packet.h"

bool snmp_parse_int_response(uint8_t *buf, int len, long *value) {
  if (len < 2) return false;

  for (int i = 0; i < len - 2; i++) {
    if (buf[i] == 0x02) {  // Integer tag
      int vlen = buf[i + 1];
      if (vlen > 0 && vlen <= 4 && (i + 2 + vlen) <= len) {
        long val = 0;
        for (int j = 0; j < vlen; j++) {
          val = (val << 8) | buf[i + 2 + j];
        }
        *value = val;
        return true;
      }
    }
  }

  return false;
}
