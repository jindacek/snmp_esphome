#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// 🔥 13 OID – statické pole mimo funkce
static const char *UPS_OIDS[13] = {
  "1.3.6.1.4.1.318.1.1.1.2.2.3.0",  // 0 Runtime seconds
  "1.3.6.1.4.1.318.1.1.1.2.2.1.0",  // 1 Battery capacity
  "1.3.6.1.4.1.318.1.1.1.2.2.2.0",  // 2 Battery temp ×10
  "1.3.6.1.4.1.318.1.1.1.2.2.8.0",  // 3 Battery voltage
  "1.3.6.1.4.1.318.1.1.1.3.2.1.0",  // 4 Input voltage
  "1.3.6.1.4.1.318.1.1.1.4.2.1.0",  // 5 Output voltage
  "1.3.6.1.4.1.318.1.1.1.4.2.3.0",  // 6 Load %
  "1.3.6.1.4.1.318.1.1.1.4.1.1.0",  // 7 Output status
  "1.3.6.1.4.1.318.1.1.1.1.1.1.0",  // 8 Model
  "1.3.6.1.4.1.318.1.1.1.1.1.2.0",  // 9 Name
  "1.3.6.1.4.1.318.1.1.1.1.2.2.0",  // 10 Manufacture date
  "1.3.6.1.4.1.318.1.1.1.2.1.3.0",  // 11 Last battery replacement
  "1.3.6.1.4.1.318.1.1.1.7.2.4.0"   // 12 Last start time
};

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP multi-OID sensor init");
}

void SnmpSensor::update() {
  const int COUNT = 13;
  long values[COUNT];

  ESP_LOGD(TAG, "SNMP MULTI-GET (13 OID) host=%s community=%s",
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

  // Log
  ESP_LOGI(TAG, "MULTI-GET OK:");
  ESP_LOGI(TAG, "  Runtime: %ld ticks (%.1f s)", values[0], values[0] / 100.0f);
  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values[1]);
  ESP_LOGI(TAG, "  Battery Temp: %.1f C", values[2] / 10.0f);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values[6]);
  ESP_LOGI(TAG, "  Output Status: %ld", values[7]);
  ESP_LOGI(TAG, "  Model: %ld", values[8]);
  ESP_LOGI(TAG, "  Name: %ld", values[9]);
  ESP_LOGI(TAG, "  Manufacture Date: %ld", values[10]);
  ESP_LOGI(TAG, "  Last Battery Repl: %ld", values[11]);
  ESP_LOGI(TAG, "  Last Start Time: %ld", values[12]);

  // 🔥 Publish do Home Assistantu (template senzory)
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
