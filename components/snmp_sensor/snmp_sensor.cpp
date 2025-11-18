#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::update() {
  ESP_LOGW(TAG, "SNMP polling not yet implemented – placeholder");
}

}  // namespace snmp_sensor
}  // namespace esphome
