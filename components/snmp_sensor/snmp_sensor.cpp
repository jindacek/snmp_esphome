#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Jeden společný SNMP klient pro všechny instance
static SnmpClient snmp_client;
static bool snmp_inited = false;

void SnmpSensor::setup() {
  if (!snmp_inited) {
    ESP_LOGI(TAG, "Initializing SNMP client...");

    // Lokální port 50000 – vyhneme se 161, který je v ESP-IDF problémový
    if (!snmp_client.begin(50000)) {
      ESP_LOGE(TAG, "Failed to start SNMP client on port 50000!");
    } else {
      ESP_LOGI(TAG, "SNMP client ready on local port 50000");
      snmp_inited = true;
    }
  }
}

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP GET host=%s community=%s oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  long value = 0;

  bool ok = snmp_client.get(
      host_.c_str(),
      community_.c_str(),
      oid_.c_str(),
      &value);

  if (!ok) {
    ESP_LOGW(TAG, "SNMP GET FAILED for oid=%s", oid_.c_str());
    this->publish_state(NAN);
    return;
  }

  ESP_LOGI(TAG, "SNMP OK: %ld", value);
  this->publish_state((float) value);
}

}  // namespace snmp_sensor
}  // namespace esphome
