#pragma once

#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <WiFi.h>
#include <Udp.h>

namespace esphome {
namespace snmp_sensor {

class SNMPSensor : public PollingComponent {
 public:
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_name(const std::string &name) { name_ = name; }
  void set_unit(const std::string &unit) { unit_ = unit; }

  void update() override;

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
  std::string name_;
  std::string unit_;
};

}  // namespace snmp_sensor
}  // namespace esphome
