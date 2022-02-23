#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace ant_modbus {

class AntModbusDevice;

class AntModbus : public uart::UARTDevice, public Component {
 public:
  AntModbus() = default;

  void setup() override;

  void loop() override;

  void dump_config() override;

  void register_device(AntModbusDevice *device) { this->devices_.push_back(device); }

  float get_setup_priority() const override;

  void send(uint8_t function, uint8_t address, uint16_t value);
  void read_registers();

  void set_flow_control_pin(GPIOPin *flow_control_pin) { this->flow_control_pin_ = flow_control_pin; }

 protected:
  GPIOPin *flow_control_pin_{nullptr};

  bool parse_ant_modbus_byte_(uint8_t byte);

  std::vector<uint8_t> rx_buffer_;
  uint32_t last_ant_modbus_byte_{0};
  std::vector<AntModbusDevice *> devices_;
};

class AntModbusDevice {
 public:
  void set_parent(AntModbus *parent) { parent_ = parent; }
  void set_address(uint8_t address) { address_ = address; }
  virtual void on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) = 0;

  void send(uint8_t function, uint8_t address, uint16_t value) { this->parent_->send(function, address, value); }
  void read_registers() { this->parent_->read_registers(); }

 protected:
  friend AntModbus;

  AntModbus *parent_;
  uint8_t address_;
};

}  // namespace ant_modbus
}  // namespace esphome
