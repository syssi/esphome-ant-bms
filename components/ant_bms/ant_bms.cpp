#include "ant_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ant_bms {

static const char *const TAG = "ant_bms";

void AntBms::on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) {
  ESP_LOGW(TAG, "Invalid size (%zu) for ANT BMS frame!", data.size());
}

void AntBms::update() { this->read_registers(); }

void AntBms::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBms::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBms:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);

  LOG_SENSOR("", "Capacity Remaining", this->capacity_remaining_sensor_);
}

}  // namespace ant_bms
}  // namespace esphome
