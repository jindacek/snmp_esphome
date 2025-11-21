#include "snmp_sensor.h"
#include "esphome/core/log.h"

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

  // OID seznam
  const char *oids_num[14] = {
    "1.3.6.1.2.1.1.3.0",                       // 0 Runtime (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0",           // 1 Battery capacity
    "1.3.6.1.4.1.318.1.1.1.2.2.2.0",           // 2 Battery temp
    "1.3.6.1.4.1.318.1.1.1.2.2.8.0",           // 3 Battery voltage
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",           // 4 Input voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.1.0",           // 5 Output voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.3.0",           // 6 Load
    "1.3.6.1.4.1.318.1.1.1.4.1.1.0",           // 7 Output status
    "1.3.6.1.4.1.318.1.1.1.2.2.3.0",           // 8 Remaining runtime (TimeTicks) 
    "1.3.6.1.4.1.318.1.1.1.7.2.3.0",           // 9 Self test result
    "1.3.6.1.4.1.318.1.1.1.2.2.4.0",           // 10 Battery replace indicator
    "1.3.6.1.4.1.318.1.1.1.4.1.2.0",           // 11 Output Source (integer)
    "1.3.6.1.4.1.318.1.1.1.3.2.4.0",           // 12 Input frequency
    "1.3.6.1.4.1.318.1.1.1.4.2.2.0"            // 13 Output frequency
  };

  const char *oids_str[6] = {
    "1.3.6.1.4.1.318.1.1.1.1.1.1.0",           // 0 Model
    "1.3.6.1.4.1.318.1.1.1.1.1.2.0",           // 1 Name
    "1.3.6.1.4.1.318.1.1.1.1.2.2.0",           // 2 Manufacture date
    "1.3.6.1.4.1.318.1.1.1.2.1.3.0",           // 3 Last battery replacement
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0",           // 4 Last self test date
    "1.3.6.1.4.1.318.1.1.1.1.2.3.0"            // 5 Serial number
  };

  long values_num[14];
  for (int i = 0; i < 14; i++) values_num[i] = -1;

  std::string values_str[6];

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  // -------- Numerické hodnoty – batch po 3 --------
  const int BATCH = 3;
  bool any_ok_num = false;

  for (int start = 0; start < 14; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > 14)
      batch_count = 14 - start;

    const char *batch_oids[BATCH];
    long batch_vals[BATCH];

    for (int i = 0; i < batch_count; i++) {
      batch_oids[i] = oids_num[start + i];
      batch_vals[i] = -1;
    }

    bool ok = snmp_.get_many(
        host_.c_str(),
        community_.c_str(),
        batch_oids,
        batch_count,
        batch_vals
    );

    if (!ok) {
      ESP_LOGW(TAG, "SNMP MULTI-GET (numeric) batch %d..%d FAILED",
               start, start + batch_count - 1);
      continue;
    }

    any_ok_num = true;

    for (int i = 0; i < batch_count; i++) {
      values_num[start + i] = batch_vals[i];
    }
  }

  // -------- Stringové hodnoty – jedním dotazem --------
// -------- Stringové hodnoty – BATCH 3+3 OID --------
const int NSTR = 6;
const int BATCH_STR = 3;
bool any_ok_str = false;

