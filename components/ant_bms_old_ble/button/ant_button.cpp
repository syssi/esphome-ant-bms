#include "ant_button.h"

namespace esphome::ant_bms_old_ble {

void AntButton::press_action() { this->parent_->write_register(this->holding_register_, 0x00); }

}  // namespace esphome::ant_bms_old_ble
