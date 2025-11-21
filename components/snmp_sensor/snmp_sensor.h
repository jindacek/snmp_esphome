#pragma once
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }
  void set_oid(const std::string &o) { oid_ = o; }

  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_input_voltage_sensor(sensor::Sensor *s) { input_voltage_sensor_ = s; }
  void set_output_voltage_sensor(sensor::Sensor *s) { output_voltage_sensor_ = s; }
  void set_load_sensor(sensor::Sensor *s) { load_sensor_ = s; }


  void setup() override;
  void update() override;

 private:
  std::string host_;
  std::string community_;
  std::string oid_;

  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *load_sensor_{nullptr};


  SnmpClient snmp_;
  bool snmp_initialized_ = false;
};

}  // namespace snmp_sensor
}  // namespace esphome
