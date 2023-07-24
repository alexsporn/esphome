#include "esphome/core/log.h"
#include "levoit_select.h"

namespace esphome {
namespace levoit {

static const char *const TAG = "levoit.select";

void LevoitSelect::setup() {
  this->parent_->register_listener(LevoitPayloadType::STATUS_RESPONSE,
                                   [this](const uint8_t *payloadBuf, size_t payloadLen) {
                                     if (this->purpose_ == LevoitSelectPurpose::PURIFIER_FAN_MODE) {
                                       uint8_t purifierFanMode = payloadBuf[5];
                                       if (purifierFanMode == 0x00) {
                                         this->publish_state("Manual");
                                       } else if (purifierFanMode == 0x01) {
                                         this->publish_state("Sleep");
                                       } else if (purifierFanMode == 0x02) {
                                         this->publish_state("Auto");
                                       }
                                     }
                                   });
}

void LevoitSelect::control(const std::string &value) {
  if (value == "Manual") {
    this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE,
                                              .packetType = LevoitPacketType::SEND_MESSAGE,
                                              .payload = {0x00, 0x00}});
  } else if (value == "Sleep") {
    this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE,
                                              .packetType = LevoitPacketType::SEND_MESSAGE,
                                              .payload = {0x00, 0x01}});
  } else if (value == "Auto") {
    this->parent_->send_command(LevoitCommand{.payloadType = LevoitPayloadType::SET_FAN_MODE,
                                              .packetType = LevoitPacketType::SEND_MESSAGE,
                                              .payload = {0x00, 0x02}});
  }
}

void LevoitSelect::dump_config() { LOG_SELECT("", "Levoit Select", this); }

}  // namespace levoit
}  // namespace esphome
