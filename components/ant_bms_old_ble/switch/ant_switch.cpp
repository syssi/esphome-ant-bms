#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::ant_bms_old_ble {

ESPHOME_LOG_TAG(TAG, "ant_bms_old_ble.switch");

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBmsOldBle Switch", this); }
void AntSwitch::write_state(bool state) { this->parent_->write_register(this->holding_register_, (uint16_t) state); }

}  // namespace esphome::ant_bms_old_ble
