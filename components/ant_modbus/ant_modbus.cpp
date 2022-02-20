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
  for (uint16_t i = 0; i < len; i++) {
    checksum = checksum + data[i];
  }
  return checksum;
}

bool AntModbus::parse_ant_modbus_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];

  // Byte 0: Start sequence (0x4E)
  if (at == 0)
    return true;
  uint8_t address = raw[0];

  // Byte 1: Start sequence (0x57)
  if (at == 1)
    return true;

  // Byte 2: Size (low byte)
  if (at == 2)
    return true;

  // Byte 3: Size (high byte)
  if (at == 3)
    return true;
  uint16_t data_len = (uint16_t(raw[2]) << 8 | (uint16_t(raw[2 + 1]) << 0));

  // data_len: CRC_LO (over all bytes)
  if (at <= data_len)
    return true;

  uint8_t function = raw[8];

  // data_len+1: CRC_HI (over all bytes)
  uint16_t computed_crc = chksum(raw, data_len);
  uint16_t remote_crc = uint16_t(raw[data_len]) << 8 | (uint16_t(raw[data_len + 1]) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "AntModbus CRC Check failed! %02X!=%02X", computed_crc, remote_crc);
    return false;
  }

  std::vector<uint8_t> data(this->rx_buffer_.begin() + 11, this->rx_buffer_.begin() + data_len - 3);

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
void AntModbus::send(uint8_t address, uint8_t function, uint16_t start_address, uint16_t register_count) {
  uint8_t frame[8];
  frame[0] = address;
  frame[1] = function;
  frame[2] = start_address >> 8;
  frame[3] = start_address >> 0;
  frame[4] = register_count >> 8;
  frame[5] = register_count >> 0;
  auto crc = chksum(frame, 6);
  frame[6] = crc >> 8;
  frame[7] = crc >> 0;

  this->write_array(frame, 8);
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
