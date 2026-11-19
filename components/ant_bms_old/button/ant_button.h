#pragma once

#include "../ant_bms_old.h"
#include "esphome/components/button/button.h"

namespace esphome::ant_bms_old {

class AntBmsOld;
class AntButton : public button::Button {
 public:
  void set_parent(AntBmsOld *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };

 protected:
  void press_action() override;
  AntBmsOld *parent_;
  uint8_t holding_register_;
};

}  // namespace esphome::ant_bms_old
