#pragma once

#include "../ant_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace ant_bms {

class AntBms;
class AntSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(AntBms *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };
  void set_new_protocol_turn_on_register(uint8_t new_protocol_turn_on_register) {
    this->new_protocol_turn_on_register_ = new_protocol_turn_on_register;
  };
  void set_new_protocol_turn_off_register(uint8_t new_protocol_turn_off_register) {
    this->new_protocol_turn_off_register_ = new_protocol_turn_off_register;
  };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  void write_state(bool state) override;
  AntBms *parent_;
  uint8_t holding_register_;
  uint8_t new_protocol_turn_on_register_;
  uint8_t new_protocol_turn_off_register_;
};

}  // namespace ant_bms
}  // namespace esphome
