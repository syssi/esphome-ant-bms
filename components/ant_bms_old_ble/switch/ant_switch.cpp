#include "ant_switch.h"

namespace esphome::ant_bms_old_ble {

void AntSwitch::write_state(bool state) { this->parent_->write_register(this->holding_register_, (uint16_t) state); }

}  // namespace esphome::ant_bms_old_ble
