#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "snmp_sensor setup");
}


void SnmpSensor::update() {

  // 🟢 ODLOŽENÁ INITIALIZACE SNMP UDP – bezpečné místo!
  static bool initialized = false;
  if (!initialized) {
    if (!snmp_.begin(50000)) {
      ESP_LOGE(TAG, "SNMP UDP init failed!");
      return;
    }
    ESP_LOGI(TAG, "SNMP UDP initialized (deferred)");
    initialized = true;
  }

  // 🔥 PROTOTYP: jeden multi-OID dotaz se dvěma OID
  const char *oids[2] = {
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",  // Input Voltage
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0"   // Battery Capacity
  };
  long values[2] = {0, 0};

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  bool ok = snmp_.get_many(
      host_.c_str(),
      community_.c_str(),
      oids,
      2,
      values
  );

  if (!ok) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED");
    return;
  }

  ESP_LOGI(TAG, "SNMP MULTI OK: voltage=%ld capacity=%ld",
           values[0], values[1]);

  // zatím netlačíme hodnoty do senzorů – jen test
}


}  // namespace snmp_sensor
}  // namespace esphome
