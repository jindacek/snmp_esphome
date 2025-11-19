#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

SnmpClient global_snmp_client;

void SnmpSensor::setup() {
  static bool started = false;

  if (!started) {
    ESP_LOGI(TAG, "Starting global SNMP client on port 50000...");
    if (!global_snmp_client.begin(50000)) {
      ESP_LOGE(TAG, "Failed to start global SNMP client!");
    } else {
      ESP_LOGI(TAG, "Global SNMP client READY");
    }
    started = true;
  }
}

void SnmpSensor::update() {
  long value = 0;

  ESP_LOGD(TAG, "SNMP GET host=%s oid=%s", host_.c_str(), oid_.c_str());

  bool ok = global_snmp_client.get(
      host_.c_str(),
      community_.c_str(),
      oid_.c_str(),
      &value);

  if (!ok) {
    ESP_LOGW(TAG, "SNMP GET FAILED for oid=%s", oid_.c_str());
    publish_state(NAN);
    return;
  }

  ESP_LOGI(TAG, "SNMP OK: %ld", value);
  publish_state((float)value);
}

}  // namespace snmp_sensor
}  // namespace esphome
