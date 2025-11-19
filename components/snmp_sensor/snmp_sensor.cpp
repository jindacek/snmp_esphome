#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Jeden globální klient
static SnmpClient snmp_client;

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP sensor initialized");
  // NIC NEVOLÁME – UDP se NESMÍ spouštět v setup()
}

void SnmpSensor::update() {
  static bool snmp_initialized = false;

  // Bezpečné opožděné spuštění SNMP UDP:
  if (!snmp_initialized) {
    ESP_LOGI(TAG, "Starting SNMP client on port 3001...");
    snmp_client.begin(3001);
    snmp_initialized = true;
  }

  ESP_LOGD(TAG, "SNMP GET host=%s community=%s oid=%s",
           host_.c_str(), community_.c_str(), oid_.c_str());

  long value = 0;
  bool ok = snmp_client.get(
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
