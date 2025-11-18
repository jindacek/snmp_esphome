#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public sensor::Sensor, public Component {
 public:
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
