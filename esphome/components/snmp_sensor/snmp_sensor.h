#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_host(const std::string &host) { host_ = host; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_community(const std::string &community) { community_ = community; }

  void update() override;

 protected:
  std::string host_;
  std::string oid_;
  std::string community_;
};

}  // namespace snmp_sensor
}  // namespace esphome
