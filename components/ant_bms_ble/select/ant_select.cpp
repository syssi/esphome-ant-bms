#include "ant_select.h"
#include "esphome/core/log.h"

namespace esphome::ant_bms_ble {

static const char *const TAG = "ant_bms_ble.select";

void AntSelect::dump_config() {
  LOG_SELECT("", "AntBmsBle Select", this);
  ESP_LOGCONFIG(TAG, "  Options are:");
  const auto &options = this->traits.get_options();
  for (size_t i = 0; i < this->mappings_.size(); i++) {
    ESP_LOGCONFIG(TAG, "    0x%04X: %s", this->mappings_.at(i), options.at(i));
  }
}

void AntSelect::control(const std::string &value) {
  auto idx = this->index_of(value);
  if (idx.has_value()) {
    uint16_t address = this->mappings_.at(idx.value());
    ESP_LOGV(TAG, "Reading settings register 0x%04X", address);
    this->parent_->read_settings(address);
    return;
  }
  ESP_LOGW(TAG, "Invalid value %s", value.c_str());
}

}  // namespace esphome::ant_bms_ble
