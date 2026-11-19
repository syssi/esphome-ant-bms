#pragma once

#include "../ant_bms_old.h"
#include "esphome/components/switch/switch.h"

namespace esphome::ant_bms_old {

class AntBmsOld;
class AntSwitch : public switch_::Switch {
 public:
  void set_parent(AntBmsOld *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };

 protected:
  AntBmsOld *parent_;
  uint8_t holding_register_;

  void write_state(bool state) override;
};

}  // namespace esphome::ant_bms_old
