#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "Initializing SNMP client...");

  if (!snmp_.begin(50000)) {
    ESP_LOGE(TAG, "Failed to start SNMP client on port 50000!");
  } else {
    ESP_LOGI(TAG, "SNMP client ready on local port 50000");
  }
}


void SnmpSensor::on_wifi_ready() {
  ESP_LOGI(TAG, "WiFi is ready – starting SNMP client...");

  // Lokální port musí být >1024 (ESP-IDF blokuje 161)
  if (!snmp_.begin(50000)) {
    ESP_LOGE(TAG, "Failed to start SNMP UDP client on port 50000!");
  } else {
    ESP_LOGI(TAG, "SNMP client started on UDP port 50000");
  }
}

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP GET host=%s community=%s oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  long value = 0;

  bool ok = snmp_.get(
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
