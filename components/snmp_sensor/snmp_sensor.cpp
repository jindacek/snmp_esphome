#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include <Arduino_SNMP.h>

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

static SNMPManager snmp;   // SNMP manager instance

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP sensor initialized");

  snmp.begin();  // init SNMP stack
}

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP GET: %s %s %s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  IPAddress target;
  target.fromString(host_.c_str());

  SNMPRequest request(target, community_.c_str(), oid_.c_str());

  // Non-blocking request
  SNMPResponse response = snmp.send(request);

  if (response.error != SNMP_ERR_NOERROR) {
    ESP_LOGW(TAG, "SNMP error: %d", response.error);
    this->publish_state(0);
    return;
  }

  if (response.type == SNMP_TYPE_INTEGER) {
    int value = response.value.integer;
    ESP_LOGI(TAG, "SNMP INTEGER: %d", value);
    this->publish_state(value);
    return;
  }

  ESP_LOGW(TAG, "Unsupported SNMP type %d", response.type);
  this->publish_state(0);
}

}  // namespace snmp_sensor
}  // namespace esphome
