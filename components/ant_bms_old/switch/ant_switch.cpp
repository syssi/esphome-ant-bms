#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome::ant_bms_old {

static const char *const TAG = "ant_bms_old.switch";

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBmsOld Switch", this); }
void AntSwitch::write_state(bool state) { this->parent_->write_register(this->holding_register_, (uint16_t) state); }

}  // namespace esphome::ant_bms_old
