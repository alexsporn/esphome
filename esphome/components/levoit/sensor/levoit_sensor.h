#pragma once

#include "esphome/core/component.h"
#include "esphome/components/levoit/levoit.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace levoit {

enum LevoitSensorPurpose : uint8_t { NONE, PM25 };

class LevoitSensor : public Component, public sensor::Sensor {
 public:
  LevoitSensor(Levoit *parent, LevoitSensorPurpose purpose) : parent_(parent), purpose_(purpose) {}
  void setup() override;
  void dump_config() override;

 protected:
  LevoitSensorPurpose purpose_;
  Levoit *parent_;
};

}  // namespace levoit
}  // namespace esphome
