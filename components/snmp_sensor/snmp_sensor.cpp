#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SnmpSensor update() called");
  // tady později doplníme SNMP dotaz
}

}  // namespace snmp_sensor
}  // namespace esphome