for (int start = 0; start < NSTR; start += BATCH_STR) {
  int batch_count = BATCH_STR;
  if (start + batch_count > NSTR)
    batch_count = NSTR - start;

  const char *batch_oids[BATCH_STR];
  std::string batch_vals[BATCH_STR];

  for (int i = 0; i < batch_count; i++) {
    batch_oids[i] = oids_str[start + i];
    batch_vals[i] = "";
  }

  bool ok = snmp_.get_many_string(
      host_.c_str(),
      community_.c_str(),
      batch_oids,
      batch_count,
      batch_vals
  );

  if (!ok) {
    ESP_LOGW(TAG, "SNMP MULTI-GET (string) batch %d..%d FAILED",
             start, start + batch_count - 1);
    continue;
  }

  any_ok_str = true;

  // zkopírovat výsledky zpět
  for (int i = 0; i < batch_count; i++) {
    values_str[start + i] = batch_vals[i];
  }
}


  if (!any_ok_num && !any_ok_str) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED (numeric + string)");
    return;
  }

  ESP_LOGI(TAG, "MULTI-GET OK:");

  long runtime_sec = (values_num[0] >= 0) ? (values_num[0] / 100) : -1;
  ESP_LOGI(TAG, "  Runtime: %ld Sec", runtime_sec);
  ESP_LOGI(TAG, "  Runtime formatted: %s", format_runtime(runtime_sec).c_str());

  //ESP_LOGI(TAG, "  Runtime: %ld Sec", runtime_sec);
  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values_num[1]);
  ESP_LOGI(TAG, "  Battery Temp: %ld C", values_num[2]);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values_num[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values_num[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values_num[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values_num[6]);

  if (battery_voltage_sensor_ != nullptr && values_num[3] >= 0)
      battery_voltage_sensor_->publish_state(values_num[3]);

  if (input_voltage_sensor_ != nullptr && values_num[4] >= 0)
      input_voltage_sensor_->publish_state(values_num[4]);

  if (output_voltage_sensor_ != nullptr && values_num[5] >= 0)
      output_voltage_sensor_->publish_state(values_num[5]);

  if (load_sensor_ != nullptr && values_num[6] >= 0)
      load_sensor_->publish_state(values_num[6]);

  
  ESP_LOGI(TAG, "  Output Status: %ld", values_num[7]);
  // Output Status (index 7)
  long os2 = values_num[7];
  const char *os2_text = "unknown";
  //--------------------------------
  if      (os2 == 1)  os2_text = "unknown";
  else if (os2 == 2)  os2_text = "onLine";
  else if (os2 == 3)  os2_text = "onBattery";
  else if (os2 == 4)  os2_text = "onSmartBoost";
  else if (os2 == 5)  os2_text = "timedSleep";
  else if (os2 == 6)  os2_text = "softwareSleep";
  else if (os2 == 7)  os2_text = "off";
  else if (os2 == 8)  os2_text = "rebooting";
  else if (os2 == 9)  os2_text = "switchedByCommand";
  else if (os2 == 10) os2_text = "onBypass";
  else if (os2 == 11) os2_text = "reducingVoltage";
  else if (os2 == 12) os2_text = "switchingToBypass";
  else if (os2 == 13) os2_text = "onSmartTrim";
  ESP_LOGI(TAG, "  Output Status Text: %s", os2_text);
  // Remaining runtime (index 8)
  long rem_sec = (values_num[8] >= 0) ? (values_num[8] / 100) : -1;
  ESP_LOGI(TAG, "  Remaining Runtime: %ld Sec", rem_sec);
  ESP_LOGI(TAG, "  Remaining Runtime formatted: %s",
           format_runtime_hms(rem_sec).c_str());
  
  // Self-test result (index 9)
  long st = values_num[9];
  const char *st_text = "unknown";
  //--------------------------------
  if      (st == 1) st_text = "Passed";
  else if (st == 2) st_text = "Failed";
  else if (st == 3) st_text = "Invalid";
  else if (st == 4) st_text = "In progress";
  else if (st == 5) st_text = "Aborted";
  else if (st == 6) st_text = "Not supported"; 
  ESP_LOGI(TAG, "  Last Self-Test Result: %ld (%s)", st, st_text);
  
  // Battery replace indicator (index 10)
  long br = values_num[10];
  const char *br_text = "unknown";
  //--------------------------------
  if      (br == 1) br_text = "OK";
  else if (br == 2) br_text = "Replace battery";
  ESP_LOGI(TAG, "  Battery Replace Status: %ld (%s)", br, br_text);

  // Output Source
  long os = values_num[11];
  const char *os_text = "unknown";
  //--------------------------------
  if      (os == 1)  os_text = "unknown";
  else if (os == 2)  os_text = "onLine";
  else if (os == 3)  os_text = "onBattery";
  else if (os == 4)  os_text = "onSmartBoost";
  else if (os == 5)  os_text = "timedSleep";
  else if (os == 6)  os_text = "softwareSleep";
  else if (os == 7)  os_text = "off";
  else if (os == 8)  os_text = "rebooting";
  else if (os == 9)  os_text = "switchedByCommand";
  else if (os == 10) os_text = "onBypass";
  else if (os == 11) os_text = "reducingVoltage";
  else if (os == 12) os_text = "switchingToBypass";
  else if (os == 13) os_text = "onSmartTrim";
  ESP_LOGI(TAG, "  Output Source: %ld (%s)", os, os_text);
  // Input Frequency (index 12)
  ESP_LOGI(TAG, "  Input Frequency: %ld Hz", values_num[12]);
  // Output Frequency (index 13)
  ESP_LOGI(TAG, "  Output Frequency: %ld Hz", values_num[13]); 



  ESP_LOGI(TAG, "  Model: %s", values_str[0].empty() ? "<none>" : values_str[0].c_str());
  ESP_LOGI(TAG, "  Name: %s", values_str[1].empty() ? "<none>" : values_str[1].c_str());

  ESP_LOGI(TAG, "  Manufacture Date: %s",
           values_str[2].empty() ? "<none>" : convert_apc_date(values_str[2]).c_str());

  ESP_LOGI(TAG, "  Last Battery Replacement: %s",
           values_str[3].empty() ? "<none>" : convert_apc_date(values_str[3]).c_str());

  ESP_LOGI(TAG, "  Last Self Test: %s",
           values_str[4].empty() ? "<none>" : convert_apc_date(values_str[4]).c_str());

  ESP_LOGI(TAG, "  Serial Number: %s",
           values_str[5].empty() ? "<none>" : values_str[5].c_str());
  

}

}  // namespace snmp_sensor
}  // namespace esphome
