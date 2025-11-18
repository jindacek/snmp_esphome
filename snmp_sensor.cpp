#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include <WiFiUdp.h>

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SNMPSensor::update() {
  ESP_LOGI(TAG, "SNMP query to %s, oid=%s", host_.c_str(), oid_.c_str());
  // TODO: zde přijde jednoduchá SNMP implementace
}

}  // namespace snmp_sensor
}  // namespace esphome
