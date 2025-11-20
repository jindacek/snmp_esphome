#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent {
 public:
  SnmpSensor(const std::string &host, const std::string &community)
      : host_(host), community_(community) {}

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;
  SnmpClient snmp_;
};

}  // namespace snmp_sensor
}  // namespace esphome
