#pragma once

#include <string>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent {
 public:
  // setters z codegen / yaml
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }

  // (zatím nepoužíváme, ale necháme si pro příští krok – publish do HA)
  void set_runtime_sensor(sensor::Sensor *s) { runtime_sensor_ = s; }
  void set_battery_capacity_sensor(sensor::Sensor *s) { battery_capacity_sensor_ = s; }
  void set_battery_temp_sensor(sensor::Sensor *s) { battery_temp_sensor_ = s; }
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_input_voltage_sensor(sensor::Sensor *s) { input_voltage_sensor_ = s; }
  void set_output_voltage_sensor(sensor::Sensor *s) { output_voltage_sensor_ = s; }
  void set_load_sensor(sensor::Sensor *s) { load_sensor_ = s; }
  void set_output_status_sensor(sensor::Sensor *s) { output_status_sensor_ = s; }
  void set_remaining_runtime_sensor(sensor::Sensor *s) { remaining_runtime_sensor_ = s; }
  void set_output_frequency_sensor(sensor::Sensor *s) { output_frequency_sensor_ = s; }
  void set_selftest_result_sensor(sensor::Sensor *s) { selftest_result_sensor_ = s; }
  void set_selftest_duration_sensor(sensor::Sensor *s) { selftest_duration_sensor_ = s; }

  void set_model_text_sensor(text_sensor::TextSensor *s) { model_text_sensor_ = s; }
  void set_name_text_sensor(text_sensor::TextSensor *s) { name_text_sensor_ = s; }
  void set_manufacture_date_text_sensor(text_sensor::TextSensor *s) { manufacture_date_text_sensor_ = s; }
  void set_last_battery_replace_text_sensor(text_sensor::TextSensor *s) { last_battery_replace_text_sensor_ = s; }
  void set_last_start_time_text_sensor(text_sensor::TextSensor *s) { last_start_time_text_sensor_ = s; }
  void set_last_selftest_date_text_sensor(text_sensor::TextSensor *s) { last_selftest_date_text_sensor_ = s; }
  void set_runtime_formatted_text_sensor(text_sensor::TextSensor *s) { runtime_formatted_text_sensor_ = s; }
  void set_remaining_runtime_formatted_text_sensor(text_sensor::TextSensor *s) { remaining_runtime_formatted_text_sensor_ = s; }
  void set_output_status_text_sensor(text_sensor::TextSensor *s) { output_status_text_sensor_ = s; }
  void set_selftest_result_text_sensor(text_sensor::TextSensor *s) { selftest_result_text_sensor_ = s; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;

  SnmpClient snmp_;

  // numeric sensors (publishing později)
  sensor::Sensor *runtime_sensor_{nullptr};
  sensor::Sensor *battery_capacity_sensor_{nullptr};
  sensor::Sensor *battery_temp_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *load_sensor_{nullptr};
  sensor::Sensor *output_status_sensor_{nullptr};
  sensor::Sensor *remaining_runtime_sensor_{nullptr};
  sensor::Sensor *output_frequency_sensor_{nullptr};
  sensor::Sensor *selftest_result_sensor_{nullptr};
  sensor::Sensor *selftest_duration_sensor_{nullptr};

  // text sensors (publishing později)
  text_sensor::TextSensor *model_text_sensor_{nullptr};
  text_sensor::TextSensor *name_text_sensor_{nullptr};
  text_sensor::TextSensor *manufacture_date_text_sensor_{nullptr};
  text_sensor::TextSensor *last_battery_replace_text_sensor_{nullptr};
  text_sensor::TextSensor *last_start_time_text_sensor_{nullptr};
  text_sensor::TextSensor *last_selftest_date_text_sensor_{nullptr};
  text_sensor::TextSensor *runtime_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *remaining_runtime_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *output_status_text_sensor_{nullptr};
  text_sensor::TextSensor *selftest_result_text_sensor_{nullptr};
};

}  // namespace snmp_sensor
}  // namespace esphome
