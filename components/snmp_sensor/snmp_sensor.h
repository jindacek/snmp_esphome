#pragma once

#include "esphome.h"

namespace esphome {
namespace snmp {

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  SnmpSensor() : PollingComponent(0) {}
  
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_port(uint16_t port) { port_ = port; }

  void update() override;

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
  uint16_t port_{161};
  
  bool send_snmp_query();
  std::vector<uint8_t> build_snmp_packet();
  std::string oid_to_bytes(const std::string &oid);
  float parse_snmp_response(const uint8_t *buffer, size_t length);
};

}  // namespace snmp
}  // namespace esphome
