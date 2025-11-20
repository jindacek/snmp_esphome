#include "snmp_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace snmp_sensor {

static const char *TAG = "snmp_sensor";

// APC UPS OID – 13 hodnot v pevně daném pořadí
static const char *UPS_OIDS[13] = {
  "1.3.6.1.4.1.318.1.1.1.2.2.3.0",  // 0 Runtime (ticks / 100 = s)
  "1.3.6.1.4.1.318.1.1.1.2.2.1.0",  // 1 Battery capacity (%)
  "1.3.6.1.4.1.318.1.1.1.2.2.2.0",  // 2 Battery temperature (0.1 °C)
  "1.3.6.1.4.1.318.1.1.1.2.2.8.0",  // 3 Battery voltage (V)
  "1.3.6.1.4.1.318.1.1.1.3.2.1.0",  // 4 Input voltage (V)
  "1.3.6.1.4.1.318.1.1.1.4.2.1.0",  // 5 Output voltage (V)
  "1.3.6.1.4.1.318.1.1.1.4.2.3.0",  // 6 Load (%)
  "1.3.6.1.4.1.318.1.1.1.4.1.1.0",  // 7 Output status
  "1.3.6.1.4.1.318.1.1.1.1.1.1.0",  // 8 Model (string / zde jako číslo – viz poznámka)
  "1.3.6.1.4.1.318.1.1.1.1.1.2.0",  // 9 Name
  "1.3.6.1.4.1.318.1.1.1.1.2.2.0",  // 10 Manufacture date
  "1.3.6.1.4.1.318.1.1.1.2.1.3.0",  // 11 Last battery replacement
  "1.3.6.1.4.1.318.1.1.1.7.2.4.0"   // 12 Last start time
};

void SnmpSensor::setup() {
  ESP_LOGI(TAG, "SNMP multi-OID sensor init (host=%s)", host_.c_str());

  // Inicializace UDP klienta pro SNMP
  if (!this->snmp_.begin(161)) {
    ESP_LOGE(TAG, "SNMP UDP begin failed on port 161");
    this->mark_failed();
  }
}

void SnmpSensor::update() {
  const int COUNT = 13;
  long values[COUNT];

  ESP_LOGD(TAG, "SNMP MULTI-GET host=%s community=%s",
           this->host_.c_str(), this->community_.c_str());

  bool ok = this->snmp_.get_many(
      this->host_.c_str(),
      this->community_.c_str(),
      UPS_OIDS,
      COUNT,
      values
  );

  if (!ok) {
    ESP_LOGW(TAG, "SNMP MULTI-GET FAILED");
    return;
  }

  ESP_LOGD(TAG, "SNMP MULTI-GET OK:"
                " runtime=%ld, cap=%ld, temp=%ld, battV=%ld, inV=%ld, outV=%ld,"
                " load=%ld, status=%ld, model=%ld, name=%ld, mfg=%ld, repl=%ld, start=%ld",
           values[0], values[1], values[2], values[3], values[4], values[5],
           values[6], values[7], values[8], values[9], values[10], values[11], values[12]);

  // Publikace do jednotlivých senzorů – pokud jsou připojené v YAML

  // Runtime – ticks / 100 → sekundy
  if (this->runtime_sensor_ != nullptr)
    this->runtime_sensor_->publish_state(values[0] / 100.0f);

  // Battery capacity (%)
  if (this->battery_capacity_sensor_ != nullptr)
    this->battery_capacity_sensor_->publish_state(values[1]);

  // Battery temperature – 0.1°C → °C
  if (this->battery_temp_sensor_ != nullptr)
    this->battery_temp_sensor_->publish_state(values[2] / 10.0f);

  // Battery voltage (V)
  if (this->battery_voltage_sensor_ != nullptr)
    this->battery_voltage_sensor_->publish_state(values[3]);

  // Input voltage (V)
  if (this->input_voltage_sensor_ != nullptr)
    this->input_voltage_sensor_->publish_state(values[4]);

  // Output voltage (V)
  if (this->output_voltage_sensor_ != nullptr)
    this->output_voltage_sensor_->publish_state(values[5]);

  // Load (%)
  if (this->load_sensor_ != nullptr)
    this->load_sensor_->publish_state(values[6]);

  // Output status (raw code)
  if (this->output_status_sensor_ != nullptr)
    this->output_status_sensor_->publish_state(values[7]);

  // Model – POZOR: v SNMP je to OCTET STRING, aktuálně interpretováno jako číslo
  if (this->model_sensor_ != nullptr)
    this->model_sensor_->publish_state(values[8]);

  // Name – opět typicky STRING, teď jen jako číslo
  if (this->name_sensor_ != nullptr)
    this->name_sensor_->publish_state(values[9]);

  // Manufacture date – STRING/Date, zde raw číslo
  if (this->manufacture_date_sensor_ != nullptr)
    this->manufacture_date_sensor_->publish_state(values[10]);

  // Last battery replacement – STRING/Date
  if (this->last_battery_replacement_sensor_ != nullptr)
    this->last_battery_replacement_sensor_->publish_state(values[11]);

  // Last start time – STRING/Date
  if (this->last_start_time_sensor_ != nullptr)
    this->last_start_time_sensor_->publish_state(values[12]);
}

}  // namespace snmp_sensor
}  // namespace esphome
