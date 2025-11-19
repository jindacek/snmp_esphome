#pragma once

#include "esphome.h"
#include <WiFiUdp.h>

namespace esphome {
namespace snmp {

class SnmpSensor : public sensor::Sensor {  // ZMĚNA: pouze sensor::Sensor
 public:
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_port(uint16_t port) { port_ = port; }

  void setup() override;
  void update() override;
  void loop() override;
  
  // ZMĚNA: Přesunuto z Component do Sensor
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
  uint16_t port_{161};
  
  WiFiUDP udp_;
  bool initialized_{false};
  bool waiting_response_{false};
  unsigned long send_time_{0};
  const unsigned long timeout_{5000};
  
  void send_snmp_get();
  void handle_response();
  std::string oid_to_bytes(const std::string &oid);
  float parse_snmp_response(const uint8_t *buffer, size_t length);
};

}  // namespace snmp
}  // namespace esphome
