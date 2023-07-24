#pragma once

#include "esphome/core/component.h"
#include "esphome/components/levoit/levoit.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace levoit {

enum LevoitSensorPurpose : uint8_t { PM25 = 1 };

class LevoitSensor : public Component, public sensor::Sensor {
 public:
  LevoitSensor(Levoit *parent) : parent_(parent) {}
  void setup() override;
  void dump_config() override;

  void set_purpose(uint8_t purpose) { this->purpose_ = (LevoitSensorPurpose) purpose; }

 protected:
  uint8_t purpose_;
  Levoit *parent_;
};

}  // namespace levoit
}  // namespace esphome
