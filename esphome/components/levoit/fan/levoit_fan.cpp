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

    this->state = (fanMode == 0x00);
    this->speed = reportedManualFanSpeed;

    this->publish_state();
  });
}

void LevoitFan::dump_config() { LOG_FAN("", "Levoit Fan", this); }

fan::FanTraits LevoitFan::get_traits() { return fan::FanTraits(false, true, false, 3); }

void LevoitFan::control(const fan::FanCall &call) {
  if (call.get_state().has_value()) {
    bool newPowerState = *call.get_state();
    if (newPowerState == true) {
      this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE,
                                                .packetType = LevoitPacketType::SEND_MESSAGE,
                                                .payload = {0x00, 0x00}});
    } else {
      this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE,
                                                .packetType = LevoitPacketType::SEND_MESSAGE,
                                                .payload = {0x00, 0x02}});
    }
  }

  if (call.get_speed().has_value()) {
    uint8_t targetSpeed = *call.get_speed();
    this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MANUAL,
                                              .packetType = LevoitPacketType::SEND_MESSAGE,
                                              .payload = {0x00, 0x00, 0x01, targetSpeed}});
  }
}

}  // namespace levoit
}  // namespace esphome
