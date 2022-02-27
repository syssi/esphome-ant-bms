#include "ant_switch.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ant_bms {

static const char *const TAG = "ant_bms.switch";

void AntSwitch::dump_config() { LOG_SWITCH("", "AntBms Switch", this); }
void AntSwitch::write_state(bool state) {
  if (this->parent_->supports_new_commands()) {
    this->parent_->write_register(
        (state) ? this->new_protocol_turn_on_register_ : this->new_protocol_turn_off_register_, 0x0000);
    return;
  }

  if (this->holding_register_ == 0x00) {
    ESP_LOGW(TAG, "This switch isn't supported by the selected protocol version.");
    return;
  }

  this->parent_->write_register(this->holding_register_, (uint16_t) state);
}

}  // namespace ant_bms
}  // namespace esphome
