#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include "snmp_client.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Jeden globální SNMP klient pro všechny instance
static SnmpClient snmp_client;

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "Initializing SNMP client...");
  snmp_.begin(3001);  // místo defaultu 161 použijeme 3001
}

void SnmpSensor::update() {
  static bool snmp_inited = false;

  // Lazy inicializace SNMP klienta – až při prvním update
  if (!snmp_inited) {
    ESP_LOGI(TAG, "Initializing SNMP client...");
    snmp_.begin(3001);        // používá WiFiUDP::begin(...)
    snmp_inited = true;
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
