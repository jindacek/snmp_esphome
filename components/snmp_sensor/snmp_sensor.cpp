#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// -----------------------------------------------------
// Setup – zatím prázdné, jen log
// -----------------------------------------------------
void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP sensor initialized");
}

// -----------------------------------------------------
// Update – zde později bude SNMP GET
// -----------------------------------------------------
void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP request: host=%s, community=%s, oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  // ZATÍM: jen placeholder
  // Ať to kompiluje, funguje a je připravené.
  float dummy_value = 0.0f;

  this->publish_state(dummy_value);
}

}  // namespace snmp_sensor
}  // namespace esphome
