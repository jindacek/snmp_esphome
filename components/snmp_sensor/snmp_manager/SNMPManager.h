#pragma once
#include <Arduino.h>
#include <WiFiUdp.h>
#include "SNMPPacket.h"
#include "SNMPVariable.h"

class SNMPManager {
public:
  SNMPManager();
  bool begin();

  bool get(const char* host, const char* community, const char* oid, long* value);

private:
  WiFiUDP udp_;
};
