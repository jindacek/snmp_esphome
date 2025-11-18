#pragma once
#include <Arduino.h>

class SNMPVariable {
public:
  SNMPVariable() : type_(0), int_value_(0) {}

  void setInteger(long value) { type_ = 2; int_value_ = value; }
  long asInteger() { return int_value_; }

private:
  int type_;
  long int_value_;
};

