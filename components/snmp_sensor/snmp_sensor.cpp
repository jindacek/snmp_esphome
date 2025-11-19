#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "Initializing SNMP client...");

  // Každý senzor dostane vlastní port podle hash OID → žádné kolize
  uint16_t port = 40000 + (uint16_t)(std::hash<std::string>{}(oid_) % 20000);

  if (!snmp_.begin(port)) {
    ESP_LOGE(TAG, "Failed to start SNMP client on port %u!", port);
  } else {
    ESP_LOGI(TAG, "SNMP client ready on local port %u", port);
  }
}


void SnmpSensor::update() {

  // ---- Lazy init – první update po WiFi ----
  if (!snmp_initialized_) {
    ESP_LOGI(TAG, "Initializing SNMP client on port 50000...");

    if (!snmp_.begin(50000)) {
      ESP_LOGE(TAG, "Failed to init SNMP on port 50000");
      return;
    }

    snmp_initialized_ = true;
    ESP_LOGI(TAG, "SNMP client ready.");
  }

  // ---- SNMP GET ----
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
