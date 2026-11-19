#include "ant_button.h"

namespace esphome::ant_bms {

void AntButton::press_action() { this->parent_->write_register(this->holding_register_, 0x0000); }

}  // namespace esphome::ant_bms
