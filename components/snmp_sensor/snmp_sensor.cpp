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
  const char *oids[13] = {
    "1.3.6.1.4.1.318.1.1.1.2.2.3.0",  // 0 Runtime seconds
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0",  // 1 Battery capacity
    "1.3.6.1.4.1.318.1.1.1.2.2.2.0",  // 2 Battery temp ×10
    "1.3.6.1.4.1.318.1.1.1.2.2.8.0",  // 3 Battery voltage
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",  // 4 Input voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.1.0",  // 5 Output voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.3.0",  // 6 Load %
    "1.3.6.1.4.1.318.1.1.1.4.1.1.0",  // 7 Output status
    "1.3.6.1.4.1.318.1.1.1.1.1.1.0",  // 8 Model
    "1.3.6.1.4.1.318.1.1.1.1.1.2.0",  // 9 Name
    "1.3.6.1.4.1.318.1.1.1.1.2.2.0",  // 10 Manufacture date
    "1.3.6.1.4.1.318.1.1.1.2.1.3.0",  // 11 Last battery replacement
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0"   // 12 Last start time
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

  // Dump všech hodnot
  ESP_LOGI(TAG, "MULTI-GET OK:");
  ESP_LOGI(TAG, "  Runtime: %ld ticks = %.1f s", values[0], values[0] / 100.0f);
  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values[1]);
  ESP_LOGI(TAG, "  Battery Temp: %.1f C", values[2] / 10.0f);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values[6]);
  ESP_LOGI(TAG, "  Output Status: %ld", values[7]);
  ESP_LOGI(TAG, "  Model: %ld", values[8]);
  ESP_LOGI(TAG, "  Name: %ld", values[9]);
  ESP_LOGI(TAG, "  Manufacture Date: %ld", values[10]);
  ESP_LOGI(TAG, "  Last Battery Replacement: %ld", values[11]);
  ESP_LOGI(TAG, "  Last Start Time: %ld", values[12]);

  
  // zatím netlačíme hodnoty do senzorů – jen test
}


}  // namespace snmp_sensor
}  // namespace esphome
