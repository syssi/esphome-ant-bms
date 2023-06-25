#include "ant_button.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ant_bms_old {

static const char *const TAG = "ant_bms_old.button";

void AntButton::dump_config() { LOG_BUTTON("", "AntBmsOld Button", this); }
void AntButton::press_action() { this->parent_->write_register(this->holding_register_, 0x0000); }

}  // namespace ant_bms_old
}  // namespace esphome
