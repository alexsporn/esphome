#include "esphome/core/log.h"
#include "levoit_switch.h"

namespace esphome {
namespace levoit {

static const char *const TAG = "levoit.switch";

void LevoitSwitch::setup() {
  this->parent_->register_listener(LevoitPayloadType::STATUS_RESPONSE, [this](uint8_t *buf, size_t len) {
    if (this->purpose_ == DISPLAY_LOCK) {
      bool currentDisplayLockState = buf[14];
      this->publish_state(currentDisplayLockState);
    }
  });
}

void LevoitSwitch::write_state(bool state) {
  ESP_LOGV(TAG, "Setting switch purposecode %u: %s", (uint8_t) this->purpose_, ONOFF(state));
  if (this->purpose_ == DISPLAY_LOCK) {
    this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_DISPLAY_LOCK,
                                              .packetType = LevoitPacketType::SEND_MESSAGE,
                                              .payload = {0x00, state}});
  }
}

void LevoitSwitch::dump_config() {
  LOG_SWITCH("", "Levoit Switch", this);
  ESP_LOGCONFIG(TAG, "  Switch purposecode: %u", (uint8_t) this->purpose_);
}

}  // namespace levoit
}  // namespace esphome
