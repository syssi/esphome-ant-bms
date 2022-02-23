#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/ant_modbus/ant_modbus.h"

namespace esphome {
namespace ant_bms {

class AntBms : public PollingComponent, public ant_modbus::AntModbusDevice {
 public:
  void set_battery_strings_sensor(sensor::Sensor *battery_strings_sensor) {
    battery_strings_sensor_ = battery_strings_sensor;
  }
  void set_soc_sensor(sensor::Sensor *soc_sensor) { soc_sensor_ = soc_sensor; }
  void set_total_battery_capacity_setting_sensor(sensor::Sensor *total_battery_capacity_setting_sensor) {
    total_battery_capacity_setting_sensor_ = total_battery_capacity_setting_sensor;
  }
  void set_capacity_remaining_sensor(sensor::Sensor *capacity_remaining_sensor) {
    capacity_remaining_sensor_ = capacity_remaining_sensor;
  }
  void set_battery_cycle_capacity_sensor(sensor::Sensor *battery_cycle_capacity_sensor) {
    battery_cycle_capacity_sensor_ = battery_cycle_capacity_sensor;
  }
  void set_total_voltage_sensor(sensor::Sensor *total_voltage_sensor) { total_voltage_sensor_ = total_voltage_sensor; }
  void set_average_cell_voltage_sensor(sensor::Sensor *average_cell_voltage_sensor) {
    average_cell_voltage_sensor_ = average_cell_voltage_sensor;
  }
  void set_min_cell_voltage_sensor(sensor::Sensor *min_cell_voltage_sensor) {
    min_cell_voltage_sensor_ = min_cell_voltage_sensor;
  }
  void set_max_cell_voltage_sensor(sensor::Sensor *max_cell_voltage_sensor) {
    max_cell_voltage_sensor_ = max_cell_voltage_sensor;
  }

  void set_cell_voltage_sensor(uint8_t cell, sensor::Sensor *cell_voltage_sensor) {
    this->cells_[cell].cell_voltage_sensor_ = cell_voltage_sensor;
  }
  void set_temperature_sensor(uint8_t temperature, sensor::Sensor *temperature_sensor) {
    this->temperatures_[temperature].temperature_sensor_ = temperature_sensor;
  }

  void set_enable_fake_traffic(bool enable_fake_traffic) { enable_fake_traffic_ = enable_fake_traffic; }

  void dump_config() override;

  void on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) override;

  void update() override;

 protected:
  sensor::Sensor *battery_strings_sensor_;
  sensor::Sensor *soc_sensor_;
  sensor::Sensor *total_battery_capacity_setting_sensor_;
  sensor::Sensor *capacity_remaining_sensor_;
  sensor::Sensor *battery_cycle_capacity_sensor_;
  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *average_cell_voltage_sensor_;
  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[32];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  bool enable_fake_traffic_;

  void on_status_data_(const std::vector<uint8_t> &data);
  void publish_state_(sensor::Sensor *sensor, float value);
};

}  // namespace ant_bms
}  // namespace esphome
