#pragma once

#include "esphome/core/component.h"
#include "esphome/components/levoit/levoit.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace levoit {

enum LevoitSwitchPurpose : uint8_t { AUTO_MODE = 1, SLEEP_MODE = 2 };

class LevoitSwitch : public switch_::Switch, public Component {
 public:
  void setup() override;
  void dump_config() override;
  void set_purpose(uint8_t purpose) { this->purpose = (LevoitSwitchPurpose) purpose; }

  void set_levoit_parent(Levoit *parent) { this->parent_ = parent; }

 protected:
  void write_state(bool state) override;

  Levoit *parent_;
  LevoitSwitchPurpose purpose;
};

}  // namespace levoit
}  // namespace esphome
