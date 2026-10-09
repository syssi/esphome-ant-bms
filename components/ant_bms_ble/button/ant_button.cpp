#include "ant_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// Fallback for ESPHome < 2026.10.0
#ifndef ESPHOME_LOG_TAG
#define ESPHOME_LOG_TAG(name, tag) static const char *const name = tag
#endif

namespace esphome::ant_bms_ble {

ESPHOME_LOG_TAG(TAG, "ant_bms_ble.button");

void AntButton::dump_config() { LOG_BUTTON("", "AntBmsBle Button", this); }
void AntButton::press_action() { this->parent_->write_register(this->holding_register_, 0x0000); }

}  // namespace esphome::ant_bms_ble
