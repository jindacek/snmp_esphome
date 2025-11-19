#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent, public sensor::Sensor {
 public:
  // Konstruktor musí přijmout interval – ESPHome to vyžaduje
  SnmpSensor() : PollingComponent(2000) {}   // default 2000 ms

  void setup() override;
  void update() override;

  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
};

}  // namespace snmp_sensor
}  // namespace esphome
