#include "ant_modbus.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ant_modbus {

static const char *const TAG = "ant_modbus";

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

uint16_t crc16(const uint8_t *data, uint8_t len) {
  uint16_t crc = 0xFFFF;
  while (len--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; i++) {
      if ((crc & 0x01) != 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
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

  if (!(raw[0] == 0xAA && raw[1] == 0x55 && raw[2] == 0xAA && raw[3] == 0xFF) && !(raw[0] == 0x7E && raw[1] == 0xA1)) {
    ESP_LOGW(TAG, "Invalid header.");

    // return false to reset buffer
    return false;
  }

  // Byte 0...5
  if (at < 6)
    return true;

  // Calculate new protocol frame length
  if (address == 0x7E) {
    frame_len = 6 + raw[5] + 4;
  }

  // Byte 0...139
  if (at < frame_len - 1)
    return true;

  // New protocol response frame
  //
  // 0x7E 0xA1 0x43 0x6A 0x01 0x02 0x05 0x00 0x1E 0x5C 0xAA 0x55    Password ACK
  // 0x7E 0xA1 0x61 0x04 0x00 0x02 0x01 0x00 0xF2 0x2B 0xAA 0x55    Register 0x04 ACK
  // 0x7E 0xA1 0x61 0x06 0x00 0x02 0x01 0x00 0x8B 0xEB 0xAA 0x55    Register 0x06 ACK
  //  0    1    2    3    4    5    6    7    8    9    10   11
  // head add  func val  val  len  data data crc  crc  end  end
  if (address == 0x7E) {
    // Discard new protocol frame for now
    ESP_LOGD(TAG, "New protocol response frame discarded: %s", format_hex_pretty(raw, at + 1).c_str());
    return false;
  }

  uint8_t function = raw[3];

  uint16_t computed_crc = chksum(raw, frame_len - 2);
  uint16_t remote_crc = uint16_t(raw[frame_len - 2]) << 8 | (uint16_t(raw[frame_len - 1]) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "AntModbus CRC Check failed! %04X != %04X", computed_crc, remote_crc);
    return false;
  }

  ESP_LOGVV(TAG, "RX <- %s", format_hex_pretty(raw, at + 1).c_str());

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

void AntModbus::dump_config() { ESP_LOGCONFIG(TAG, "AntModbus:"); }
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

void AntModbus::authenticate_v2021_() {
  uint8_t frame[22];

  frame[0] = 0x7e;  // header
  frame[1] = 0xa1;  // header
  frame[2] = 0x23;  // control
  frame[3] = 0x6a;  // address
  frame[4] = 0x01;
  frame[5] = 0x0c;   // data len
  frame[6] = 0x31;   // 1
  frame[7] = 0x32;   // 2
  frame[8] = 0x33;   // 3
  frame[9] = 0x34;   // 4
  frame[10] = 0x35;  // 5
  frame[11] = 0x36;  // 6
  frame[12] = 0x37;  // 7
  frame[13] = 0x38;  // 8
  frame[14] = 0x39;  // 9
  frame[15] = 0x61;  // a
  frame[16] = 0x62;  // b
  frame[17] = 0x63;  // c

  auto crc = crc16(frame + 1, 17);
  frame[18] = crc >> 0;  // CRC
  frame[19] = crc >> 8;  // CRC

  frame[20] = 0xaa;
  frame[21] = 0x55;

  this->write_array(frame, 22);
  this->flush();
}

void AntModbus::authenticate_v2021_variable_(uint8_t data_len, const uint8_t *data) {
  std::vector<uint8_t> data = {0x7E, 0xA1, 0x23, 0x6A, 0x01};
  data.push_back(data_len);
  for (int i = 0; i < data_len; i++) {
    data.push_back(data[i]);
  }
  auto crc = crc16(data.data() + 1, data.size());
  data.push_back(crc >> 0);
  data.push_back(crc >> 8);
  data.push_back(0xAA);
  data.push_back(0x55);

  this->write_array(data);
  this->flush();
}

void AntModbus::send_v2021(uint8_t function, uint8_t address, uint16_t value) {
  this->authenticate_v2021_();

  uint8_t frame[10];
  frame[0] = 0x7e;        // header
  frame[1] = 0xa1;        // header
  frame[2] = function;    // control
  frame[3] = address;     // address
  frame[4] = value >> 8;  // value
  frame[5] = value >> 0;  // value
  auto crc = crc16(frame + 1, 5);
  frame[6] = crc >> 0;  // CRC
  frame[7] = crc >> 8;  // CRC
  frame[8] = 0xaa;      // footer
  frame[9] = 0x55;      // footer

  this->write_array(frame, 10);
  this->flush();
}

}  // namespace ant_modbus
}  // namespace esphome
