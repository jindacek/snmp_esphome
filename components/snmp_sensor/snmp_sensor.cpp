#include "snmp_sensor.h"
#include "esphome/core/log.h"
#include "esphome/components/binary_sensor/binary_sensor.h"


namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// Convert seconds -> HHh:MMm:SSs (pro remaining runtime)
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

// Convert APC date formats to DD-MM-YYYY
static std::string convert_apc_date(const std::string &raw) {
  if (raw.size() < 8) return raw;

  int mm = 0, dd = 0, yy = 0;
  sscanf(raw.c_str(), "%d/%d/%d", &mm, &dd, &yy);

  if (yy < 100) {
    if (yy <= 80) yy += 2000;
    else yy += 1900;
  }

  char out[16];
  snprintf(out, sizeof(out), "%02d-%02d-%04d", dd, mm, yy);
  return std::string(out);
}

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

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "snmp_sensor setup");
}

void SnmpSensor::update() {
  // Odložená inicializace SNMP UDP
  static bool initialized = false;
  if (!initialized) {
    if (!snmp_.begin(50000)) {
      ESP_LOGE(TAG, "SNMP UDP init failed!");
      return;
    }
    ESP_LOGI(TAG, "SNMP UDP initialized (deferred)");
    initialized = true;
  }

  // ---- numeric OIDs ----
  const char *oids_num[14] = {
    "1.3.6.1.2.1.1.3.0",                       // 0 Runtime (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0",           // 1 Battery capacity
    "1.3.6.1.4.1.318.1.1.1.2.2.2.0",           // 2 Battery temp
    "1.3.6.1.4.1.318.1.1.1.2.2.8.0",           // 3 Battery voltage
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",           // 4 Input voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.1.0",           // 5 Output voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.3.0",           // 6 Load %
    "1.3.6.1.4.1.318.1.1.1.4.1.1.0",           // 7 Output status
    "1.3.6.1.4.1.318.1.1.1.2.2.3.0",           // 8 Remaining runtime (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.7.2.3.0",           // 9 Last self-test result (int)
    "1.3.6.1.4.1.318.1.1.1.2.2.4.0",           // 10 Battery replace status (int)
    "1.3.6.1.4.1.318.1.1.1.4.1.2.0",           // 11 Output source (int)
    "1.3.6.1.4.1.318.1.1.1.3.2.4.0",           // 12 Input freq
    "1.3.6.1.4.1.318.1.1.1.4.2.2.0"            // 13 Output freq
  };

  long values_num[14];
  for (int i = 0; i < 14; i++) values_num[i] = -1;

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  const int BATCH = 3;
  bool any_ok_num = false;

  for (int start = 0; start < 14; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > 14)
      batch_count = 14 - start;

    bool ok_batch = snmp_.get_many(
        host_.c_str(),
        community_.c_str(),
        &oids_num[start],
        batch_count,
        &values_num[start]);

    if (ok_batch) any_ok_num = true;
  }

  // ---- string OIDs ----
  const char *oids_str[6] = {
    "1.3.6.1.4.1.318.1.1.1.1.1.1.0",  // 0 Model
    "1.3.6.1.4.1.318.1.1.1.1.1.2.0",  // 1 Name
    "1.3.6.1.4.1.318.1.1.1.1.2.2.0",  // 2 Manufacture date
    "1.3.6.1.4.1.318.1.1.1.2.1.3.0",  // 3 Last battery replacement
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0",  // 4 Last self-test date
    "1.3.6.1.4.1.318.1.1.1.1.2.3.0"   // 5 Serial number
  };

  std::string values_str[6];
  bool any_ok_str = false;

  for (int start = 0; start < 6; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > 6)
      batch_count = 6 - start;

    bool ok_batch = snmp_.get_many_string(
        host_.c_str(),
        community_.c_str(),
        &oids_str[start],
        batch_count,
        &values_str[start]);

    if (ok_batch) any_ok_str = true;
  }

  if (!any_ok_num && !any_ok_str) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED (numeric + string)");
    return;
  }

  ESP_LOGI(TAG, "MULTI-GET OK:");

  // ---- runtime ----
  long runtime_sec = (values_num[0] >= 0) ? (values_num[0] / 100) : -1;
  std::string runtime_fmt = format_runtime(runtime_sec);

  ESP_LOGI(TAG, "  Runtime: %ld Sec", runtime_sec);
  ESP_LOGI(TAG, "  Runtime formatted: %s", runtime_fmt.c_str());

  // ---- remaining runtime ----
  long rem_sec = (values_num[8] >= 0) ? (values_num[8] / 100) : -1;
  std::string rem_fmt = format_runtime_hms(rem_sec);

  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values_num[1]);
  ESP_LOGI(TAG, "  Battery Temp: %ld C", values_num[2]);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values_num[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values_num[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values_num[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values_num[6]);

  long os = values_num[7];
  ESP_LOGI(TAG, "  Output Status: %ld", os);

  ESP_LOGI(TAG, "  Remaining Runtime: %ld Sec", rem_sec);
  ESP_LOGI(TAG, "  Remaining Runtime formatted: %s", rem_fmt.c_str());

  // ---- Self test result mapping ----
  long st = values_num[9];
  const char *st_text = "unknown";
  if      (st == 1) st_text = "Passed";
  else if (st == 2) st_text = "Failed";
  else if (st == 3) st_text = "Invalid test";
  ESP_LOGI(TAG, "  Last Self-Test Result: %ld (%s)", st, st_text);

  // ---- Battery replace mapping ----
  long br = values_num[10];
  const char *br_text = "unknown";
  if      (br == 1) br_text = "OK";
  else if (br == 2) br_text = "Replace battery";
  ESP_LOGI(TAG, "  Battery Replace Status: %ld (%s)", br, br_text);

  // ---- Output status -> text ----
  const char *os_text = "unknown";
  if      (os == 1)  os_text = "unknown";
  else if (os == 2)  os_text = "onLine";
  else if (os == 3)  os_text = "onBattery";
  else if (os == 4)  os_text = "onSmartBoost";
  else if (os == 5)  os_text = "timedSleeping";
  else if (os == 6)  os_text = "softwareBypass";
  else if (os == 7)  os_text = "off";
  else if (os == 8)  os_text = "rebooting";
  else if (os == 9)  os_text = "switchedBypass";
  else if (os == 10) os_text = "hardwareFailureBypass";
  else if (os == 11) os_text = "sleepingUntilPowerReturn";
  else if (os == 12) os_text = "onSmartTrim";

  ESP_LOGI(TAG, "  Output Status Text: %s", os_text);

  // ---- Output source ----
  ESP_LOGI(TAG, "  Output Source: %ld", values_num[11]);
  ESP_LOGI(TAG, "  Input Frequency: %ld Hz", values_num[12]);
  ESP_LOGI(TAG, "  Output Frequency: %ld Hz", values_num[13]);

  // ---- text values ----
  std::string model        = values_str[0];
  std::string ups_name     = values_str[1];
  std::string manu_date    = convert_apc_date(values_str[2]);
  std::string batt_repl    = convert_apc_date(values_str[3]);
  std::string last_test    = convert_apc_date(values_str[4]);
  std::string serial       = values_str[5];

  ESP_LOGI(TAG, "  Model: %s", model.c_str());
  ESP_LOGI(TAG, "  Name: %s", ups_name.c_str());
  ESP_LOGI(TAG, "  Manufacture Date: %s", manu_date.c_str());
  ESP_LOGI(TAG, "  Last Battery Replacement: %s", batt_repl.c_str());
  ESP_LOGI(TAG, "  Last Self Test: %s", last_test.c_str());
  ESP_LOGI(TAG, "  Serial Number: %s", serial.c_str());

  // -------- publish NUM --------
  if (runtime_sensor_) runtime_sensor_->publish_state(runtime_sec);
  if (battery_capacity_sensor_) battery_capacity_sensor_->publish_state(values_num[1]);
  if (battery_temp_sensor_) battery_temp_sensor_->publish_state(values_num[2]);
  if (battery_voltage_sensor_) battery_voltage_sensor_->publish_state(values_num[3]);
  if (input_voltage_sensor_) input_voltage_sensor_->publish_state(values_num[4]);
  if (output_voltage_sensor_) output_voltage_sensor_->publish_state(values_num[5]);
  if (load_sensor_) load_sensor_->publish_state(values_num[6]);
  if (output_status_sensor_) output_status_sensor_->publish_state(os);
  if (remaining_runtime_sensor_) remaining_runtime_sensor_->publish_state(rem_sec);
  if (self_test_result_sensor_) self_test_result_sensor_->publish_state(st);
  if (battery_replace_status_sensor_) battery_replace_status_sensor_->publish_state(br);
  if (output_source_sensor_) output_source_sensor_->publish_state(values_num[11]);
  if (input_frequency_sensor_) input_frequency_sensor_->publish_state(values_num[12]);
  if (output_frequency_sensor_) output_frequency_sensor_->publish_state(values_num[13]);

  // -------- publish BINARY (odvozené) --------
  if (on_battery_binary_sensor_) on_battery_binary_sensor_->publish_state(os == 3);
  if (online_binary_sensor_) online_binary_sensor_->publish_state(os == 2);

  // -------- publish TEXT --------
  if (model_text_sensor_) model_text_sensor_->publish_state(model);
  if (ups_name_text_sensor_) ups_name_text_sensor_->publish_state(ups_name);
  if (manufacture_date_text_sensor_) manufacture_date_text_sensor_->publish_state(manu_date);
  if (last_battery_replacement_text_sensor_) last_battery_replacement_text_sensor_->publish_state(batt_repl);
  if (last_self_test_text_sensor_) last_self_test_text_sensor_->publish_state(last_test);
  if (serial_number_text_sensor_) serial_number_text_sensor_->publish_state(serial);

  if (output_status_text_sensor_) output_status_text_sensor_->publish_state(os_text);
  if (runtime_formatted_text_sensor_) runtime_formatted_text_sensor_->publish_state(runtime_fmt);
  if (remaining_runtime_formatted_text_sensor_) remaining_runtime_formatted_text_sensor_->publish_state(rem_fmt);
}

}  // namespace snmp_sensor
}  // namespace esphome
