#pragma once

#include "../ant_bms_old_ble.h"
#include "esphome/components/button/button.h"

namespace esphome::ant_bms_old_ble {

class AntBmsOldBle;
class AntButton : public button::Button {
 public:
  void set_parent(AntBmsOldBle *parent) { this->parent_ = parent; };
  void set_holding_register(uint8_t holding_register) { this->holding_register_ = holding_register; };

 protected:
  void press_action() override;
  AntBmsOldBle *parent_;
  uint8_t holding_register_;
};

}  // namespace esphome::ant_bms_old_ble
