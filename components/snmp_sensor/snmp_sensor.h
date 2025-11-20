#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

// Přidejte tuto třídu do snmp_sensor.h
class SNMPMultiComponent : public PollingComponent {
 public:
  SNMPMultiComponent() : PollingComponent(0) {}
  
  void set_host(const std::string &host) { host_ = host; }
  void set_community(const std::string &community) { community_ = community; }
  void set_port(uint16_t port) { port_ = port; }
  
  void set_voltage_oid(const std::string &oid) { voltage_oid_ = oid; }
  void set_capacity_oid(const std::string &oid) { capacity_oid_ = oid; }
  void set_runtime_oid(const std::string &oid) { runtime_oid_ = oid; }
  void set_load_oid(const std::string &oid) { load_oid_ = oid; }
  // Přidejte další OID podle potřeby...

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;
  uint16_t port_{161};
  
  std::string voltage_oid_;
  std::string capacity_oid_;
  std::string runtime_oid_;
  std::string load_oid_;
  // Přidejte další OID podle potřeby...
  
  bool send_snmp_multi_query();
};

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }

  void setup() override;
  void update() override;

 private:
  std::string host_;
  std::string community_;

  SnmpClient snmp_;
};

}  // namespace snmp_sensor
}  // namespace esphome
