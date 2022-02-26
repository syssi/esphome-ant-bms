#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ant_bms {

static const char *const TAG = "ant_bms.switch";

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBms Switch", this); }
void AntSwitch::write_state(bool state) { this->parent_->write_register(this->holding_register_, (uint16_t) !state); }

}  // namespace ant_bms
}  // namespace esphome
