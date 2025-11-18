#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP request: host=%s, community=%s, oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  // TODO: implement SNMP GET
  // zatím jen placeholder
  this->publish_state(0);
}

}  // namespace snmp_sensor
}  // namespace esphome
