#pragma once

#include "esphome.h"
#include <WiFiUdp.h>

namespace esphome {
namespace snmp {

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  // Konstruktor
  SnmpSensor() : PollingComponent(0) {}
  
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_oid(const std::string &oid) { oid_ = oid; }
  void set_port(uint16_t port) { port_ = port; }

  void setup() override;
  void update() override;
  void loop() override;

  // Přidejte chybějící virtuální metody ze sensor::Sensor
  std::string unit_of_measurement() override { return unit_of_measurement_; }
  void set_unit_of_measurement(const std::string &unit) { unit_of_measurement_ = unit; }
  
  int get_accuracy_decimals() override { return accuracy_decimals_; }
  void set_accuracy_decimals(int decimals) { accuracy_decimals_ = decimals; }
  
  std::string get_state_class() override { return state_class_; }
  void set_state_class(const std::string &state_class) { state_class_ = state_class; }
  
  std::string get_device_class() override { return device_class_; }
  void set_device_class(const std::string &device_class) { device_class_ = device_class; }

 protected:
  std::string host_;
  std::string community_;
  std::string oid_;
  uint16_t port_{161};
  
  // Přidejte proměnné pro vlastnosti senzoru
  std::string unit_of_measurement_;
  int accuracy_decimals_{0};
  std::string state_class_;
  std::string device_class_;
  
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
