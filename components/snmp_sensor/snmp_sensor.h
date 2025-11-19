#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

extern SnmpClient global_snmp_client;

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }
  void set_oid(const std::string &o) { oid_ = o; }

  void setup() override;
  void update() override;

 private:
  std::string host_;
  std::string community_;
  std::string oid_;
};

}  // namespace snmp_sensor
}  // namespace esphome
