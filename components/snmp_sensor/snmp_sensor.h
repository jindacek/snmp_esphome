#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent {
 public:
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }

  // setters pro pod-senzory (numeric only)
  void set_runtime_sensor(sensor::Sensor *s) { runtime_sensor_ = s; }
  void set_battery_capacity_sensor(sensor::Sensor *s) { battery_capacity_sensor_ = s; }
  void set_battery_temp_sensor(sensor::Sensor *s) { battery_temp_sensor_ = s; }
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_input_voltage_sensor(sensor::Sensor *s) { input_voltage_sensor_ = s; }
  void set_output_voltage_sensor(sensor::Sensor *s) { output_voltage_sensor_ = s; }
  void set_load_sensor(sensor::Sensor *s) { load_sensor_ = s; }
  void set_output_status_sensor(sensor::Sensor *s) { output_status_sensor_ = s; }
  void set_remaining_runtime_sensor(sensor::Sensor *s) { remaining_runtime_sensor_ = s; }
  void set_self_test_result_sensor(sensor::Sensor *s) { self_test_result_sensor_ = s; }
  void set_battery_replace_status_sensor(sensor::Sensor *s) { battery_replace_status_sensor_ = s; }
  void set_output_source_sensor(sensor::Sensor *s) { output_source_sensor_ = s; }
  void set_input_frequency_sensor(sensor::Sensor *s) { input_frequency_sensor_ = s; }
  void set_output_frequency_sensor(sensor::Sensor *s) { output_frequency_sensor_ = s; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;

  // numeric child sensors
  sensor::Sensor *runtime_sensor_{nullptr};
  sensor::Sensor *battery_capacity_sensor_{nullptr};
  sensor::Sensor *battery_temp_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *load_sensor_{nullptr};
  sensor::Sensor *output_status_sensor_{nullptr};
  sensor::Sensor *remaining_runtime_sensor_{nullptr};
  sensor::Sensor *self_test_result_sensor_{nullptr};
  sensor::Sensor *battery_replace_status_sensor_{nullptr};
  sensor::Sensor *output_source_sensor_{nullptr};
  sensor::Sensor *input_frequency_sensor_{nullptr};
  sensor::Sensor *output_frequency_sensor_{nullptr};

  SnmpClient snmp_;
  bool snmp_initialized_{false};
};

}  // namespace snmp_sensor
}  // namespace esphome
