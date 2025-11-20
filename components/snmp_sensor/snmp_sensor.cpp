#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "snmp_sensor setup");
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

  // Dump všech hodnot
  ESP_LOGI(TAG, "MULTI-GET OK:");
  ESP_LOGI(TAG, "  Runtime: %ld ticks = %.1f s", values[0], values[0] / 100.0f);
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
  ESP_LOGI(TAG, "  Last Battery Replacement: %ld", values[11]);
  ESP_LOGI(TAG, "  Last Start Time: %ld", values[12]);
}



}  // namespace snmp_sensor
}  // namespace esphome
