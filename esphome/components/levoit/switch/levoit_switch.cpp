#include "esphome/core/log.h"
#include "levoit_switch.h"

namespace esphome {
namespace levoit {

static const char *const TAG = "levoit.switch";

void LevoitSwitch::setup() {
  if (this->purpose == AUTO_MODE || this->purpose == SLEEP_MODE) {
    this->parent_->register_listener(LevoitPayloadType::STATUS_RESPONSE, [this](uint8_t *buf) {
      uint8_t fanMode = buf[5];
      if (this->purpose == AUTO_MODE) {
        this->publish_state(fanMode == 0x02);
      }
      if (this->purpose == SLEEP_MODE) {
        this->publish_state(fanMode == 0x01);
      }
    });
  }
}

void LevoitSwitch::write_state(bool state) {
  ESP_LOGV(TAG, "Setting switch w/ purpose #%u: %s", (uint8_t) this->purpose, ONOFF(state));
  if (this->purpose == AUTO_MODE || this->purpose == SLEEP_MODE) {
    if (state == true) {
      if (this->purpose == AUTO_MODE) {
        this->parent_->send_command(
            LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE, .payload = {0x00, 0x02}});
      }
      if (this->purpose == SLEEP_MODE) {
        this->parent_->send_command(
            LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE, .payload = {0x00, 0x01}});
      }
    }
  }
  this->publish_state(state);
}

void LevoitSwitch::dump_config() {
  LOG_SWITCH("", "Levoit Switch", this);
  ESP_LOGCONFIG(TAG, "  Switch has purpose #%u", (uint8_t) this->purpose);
}

}  // namespace levoit
}  // namespace esphome
