#pragma once
#include <Arduino.h>
#include "SNMPVariable.h"

class SNMPPacket {
public:
  SNMPPacket();

  void setVersion(int version);
  void setCommunity(const char* community);
  void addGetRequest(const char* oid);

  uint8_t* data();
  int length();

  static bool parseResponse(uint8_t* data, int len, SNMPVariable* out);

private:
  uint8_t buffer_[512];
  int length_;
};

