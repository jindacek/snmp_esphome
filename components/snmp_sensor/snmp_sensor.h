#pragma once

#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public PollingComponent {
 public:
  void set_host(const std::string &h) { host_ = h; }
  void set_community(const std::string &c) { community_ = c; }

  // numeric setters
  void set_battery_voltage_sensor(sensor::Sensor *s) { battery_voltage_sensor_ = s; }
  void set_input_voltage_sensor(sensor::Sensor *s) { input_voltage_sensor_ = s; }
  void set_output_voltage_sensor(sensor::Sensor *s) { output_voltage_sensor_ = s; }
  void set_load_sensor(sensor::Sensor *s) { load_sensor_ = s; }
  void set_runtime_sensor(sensor::Sensor *s) { runtime_sensor_ = s; }
  void set_remaining_runtime_sensor(sensor::Sensor *s) { remaining_runtime_sensor_ = s; }
  void set_battery_capacity_sensor(sensor::Sensor *s) { battery_capacity_sensor_ = s; }
  void set_battery_temp_sensor(sensor::Sensor *s) { battery_temp_sensor_ = s; }
  void set_input_frequency_sensor(sensor::Sensor *s) { input_frequency_sensor_ = s; }
  void set_output_frequency_sensor(sensor::Sensor *s) { output_frequency_sensor_ = s; }
  void set_output_status_sensor(sensor::Sensor *s) { output_status_sensor_ = s; }
  void set_output_source_sensor(sensor::Sensor *s) { output_source_sensor_ = s; }
  void set_battery_replace_status_sensor(sensor::Sensor *s) { battery_replace_status_sensor_ = s; }
  void set_self_test_result_sensor(sensor::Sensor *s) { self_test_result_sensor_ = s; }

  // text setters
  void set_model_text_sensor(text_sensor::TextSensor *s) { model_text_sensor_ = s; }
  void set_name_text_sensor(text_sensor::TextSensor *s) { name_text_sensor_ = s; }
  void set_manufacture_date_text_sensor(text_sensor::TextSensor *s) { manufacture_date_text_sensor_ = s; }
  void set_last_battery_replacement_text_sensor(text_sensor::TextSensor *s) { last_battery_replacement_text_sensor_ = s; }
  void set_last_self_test_text_sensor(text_sensor::TextSensor *s) { last_self_test_text_sensor_ = s; }
  void set_serial_number_text_sensor(text_sensor::TextSensor *s) { serial_number_text_sensor_ = s; }
  void set_runtime_formatted_text_sensor(text_sensor::TextSensor *s) { runtime_formatted_text_sensor_ = s; }
  void set_remaining_runtime_formatted_text_sensor(text_sensor::TextSensor *s) { remaining_runtime_formatted_text_sensor_ = s; }
  void set_output_status_text_sensor(text_sensor::TextSensor *s) { output_status_text_sensor_ = s; }
  void set_output_source_text_sensor(text_sensor::TextSensor *s) { output_source_text_sensor_ = s; }
  void set_battery_replace_text_sensor(text_sensor::TextSensor *s) { battery_replace_text_sensor_ = s; }
  void set_self_test_text_sensor(text_sensor::TextSensor *s) { self_test_text_sensor_ = s; }

  void setup() override;
  void update() override;

 protected:
  std::string host_;
  std::string community_;

  // numeric outputs
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  sensor::Sensor *input_voltage_sensor_{nullptr};
  sensor::Sensor *output_voltage_sensor_{nullptr};
  sensor::Sensor *load_sensor_{nullptr};
  sensor::Sensor *runtime_sensor_{nullptr};
  sensor::Sensor *remaining_runtime_sensor_{nullptr};
  sensor::Sensor *battery_capacity_sensor_{nullptr};
  sensor::Sensor *battery_temp_sensor_{nullptr};
  sensor::Sensor *input_frequency_sensor_{nullptr};
  sensor::Sensor *output_frequency_sensor_{nullptr};
  sensor::Sensor *output_status_sensor_{nullptr};
  sensor::Sensor *output_source_sensor_{nullptr};
  sensor::Sensor *battery_replace_status_sensor_{nullptr};
  sensor::Sensor *self_test_result_sensor_{nullptr};

  // text outputs
  text_sensor::TextSensor *model_text_sensor_{nullptr};
  text_sensor::TextSensor *name_text_sensor_{nullptr};
  text_sensor::TextSensor *manufacture_date_text_sensor_{nullptr};
  text_sensor::TextSensor *last_battery_replacement_text_sensor_{nullptr};
  text_sensor::TextSensor *last_self_test_text_sensor_{nullptr};
  text_sensor::TextSensor *serial_number_text_sensor_{nullptr};
  text_sensor::TextSensor *runtime_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *remaining_runtime_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *output_status_text_sensor_{nullptr};
  text_sensor::TextSensor *output_source_text_sensor_{nullptr};
  text_sensor::TextSensor *battery_replace_text_sensor_{nullptr};
  text_sensor::TextSensor *self_test_text_sensor_{nullptr};

  SnmpClient snmp_;
  bool snmp_initialized_{false};
};

}  // namespace snmp_sensor
}  // namespace esphome
