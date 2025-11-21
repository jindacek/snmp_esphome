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
  const char *oids_num[9] = {
    "1.3.6.1.2.1.1.3.0",                       // 0 Runtime (TimeTicks)
    "1.3.6.1.4.1.318.1.1.1.2.2.1.0",           // 1 Battery capacity
    "1.3.6.1.4.1.318.1.1.1.2.2.2.0",           // 2 Battery temp
    "1.3.6.1.4.1.318.1.1.1.2.2.8.0",           // 3 Battery voltage
    "1.3.6.1.4.1.318.1.1.1.3.2.1.0",           // 4 Input voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.1.0",           // 5 Output voltage
    "1.3.6.1.4.1.318.1.1.1.4.2.3.0",           // 6 Load
    "1.3.6.1.4.1.318.1.1.1.4.1.1.0",           // 7 Output status
    "1.3.6.1.4.1.318.1.1.1.2.2.3.0"            // 8 Remaining runtime (TimeTicks)  
  };

  const char *oids_str[5] = {
    "1.3.6.1.4.1.318.1.1.1.1.1.1.0",           // 0 Model
    "1.3.6.1.4.1.318.1.1.1.1.1.2.0",           // 1 Name
    "1.3.6.1.4.1.318.1.1.1.1.2.2.0",           // 2 Manufacture date
    "1.3.6.1.4.1.318.1.1.1.2.1.3.0",           // 3 Last battery replacement
    "1.3.6.1.4.1.318.1.1.1.7.2.4.0"            // 4 Last start time
  };

  long values_num[9];
  for (int i = 0; i < 9; i++) values_num[i] = -1;

  std::string values_str[5];

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           host_.c_str(), community_.c_str());

  // -------- Numerické hodnoty – batch po 3 --------
  const int BATCH = 3;
  bool any_ok_num = false;

  for (int start = 0; start < 9; start += BATCH) {
    int batch_count = BATCH;
    if (start + batch_count > 9)
      batch_count = 9 - start;

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
  bool ok_str = snmp_.get_many_string(
      host_.c_str(),
      community_.c_str(),
      oids_str,
      5,
      values_str
  );

  if (!any_ok_num && !ok_str) {
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
  ESP_LOGI(TAG, "  Output Status: %ld", values_num[7]);
  long rem_sec = (values_num[8] >= 0) ? (values_num[8] / 100) : -1;
  ESP_LOGI(TAG, "  Remaining Runtime: %ld Sec", rem_sec);
  ESP_LOGI(TAG, "  Remaining Runtime formatted: %s",
           format_runtime_hms(rem_sec).c_str());


  ESP_LOGI(TAG, "  Model: %s", values_str[0].empty() ? "<none>" : values_str[0].c_str());
  ESP_LOGI(TAG, "  Name: %s", values_str[1].empty() ? "<none>" : values_str[1].c_str());

  ESP_LOGI(TAG, "  Manufacture Date: %s",
           values_str[2].empty() ? "<none>" : convert_apc_date(values_str[2]).c_str());

  ESP_LOGI(TAG, "  Last Battery Replacement: %s",
           values_str[3].empty() ? "<none>" : convert_apc_date(values_str[3]).c_str());

  ESP_LOGI(TAG, "  Last Start Time: %s",
           values_str[4].empty() ? "<none>" : convert_apc_date(values_str[4]).c_str());

}

}  // namespace snmp_sensor
}  // namespace esphome
