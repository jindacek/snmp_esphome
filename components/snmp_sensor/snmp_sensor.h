#pragma once

#include "esphome.h"

namespace esphome {
namespace snmp {

class SNMPSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_port(uint16_t port) { port_ = port; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
  uint16_t port_{161};
  
  bool send_snmp_query();
};

class SNMPMultiComponent : public PollingComponent {
 public:
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_port(uint16_t port) { port_ = port; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;
  uint16_t port_{161};
  
  bool send_snmp_multi_query();
};

}  // namespace snmp
}  // namespace esphome
