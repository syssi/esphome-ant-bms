#include "ant_switch.h"

namespace esphome::ant_bms {

void AntSwitch::write_state(bool state) {
  this->parent_->write_register((state) ? this->turn_on_register_ : this->turn_off_register_, 0x0000);
}

}  // namespace esphome::ant_bms
