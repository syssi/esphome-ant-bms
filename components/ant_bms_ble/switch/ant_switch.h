#pragma once

#include "../ant_bms_ble.h"
#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"

namespace esphome {
namespace ant_bms_ble {

class AntBmsBle;
class AntSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(AntBmsBle *parent) { this->parent_ = parent; };
  void set_turn_on_register(uint8_t turn_on_register) { this->turn_on_register_ = turn_on_register; };
  void set_turn_off_register(uint8_t turn_off_register) { this->turn_off_register_ = turn_off_register; };
  void dump_config() override;
  void loop() override {}
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  AntBmsBle *parent_;
  uint8_t turn_on_register_;
  uint8_t turn_off_register_;

  void write_state(bool state) override;
};

}  // namespace ant_bms_ble
}  // namespace esphome
