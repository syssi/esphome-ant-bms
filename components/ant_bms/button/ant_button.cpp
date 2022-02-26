#include "ant_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ant_bms {

static const char *const TAG = "ant_bms.button";

void AntButton::dump_config() { LOG_BUTTON("", "AntBms Button", this); }
void AntButton::press_action() { this->parent_->write_register(this->holding_register_, 0x0000); }

}  // namespace ant_bms
}  // namespace esphome
