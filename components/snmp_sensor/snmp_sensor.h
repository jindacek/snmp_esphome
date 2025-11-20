#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent {
 public:
  SnmpSensor(const std::string &host, const std::string &community)
      : host_(host), community_(community) {}

  void set_runtime_sensor(sensor::Sensor *s) { runtime_sensor_ = s; }
  void set_battery_capacity_sensor(sensor::Sensor *s) { battery_capacity_sensor_ = s; }
  void set_battery_temp_sensor(sensor::Sensor *s) { battery_temp_sensor_ = s; }
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_input_voltage_sensor(sensor::Sensor *s) { input_voltage_sensor_ = s; }
  void set_output_voltage_sensor(sensor::Sensor *s) { output_voltage_sensor_ = s; }
  void set_load_sensor(sensor::Sensor *s) { load_sensor_ = s; }
  void set_output_status_sensor(sensor::Sensor *s) { output_status_sensor_ = s; }
  void set_model_sensor(sensor::Sensor *s) { model_sensor_ = s; }
  void set_name_sensor(sensor::Sensor *s) { name_sensor_ = s; }
  void set_manufacture_date_sensor(sensor::Sensor *s) { manufacture_date_sensor_ = s; }
  void set_last_battery_replacement_sensor(sensor::Sensor *s) { last_battery_replacement_sensor_ = s; }
  void set_last_start_time_sensor(sensor::Sensor *s) { last_start_time_sensor_ = s; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;
  SnmpClient snmp_;

  sensor::Sensor *runtime_sensor_{nullptr};
  sensor::Sensor *battery_capacity_sensor_{nullptr};
  sensor::Sensor *battery_temp_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *load_sensor_{nullptr};
  sensor::Sensor *output_status_sensor_{nullptr};
  sensor::Sensor *model_sensor_{nullptr};
  sensor::Sensor *name_sensor_{nullptr};
  sensor::Sensor *manufacture_date_sensor_{nullptr};
  sensor::Sensor *last_battery_replacement_sensor_{nullptr};
  sensor::Sensor *last_start_time_sensor_{nullptr};
};

}  // namespace snmp_sensor
}  // namespace esphome
