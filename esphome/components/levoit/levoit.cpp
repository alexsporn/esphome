#include "levoit.h"
#include "esphome/components/network/util.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/util.h"

namespace esphome {
namespace levoit {

static const char *const TAG = "levoit";
static const int COMMAND_DELAY = 10;
static const int RECEIVE_TIMEOUT = 300;
static const int MAX_RETRIES = 5;

void Levoit::setup() {
  this->set_interval("heartbeat", 15000, [this] { this->send_empty_command(LevoitPayloadType::STATUS_REQUEST); });
}

void Levoit::loop() {
  while (this->available()) {
    uint8_t c;
    this->read_byte(&c);
    this->handle_char_(c);
  }
  process_command_queue_();
}

void Levoit::dump_config() { ESP_LOGCONFIG(TAG, "Levoit!"); }

bool Levoit::validate_message_() {
  uint32_t at = this->rx_message_.size() - 1;
  auto *data = &this->rx_message_[0];
  uint8_t new_byte = data[at];

  // Byte 0: HEADER (always 0xA5)
  if (at == 0)
    return new_byte == 0xA5;

  // Byte 1: Packet Type (always 0x12 or 0x22 or 0x52)
  if (at == 1)
    return (new_byte == 0x12) || (new_byte == 0x22) || (new_byte == 0x52);

  // Byte 2: SequenceNumber
  uint8_t sequenceNumber = data[2];
  if (at == 2)
    return true;

  // Byte 3: PayloadLength
  uint8_t payloadLength = data[3];
  if (at == 3)
    return true;

  // Byte 4: Seperator (always 0x00)
  if (at == 4)
    return (new_byte == 0x00);

  // Byte 5: Checksum
  uint8_t payloadChecksum = data[5];
  if (at == 5) {
    return true;
  }

  // wait until all data is read
  if (at - 6 < payloadLength)
    return true;

  // Check the checksum
  uint8_t calc_checksum = 255;
  for (uint8_t i = 0; i < 6 + payloadLength; i++) {
    if (i != 5) {
      calc_checksum -= data[i];
    }
  }

  if (payloadChecksum != calc_checksum) {
    ESP_LOGW(TAG, "Received invalid message checksum %02X!=%02X", payloadChecksum, calc_checksum);
    return false;
  }

  // valid message
  const uint8_t *message_data = data + 6;

  LevoitPayloadType payloadType =
      (LevoitPayloadType) (message_data[0] | (message_data[1] << 8) | (message_data[2] << 16) | (0x00 << 24));

  uint8_t *payload_data = data + 9;

  ESP_LOGV(TAG, "Received packet: Type=%u DATA=[%s]", (uint32_t) payloadType,
           format_hex_pretty(payload_data, payloadLength - 3).c_str());
  this->handle_payload_(payloadType, sequenceNumber, payload_data, payloadLength - 3);

  // return false to reset rx buffer
  return false;
}

void Levoit::handle_char_(uint8_t c) {
  this->rx_message_.push_back(c);
  if (!this->validate_message_()) {
    this->rx_message_.clear();
  } else {
    this->last_rx_char_timestamp_ = millis();
  }
}

void Levoit::handle_payload_(LevoitPayloadType type, uint8_t sequenceNumber, uint8_t *payload, size_t len) {
  // Run through listeners
  for (auto &listener : this->listeners_) {
    if (listener.type == type)
      listener.func(payload);
  }

  // Acknowledge packet
  this->sequenceNumber = sequenceNumber;
  LevoitCommand acknowledgeResponse = {.payloadType = type, .payload = {0x00}};
  this->send_raw_command(acknowledgeResponse);
}

void Levoit::register_listener(LevoitPayloadType payloadType, const std::function<void(uint8_t *buf)> &func) {
  auto listener = LevoitListener{
      .type = payloadType,
      .func = func,
  };
  this->listeners_.push_back(listener);
}

void Levoit::send_raw_command(LevoitCommand command) {
  this->last_command_timestamp_ = millis();

  ESP_LOGI(TAG, "Sending Levoit packet: CMD=%u DATA=[%s]", static_cast<uint32_t>(command.payloadType),
           format_hex_pretty(command.payload).c_str());

  uint8_t payloadTypeByte1 = ((uint32_t) command.payloadType >> 16) & 0xff;
  uint8_t payloadTypeByte2 = ((uint32_t) command.payloadType >> 8) & 0xff;
  uint8_t payloadTypeByte3 = (uint32_t) command.payloadType & 0xff;

  uint8_t checksum = 255;

  checksum -= 0xA5;
  checksum -= (uint8_t) LevoitPacketType::SEND_MESSAGE;
  checksum -= sequenceNumber;
  checksum -= (command.payload.size() + 3);
  checksum -= payloadTypeByte1;
  checksum -= payloadTypeByte2;
  checksum -= payloadTypeByte3;

  for (auto &data : command.payload) {
    checksum -= data;
  }

  std::vector<uint8_t> rawPacket = {0xA5,
                                    (uint8_t) LevoitPacketType::SEND_MESSAGE,
                                    sequenceNumber,
                                    (uint8_t) (command.payload.size() + 3),
                                    0x00,
                                    checksum,
                                    payloadTypeByte1,
                                    payloadTypeByte2,
                                    payloadTypeByte3};

  if (!command.payload.empty())
    rawPacket.insert(rawPacket.end(), command.payload.begin(), command.payload.end());

  ESP_LOGV(TAG, "Raw packet data: %s", format_hex_pretty(rawPacket).c_str());

  this->write_array(rawPacket);

  sequenceNumber++;
}

void Levoit::process_command_queue_() {
  uint32_t now = millis();
  uint32_t delay = now - this->last_command_timestamp_;

  if (now - this->last_rx_char_timestamp_ > RECEIVE_TIMEOUT) {
    this->rx_message_.clear();
  }

  // Left check of delay since last command in case there's ever a command sent by calling send_raw_command_ directly
  if (delay > COMMAND_DELAY && !this->command_queue_.empty() && this->rx_message_.empty()) {
    this->send_raw_command(command_queue_.front());
    this->command_queue_.erase(command_queue_.begin());
  }
}

void Levoit::send_command(const LevoitCommand &command) {
  command_queue_.push_back(command);
  process_command_queue_();
}

void Levoit::send_empty_command(LevoitPayloadType payloadType) {
  send_command(LevoitCommand{.payloadType = payloadType, .payload = std::vector<uint8_t>{}});
}

}  // namespace levoit
}  // namespace esphome
