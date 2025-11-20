#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Globální SNMP klient
static SnmpClient snmp_global;

void SnmpSensor::setup() {
  static bool started = false;

  if (!started) {
    ESP_LOGI(TAG, "Starting global SNMP client...");

    if (!snmp_global.begin(50000)) {
      ESP_LOGE(TAG, "Failed to start SNMP client on port 50000!");
    } else {
      ESP_LOGI(TAG, "SNMP client ready on port 50000");
    }

    started = true;
  }
}

void SnmpSensor::update() {
  ESP_LOGD(TAG, "SNMP GET host=%s community=%s oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  long value = 0;
  bool ok = snmp_global.get(
      host_.c_str(),
      community_.c_str(),
      oid_.c_str(),
      &value
  );

  if (!ok) {
    ESP_LOGW(TAG, "SNMP GET FAILED for oid=%s", oid_.c_str());
    this->publish_state(NAN);
    return;
  }

  ESP_LOGI(TAG, "SNMP OK: %ld", value);
  this->publish_state((float)value);
}

}  // namespace snmp_sensor
}  // namespace esphome
