#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ant_bms_ble {

static const char *const TAG = "ant_bms_ble.switch";

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBmsBle Switch", this); }
void AntSwitch::write_state(bool state) {
  this->parent_->write_register((state) ? this->turn_on_register_ : this->turn_off_register_, 0x00);
}

}  // namespace ant_bms_ble
}  // namespace esphome
