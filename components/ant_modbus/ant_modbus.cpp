#include "ant_modbus.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ant_modbus {

static const char *const TAG = "ant_modbus";

void AntModbus::setup() {
  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
  }
}
void AntModbus::loop() {
  const uint32_t now = millis();
  if (now - this->last_ant_modbus_byte_ > 50) {
    this->rx_buffer_.clear();
    this->last_ant_modbus_byte_ = now;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_ant_modbus_byte_(byte)) {
      this->last_ant_modbus_byte_ = now;
    } else {
      this->rx_buffer_.clear();
    }
  }
}

uint16_t chksum(const uint8_t data[], const uint16_t len) {
  uint16_t checksum = 0;
  for (uint16_t i = 4; i < len; i++) {
    checksum = checksum + data[i];
  }
  return checksum;
}

bool AntModbus::parse_ant_modbus_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];
  uint16_t frame_len = 140;

  // 0xAA 0x55 0xAA 0xFF 0x02 0x1D 0x10 0x2A ...
  //   0    1    2    3  ^^^^^ data ^^^^^^^^

  // Byte 0: Ant BMS start byte (match all)
  if (at == 0)
    return true;
  uint8_t address = raw[0];

  // Byte 0...3: Header
  if (at < 4)
    return true;

  if (raw[0] != 0xAA || raw[1] != 0x55 || raw[2] != 0xAA || raw[3] != 0xFF) {
    ESP_LOGW(TAG, "Invalid header.");

    // return false to reset buffer
    return false;
  }

  // Byte 0...139
  if (at < frame_len - 1)
    return true;

  uint8_t function = raw[3];

  uint16_t computed_crc = chksum(raw, frame_len - 2);
  uint16_t remote_crc = uint16_t(raw[frame_len - 2]) << 8 | (uint16_t(raw[frame_len - 1]) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "AntModbus CRC Check failed! %04X != %04X", computed_crc, remote_crc);
    return false;
  }

  ESP_LOGVV(TAG, "RX <- %s", format_hex_pretty(raw, at + 1).c_str());
  ESP_LOGVV(TAG, "CRC: 0x%02X 0x%02X", raw[138], raw[139]);

  std::vector<uint8_t> data(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);

  bool found = false;
  for (auto *device : this->devices_) {
    if (device->address_ == address) {
      device->on_ant_modbus_data(function, data);
      found = true;
    }
  }
  if (!found) {
    ESP_LOGW(TAG, "Got AntModbus frame from unknown address 0x%02X!", address);
  }

  // return false to reset buffer
  return false;
}

void AntModbus::dump_config() {
  ESP_LOGCONFIG(TAG, "AntModbus:");
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
}
float AntModbus::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}
void AntModbus::send(uint8_t function, uint8_t address, uint16_t value) {
  uint8_t frame[6];
  frame[0] = 0xA5;
  frame[1] = function;    // 0xA5
  frame[2] = address;     // 0xFA (Charge MOSFET)
  frame[3] = value >> 8;  // 0x00
  frame[4] = value >> 0;  // 0x01 (On)
  frame[5] = (uint8_t)(frame[2] + frame[3]) + frame[4];

  this->write_array(frame, 6);
  this->flush();
}

void AntModbus::read_registers() {
  uint8_t frame[6];
  frame[0] = 0x5A;
  frame[1] = 0x5A;
  frame[2] = 0x00;
  frame[3] = 0x00;
  frame[4] = 0x01;
  frame[5] = 0x01;

  this->write_array(frame, 6);
  this->flush();
}

}  // namespace ant_modbus
}  // namespace esphome
