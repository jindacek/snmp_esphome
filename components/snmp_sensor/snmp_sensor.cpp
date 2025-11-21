#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// ------------------ helpers ------------------

// Convert APC date formats to DD-MM-YYYY
static std::string convert_apc_date(const std::string &raw) {
  if (raw.size() < 8) return "unknown";

  int mm = 0, dd = 0, yy = 0;
  sscanf(raw.c_str(), "%d/%d/%d", &mm, &dd, &yy);

  if (yy < 100) {  // 2-digit year
    if (yy <= 80) yy += 2000;
    else yy += 1900;
  }

  char out[16];
  snprintf(out, sizeof(out), "%02d-%02d-%04d", dd, mm, yy);
  return std::string(out);
}

// Convert seconds → DDDd-HHh-MMm-SSs (total runtime)
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

// Output status mapping (APC)
static std::string map_output_status(long v) {
  switch (v) {
    case 1: return "unknown";
    case 2: return "on-line";
    case 3: return "on-battery";
    case 4: return "on-smart-boost";
    case 5: return "timed-sleep";
    case 6: return "software-bypass";
    case 7: return "off";
    case 8: return "rebooting";
    case 9: return "switched-bypass";
    case 10: return "hardware-failure-bypass";
    case 11: return "sleeping-until-power-return";
    case 12: return "on-smart-trim";
    default: return "unknown";
  }
}

// Self-test result mapping
static std::string map_selftest_result(long v) {
  switch (v) {
    case 1: return "Invalid";
    case 2: return "Passed";
    case 3: return "Failed";
    case 4: return "In Progress";
    default: return "Unknown";
  }
}

// ------------------ component ------------------

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "snmp_sensor setup");
}

void SnmpSensor::update() {

  // odložená init UDP SNMP
  static bool initialized = false;
  if (!initialized) {
    if (!snmp_.begin(50000)) {
      ESP_LOGE(TAG, "SNMP UDP init failed!");
      return;
    }
    ESP_LOGI(TAG, "SNMP UDP initialized (deferred)");
    initialized = true;
  }

  // -------- NUMERIC OID --------
  const char *oids_num[12] = {
    "1.3.6.1.2.1.1.3.0",                       // 0 Runtime total (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0",           // 1 Battery capacity
    "1.3.6.1.4.1.318.1.1.1.2.2.2.0",           // 2 Battery temp
    "1.3.6.1.4.1.318.1.1.1.2.2.8.0",           // 3 Battery voltage
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",           // 4 Input voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.1.0",           // 5 Output voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.3.0",           // 6 Load %
    "1.3.6.1.4.1.318.1.1.1.4.1.1.0",           // 7 Output status (raw)
    "1.3.6.1.4.1.318.1.1.1.2.2.3.0",           // 8 Remaining runtime (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.2.1.1.0",           // 9 Self-test result
    "1.3.6.1.4.1.318.1.1.1.2.1.2.0",           // 10 Self-test duration (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.7.2.3.0"            // 11 Output frequency (Hz)
  };

  long values_num[12];
  for (int i = 0; i < 12; i++) values_num[i] = -1;

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  const int NNUM = 12;
  const int BATCH = 3;
  bool any_ok_num = false;

  for (int start = 0; start < NNUM; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > NNUM)
      batch_count = NNUM - start;

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

  // -------- STRING OID --------
  const char *oids_str[6] = {
    "1.3.6.1.4.1.318.1.1.1.1.1.1.0",           // 0 Model
    "1.3.6.1.4.1.318.1.1.1.1.1.2.0",           // 1 Name
    "1.3.6.1.4.1.318.1.1.1.1.2.2.0",           // 2 Manufacture date
    "1.3.6.1.4.1.318.1.1.1.2.1.3.0",           // 3 Last battery replacement
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0",           // 4 Last start time (string)
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0"            // 5 Last self-test date (string) – stejné OID jak jsi našel
  };

  std::string values_str[6];

  bool ok_str = snmp_.get_many_string(
      host_.c_str(),
      community_.c_str(),
      oids_str,
      6,
      values_str
  );

  if (!any_ok_num && !ok_str) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED (numeric + string)");
    return;
  }

  // -------- LOG --------
  ESP_LOGI(TAG, "MULTI-GET OK:");

  long runtime_sec = (values_num[0] >= 0) ? (values_num[0] / 100) : -1;
  ESP_LOGI(TAG, "  Runtime: %ld Sec", runtime_sec);
  ESP_LOGI(TAG, "  Runtime formatted: %s", format_runtime(runtime_sec).c_str());

  ESP_LOGI(TAG, "  Battery Cap: %ld %%", values_num[1]);
  ESP_LOGI(TAG, "  Battery Temp: %ld C", values_num[2]);
  ESP_LOGI(TAG, "  Battery Voltage: %ld V", values_num[3]);
  ESP_LOGI(TAG, "  Input Voltage: %ld V", values_num[4]);
  ESP_LOGI(TAG, "  Output Voltage: %ld V", values_num[5]);
  ESP_LOGI(TAG, "  Load: %ld %%", values_num[6]);

  long out_status_raw = values_num[7];
  ESP_LOGI(TAG, "  Output Status: %ld (%s)", out_status_raw,
           map_output_status(out_status_raw).c_str());

  long rem_sec = (values_num[8] >= 0) ? (values_num[8] / 100) : -1;
  ESP_LOGI(TAG, "  Remaining Runtime: %ld Sec", rem_sec);
  ESP_LOGI(TAG, "  Remaining Runtime formatted: %s",
           format_runtime_hms(rem_sec).c_str());

  long st_res = values_num[9];
  ESP_LOGI(TAG, "  Self-Test Result: %ld (%s)",
           st_res, map_selftest_result(st_res).c_str());

  long st_dur_sec = (values_num[10] >= 0) ? (values_num[10] / 100) : -1;
  ESP_LOGI(TAG, "  Self-Test Duration: %ld Sec", st_dur_sec);

  ESP_LOGI(TAG, "  Output Frequency: %ld Hz", values_num[11]);

  if (ok_str) {
    ESP_LOGI(TAG, "  Model: %s",
             values_str[0].empty() ? "<none>" : values_str[0].c_str());

    ESP_LOGI(TAG, "  Name: %s",
             values_str[1].empty() ? "<none>" : values_str[1].c_str());

    ESP_LOGI(TAG, "  Manufacture Date: %s",
             values_str[2].empty() ? "<none>" : convert_apc_date(values_str[2]).c_str());

    ESP_LOGI(TAG, "  Last Battery Replacement: %s",
             values_str[3].empty() ? "<none>" : convert_apc_date(values_str[3]).c_str());

    ESP_LOGI(TAG, "  Last Start Time: %s",
             values_str[4].empty() ? "<none>" : convert_apc_date(values_str[4]).c_str());

    ESP_LOGI(TAG, "  Last Self-Test Date: %s",
             values_str[5].empty() ? "<none>" : convert_apc_date(values_str[5]).c_str());
  }
}

}  // namespace snmp_sensor
}  // namespace esphome
