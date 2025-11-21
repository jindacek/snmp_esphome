#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Convert seconds → DDDd-HHh-MMm-SSs
static std::string format_runtime(long seconds) {
  if (seconds < 0) return "unknown";

  long d = seconds / 86400;
  seconds %= 86400;

  long h = seconds / 3600;
  seconds %= 3600;

  long m = seconds / 60;
  long s = seconds % 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%03ldd-%02ldh-%02ldm-%02lds", d, h, m, s);
  return std::string(buf);
}

// Convert seconds -> HHh:MMm:SSs (remaining runtime)
static std::string format_runtime_hms(long seconds) {
  if (seconds < 0) return "unknown";

  long h = seconds / 3600;
  seconds %= 3600;

  long m = seconds / 60;
  long s = seconds % 60;

  char buf[32];
  snprintf(buf, sizeof(buf), "%02ldh:%02ldm:%02lds", h, m, s);
  return std::string(buf);
}

void SnmpSensor::setup() { ESP_LOGI(TAG, "snmp_sensor setup"); }

void SnmpSensor::update() {
  // Odložená inicializace SNMP UDP
  if (!snmp_initialized_) {
    if (!snmp_.begin(50000)) {
      ESP_LOGE(TAG, "SNMP UDP init failed!");
      return;
    }
    ESP_LOGI(TAG, "SNMP UDP initialized (deferred)");
    snmp_initialized_ = true;
  }

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  // -------- NUMERIC OID list (14) --------
  const char *oids_num[14] = {
      "1.3.6.1.2.1.1.3.0",             // 0 Runtime (TimeTicks)
      "1.3.6.1.4.1.318.1.1.1.2.2.1.0", // 1 Battery capacity
      "1.3.6.1.4.1.318.1.1.1.2.2.2.0", // 2 Battery temp
      "1.3.6.1.4.1.318.1.1.1.2.2.8.0", // 3 Battery voltage
      "1.3.6.1.4.1.318.1.1.1.3.2.1.0", // 4 Input voltage
      "1.3.6.1.4.1.318.1.1.1.4.2.1.0", // 5 Output voltage
      "1.3.6.1.4.1.318.1.1.1.4.2.3.0", // 6 Load
      "1.3.6.1.4.1.318.1.1.1.4.1.1.0", // 7 Output status
      "1.3.6.1.4.1.318.1.1.1.2.2.3.0", // 8 Remaining runtime (TimeTicks)
      "1.3.6.1.4.1.318.1.1.1.7.2.3.0", // 9 Last self-test result
      "1.3.6.1.4.1.318.1.1.1.2.2.4.0", // 10 Battery replace status
      "1.3.6.1.4.1.318.1.1.1.4.1.2.0", // 11 Output source
      "1.3.6.1.4.1.318.1.1.1.3.2.4.0", // 12 Input frequency
      "1.3.6.1.4.1.318.1.1.1.4.2.2.0"  // 13 Output frequency
  };

  long values_num[14];
  for (int i = 0; i < 14; i++) values_num[i] = -1;

  // batch get so we don't overflow packets
  const int BATCH = 3;
  bool any_ok_num = false;

  for (int start = 0; start < 14; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > 14) batch_count = 14 - start;

    if (snmp_.get_many(host_.c_str(), community_.c_str(),
                       &oids_num[start], batch_count,
                       &values_num[start])) {
      any_ok_num = true;
    }
  }

  if (!any_ok_num) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED (numeric)");
    return;
  }

  ESP_LOGI(TAG, "MULTI-GET OK:");

  long runtime_sec = (values_num[0] >= 0) ? (values_num[0] / 100) : -1;
  long rem_sec     = (values_num[8] >= 0) ? (values_num[8] / 100) : -1;

  ESP_LOGI(TAG, "  Runtime: %ld Sec", runtime_sec);
  ESP_LOGI(TAG, "  Runtime formatted: %s", format_runtime(runtime_sec).c_str());

  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values_num[1]);
  ESP_LOGI(TAG, "  Battery Temp: %ld C", values_num[2]);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values_num[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values_num[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values_num[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values_num[6]);

  // Output status -> text (jen log)
  long os = values_num[7];
  const char *os_text = "unknown";
  if      (os == 1) os_text = "unknown";
  else if (os == 2) os_text = "onLine";
  else if (os == 3) os_text = "onBattery";
  else if (os == 4) os_text = "onSmartBoost";
  else if (os == 5) os_text = "timedSleeping";
  else if (os == 6) os_text = "softwareBypass";
  else if (os == 7) os_text = "off";
  else if (os == 8) os_text = "rebooting";
  else if (os == 9) os_text = "switchedBypass";
  else if (os == 10) os_text = "hardwareFailureBypass";
  else if (os == 11) os_text = "sleepingUntilPowerReturn";
  ESP_LOGI(TAG, "  Output Status: %ld", os);
  ESP_LOGI(TAG, "  Output Status Text: %s", os_text);

  ESP_LOGI(TAG, "  Remaining Runtime: %ld Sec", rem_sec);
  ESP_LOGI(TAG, "  Remaining Runtime formatted: %s",
           format_runtime_hms(rem_sec).c_str());

  // Self-test result -> text (jen log)
  long st = values_num[9];
  const char *st_text = "unknown";
  if      (st == 1) st_text = "Passed";
  else if (st == 2) st_text = "Failed";
  else if (st == 3) st_text = "InProgress";
  ESP_LOGI(TAG, "  Last Self-Test Result: %ld (%s)", st, st_text);

  // Battery replace -> text (jen log)
  long br = values_num[10];
  const char *br_text = "unknown";
  if      (br == 1) br_text = "OK";
  else if (br == 2) br_text = "Replace battery";
  ESP_LOGI(TAG, "  Battery Replace Status: %ld (%s)", br, br_text);

  ESP_LOGI(TAG, "  Output Source: %ld", values_num[11]);
  ESP_LOGI(TAG, "  Input Frequency: %ld Hz", values_num[12]);
  ESP_LOGI(TAG, "  Output Frequency: %ld Hz", values_num[13]);

  // -------- publish do child senzorů (jen numeric) --------
  if (runtime_sensor_ != nullptr) runtime_sensor_->publish_state(runtime_sec);
  if (battery_capacity_sensor_ != nullptr) battery_capacity_sensor_->publish_state(values_num[1]);
  if (battery_temp_sensor_ != nullptr) battery_temp_sensor_->publish_state(values_num[2]);
  if (battery_voltage_sensor_ != nullptr) battery_voltage_sensor_->publish_state(values_num[3]);
  if (input_voltage_sensor_ != nullptr) input_voltage_sensor_->publish_state(values_num[4]);
  if (output_voltage_sensor_ != nullptr) output_voltage_sensor_->publish_state(values_num[5]);
  if (load_sensor_ != nullptr) load_sensor_->publish_state(values_num[6]);
  if (output_status_sensor_ != nullptr) output_status_sensor_->publish_state(values_num[7]);
  if (remaining_runtime_sensor_ != nullptr) remaining_runtime_sensor_->publish_state(rem_sec);
  if (self_test_result_sensor_ != nullptr) self_test_result_sensor_->publish_state(values_num[9]);
  if (battery_replace_status_sensor_ != nullptr) battery_replace_status_sensor_->publish_state(values_num[10]);
  if (output_source_sensor_ != nullptr) output_source_sensor_->publish_state(values_num[11]);
  if (input_frequency_sensor_ != nullptr) input_frequency_sensor_->publish_state(values_num[12]);
  if (output_frequency_sensor_ != nullptr) output_frequency_sensor_->publish_state(values_num[13]);
}

}  // namespace snmp_sensor
}  // namespace esphome
