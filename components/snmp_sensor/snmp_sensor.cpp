#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

static const char *UPS_OIDS[13] = {
  "1.3.6.1.4.1.318.1.1.1.2.2.3.0",
  "1.3.6.1.4.1.318.1.1.1.2.2.1.0",
  "1.3.6.1.4.1.318.1.1.1.2.2.2.0",
  "1.3.6.1.4.1.318.1.1.1.2.2.8.0",
  "1.3.6.1.4.1.318.1.1.1.3.2.1.0",
  "1.3.6.1.4.1.318.1.1.1.4.2.1.0",
  "1.3.6.1.4.1.318.1.1.1.4.2.3.0",
  "1.3.6.1.4.1.318.1.1.1.4.1.1.0",
  "1.3.6.1.4.1.318.1.1.1.1.1.1.0",
  "1.3.6.1.4.1.318.1.1.1.1.1.2.0",
  "1.3.6.1.4.1.318.1.1.1.1.2.2.0",
  "1.3.6.1.4.1.318.1.1.1.2.1.3.0",
  "1.3.6.1.4.1.318.1.1.1.7.2.4.0"
};

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP sensor init");

  if (!snmp_.begin(161)) {
    ESP_LOGE(TAG, "Failed to start UDP");
    this->mark_failed();
  }
}

void SnmpSensor::update() {
  long values[13];
  if (!snmp_.get_many(host_.c_str(), community_.c_str(), UPS_OIDS, 13, values)) {
    ESP_LOGW(TAG, "SNMP get_many failed");
    return;
  }

  if (runtime_sensor_) runtime_sensor_->publish_state(values[0] / 100.0f);
  if (battery_capacity_sensor_) battery_capacity_sensor_->publish_state(values[1]);
  if (battery_temp_sensor_) battery_temp_sensor_->publish_state(values[2] / 10.0f);
  if (battery_voltage_sensor_) battery_voltage_sensor_->publish_state(values[3]);
  if (input_voltage_sensor_) input_voltage_sensor_->publish_state(values[4]);
  if (output_voltage_sensor_) output_voltage_sensor_->publish_state(values[5]);
  if (load_sensor_) load_sensor_->publish_state(values[6]);
  if (output_status_sensor_) output_status_sensor_->publish_state(values[7]);
  if (model_sensor_) model_sensor_->publish_state(values[8]);
  if (name_sensor_) name_sensor_->publish_state(values[9]);
  if (manufacture_date_sensor_) manufacture_date_sensor_->publish_state(values[10]);
  if (last_battery_replacement_sensor_) last_battery_replacement_sensor_->publish_state(values[11]);
  if (last_start_time_sensor_) last_start_time_sensor_->publish_state(values[12]);
}

}  // namespace snmp_sensor
}  // namespace esphome
