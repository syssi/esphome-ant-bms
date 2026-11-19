#pragma once

#include "../ant_bms_old_ble.h"
#include "esphome/components/switch/switch.h"

namespace esphome::ant_bms_old_ble {

class AntBmsOldBle;
class AntSwitch : public switch_::Switch {
 public:
  void set_parent(AntBmsOldBle *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };

 protected:
  AntBmsOldBle *parent_;
  uint8_t holding_register_;

  void write_state(bool state) override;
};

}  // namespace esphome::ant_bms_old_ble
