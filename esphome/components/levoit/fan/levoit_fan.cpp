#include "esphome/core/log.h"
#include "levoit_fan.h"

namespace esphome {
namespace levoit {

static const char *const TAG = "levoit.fan";

void LevoitFan::setup() {
  this->parent_->register_listener(LevoitPayloadType::STATUS_RESPONSE, [this](uint8_t *payloadData, size_t payloadLen) {
    bool power = payloadData[4] == 0x01;
    fanMode = payloadData[5];
    reportedManualFanSpeed = payloadData[6];
    currentFanSpeed = payloadData[9];

    if (fanMode == 0) {
      // Manual
      if (reportedManualFanSpeed != 0) {
        this->speed = reportedManualFanSpeed + 1;
      }
    } else if (fanMode == 1) {
      // Sleep
      this->speed = 1;
    } else if (fanMode == 2) {
      // Auto
      if (currentFanSpeed == 255) {
        this->state = false;
      } else {
        this->state = true;
        this->speed = currentFanSpeed + 1;
      }
    }

    this->state = power;

    this->publish_state();
  });
}

void LevoitFan::dump_config() { LOG_FAN("", "Levoit Fan", this); }

fan::FanTraits LevoitFan::get_traits() { return fan::FanTraits(false, true, false, 4); }

void LevoitFan::control(const fan::FanCall &call) {
  // Speed 1 = sleep, 2->4 = low med high
  // When set, change to sleep or manual if from auto.
  if (call.get_state().has_value()) {
    bool newPowerState = *call.get_state();
    this->parent_->send_command(
        LevoitCommand{.payloadType = LevoitPayloadType::SET_POWER_STATE, .packetType = LevoitPacketType::SEND_MESSAGE, .payload = {0x00, newPowerState}});
  }
  
  if (call.get_speed().has_value()) {
    int targetSpeed = *call.get_speed();
    if (targetSpeed == 1) {
      // Sleep mode
      this->parent_->send_command(
          LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE, .packetType = LevoitPacketType::SEND_MESSAGE, .payload = {0x00, 0x01}});
    } else {
      this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MANUAL, .packetType = LevoitPacketType::SEND_MESSAGE,
                                                .payload = {0x00, 0x00, 0x01, (uint8_t) (targetSpeed - 1)}});
    }
  }
}

}  // namespace levoit
}  // namespace esphome
