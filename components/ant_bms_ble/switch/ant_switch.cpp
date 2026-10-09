#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::ant_bms_ble {

ESPHOME_LOG_TAG(TAG, "ant_bms_ble.switch");

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBmsBle Switch", this); }
void AntSwitch::write_state(bool state) {
  this->parent_->write_register((state) ? this->turn_on_register_ : this->turn_off_register_, 0x0000);
}

}  // namespace esphome::ant_bms_ble
