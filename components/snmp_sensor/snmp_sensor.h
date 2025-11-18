#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace snmp_sensor {

class SnmpSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override;
};

}  // namespace snmp_sensor
}  // namespace esphome
