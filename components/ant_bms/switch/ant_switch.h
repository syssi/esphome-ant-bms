#pragma once

#include "../ant_bms.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome::ant_bms {

class AntBms;
class AntSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(AntBms *parent) { this->parent_ = parent; };
  void set_turn_on_register(uint8_t turn_on_register) { this->turn_on_register_ = turn_on_register; };
  void set_turn_off_register(uint8_t turn_off_register) { this->turn_off_register_ = turn_off_register; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  AntBms *parent_;
  uint8_t turn_on_register_;
  uint8_t turn_off_register_;

  void write_state(bool state) override;
};

}  // namespace esphome::ant_bms
