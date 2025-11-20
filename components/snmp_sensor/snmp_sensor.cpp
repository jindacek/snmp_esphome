#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// 🔥 Seznam 13 OID
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
  ESP_LOGI(TAG, "SNMP multi-OID sensor init");

  // 🔧 MUSÍ být jinak UDP nefunguje!
  if (!snmp_.begin(161)) {
    ESP_LOGE(TAG, "SNMP UDP begin failed!");
    this->mark_failed();
  }
}

void SnmpSensor::update() {
  const int COUNT = 13;
  long values[COUNT];

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  bool ok = snmp_.get_many(
      host_.c_str(),
      community_.c_str(),
      UPS_OIDS,
      COUNT,
      values
  );

  if (!ok) {
    ESP_LOGW(TAG, "MULTI-GET FAILED!");
    return;
  }

  ESP_LOGI(TAG, "MULTI-GET OK:");
  ESP_LOGI(TAG, "Runtime: %ld ticks", values[0]);
  ESP_LOGI(TAG, "Battery Cap: %ld %%", values[1]);
  ESP_LOGI(TAG, "Battery Temp: %ld", values[2]);

  // Publikace do template senzorů v YAML
  id(ups_runtime).publish_state(values[0] / 100.0f);
  id(ups_battery_capacity).publish_state(values[1]);
  id(ups_battery_temp).publish_state(values[2] / 10.0f);
  id(ups_battery_voltage).publish_state(values[3]);
  id(ups_input_voltage).publish_state(values[4]);
  id(ups_output_voltage).publish_state(values[5]);
  id(ups_load).publish_state(values[6]);
  id(ups_output_status).publish_state(values[7]);
  id(ups_model).publish_state(values[8]);
  id(ups_name).publish_state(values[9]);
  id(ups_manufacture_date).publish_state(values[10]);
  id(ups_last_battery_replacement).publish_state(values[11]);
  id(ups_last_start_time).publish_state(values[12]);
}

}  // namespace snmp_sensor
}  // namespace esphome
