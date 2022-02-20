#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/ant_modbus/ant_modbus.h"

namespace esphome {
namespace ant_bms {

class AntBms : public PollingComponent, public ant_modbus::AntModbusDevice {
 public:
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
  }

  void dump_config() override;

  void on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) override;

  void update() override;

 protected:
  sensor::Sensor *capacity_remaining_sensor_;

  void on_status_data_(const std::vector<uint8_t> &data);
  void publish_state_(sensor::Sensor *sensor, float value);
};

}  // namespace ant_bms
}  // namespace esphome
