#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/ant_modbus/ant_modbus.h"

namespace esphome {
namespace ant_bms {

class AntBms : public PollingComponent, public ant_modbus::AntModbusDevice {
 public:
  void set_battery_strings_sensor(sensor::Sensor *battery_strings_sensor) {
    battery_strings_sensor_ = battery_strings_sensor;
  }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
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
  void set_total_runtime_sensor(sensor::Sensor *total_runtime_sensor) { total_runtime_sensor_ = total_runtime_sensor; }
  void set_average_cell_voltage_sensor(sensor::Sensor *average_cell_voltage_sensor) {
    average_cell_voltage_sensor_ = average_cell_voltage_sensor;
  }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }
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
  void set_charge_mosfet_status_code_sensor(sensor::Sensor *charge_mosfet_status_code_sensor) {
    charge_mosfet_status_code_sensor_ = charge_mosfet_status_code_sensor;
  }
  void set_discharge_mosfet_status_code_sensor(sensor::Sensor *discharge_mosfet_status_code_sensor) {
    discharge_mosfet_status_code_sensor_ = discharge_mosfet_status_code_sensor;
  }
  void set_balancer_status_code_sensor(sensor::Sensor *balancer_status_code_sensor) {
    balancer_status_code_sensor_ = balancer_status_code_sensor;
  }

  void set_charge_mosfet_status_text_sensor(text_sensor::TextSensor *charge_mosfet_status_text_sensor) {
    charge_mosfet_status_text_sensor_ = charge_mosfet_status_text_sensor;
  }
  void set_discharge_mosfet_status_text_sensor(text_sensor::TextSensor *discharge_mosfet_status_text_sensor) {
    discharge_mosfet_status_text_sensor_ = discharge_mosfet_status_text_sensor;
  }
  void set_balancer_status_text_sensor(text_sensor::TextSensor *balancer_status_text_sensor) {
    balancer_status_text_sensor_ = balancer_status_text_sensor;
  }
  void set_total_runtime_formatted_text_sensor(text_sensor::TextSensor *total_runtime_formatted_text_sensor) {
    total_runtime_formatted_text_sensor_ = total_runtime_formatted_text_sensor;
  }

  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }
  void set_balancer_switch(switch_::Switch *balancer_switch) { balancer_switch_ = balancer_switch; }
  void set_bluetooth_switch(switch_::Switch *bluetooth_switch) { bluetooth_switch_ = bluetooth_switch; }
  void set_buzzer_switch(switch_::Switch *buzzer_switch) { buzzer_switch_ = buzzer_switch; }

  void set_enable_fake_traffic(bool enable_fake_traffic) { enable_fake_traffic_ = enable_fake_traffic; }
  void set_password(const std::string &password) { this->password_ = password; }
  void set_supports_new_commands(bool supports_new_commands) { supports_new_commands_ = supports_new_commands; }

  void dump_config() override;

  void on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) override;

  void update() override;

  void write_register(uint8_t address, uint16_t value);
  bool supports_new_commands() { return supports_new_commands_; }

 protected:
  sensor::Sensor *battery_strings_sensor_;
  sensor::Sensor *current_sensor_;
  sensor::Sensor *soc_sensor_;
  sensor::Sensor *total_battery_capacity_setting_sensor_;
  sensor::Sensor *capacity_remaining_sensor_;
  sensor::Sensor *battery_cycle_capacity_sensor_;
  sensor::Sensor *total_voltage_sensor_;
  sensor::Sensor *total_runtime_sensor_;
  sensor::Sensor *average_cell_voltage_sensor_;
  sensor::Sensor *power_sensor_;
  sensor::Sensor *min_cell_voltage_sensor_;
  sensor::Sensor *max_cell_voltage_sensor_;
  sensor::Sensor *charge_mosfet_status_code_sensor_;
  sensor::Sensor *discharge_mosfet_status_code_sensor_;
  sensor::Sensor *balancer_status_code_sensor_;

  switch_::Switch *charging_switch_;
  switch_::Switch *discharging_switch_;
  switch_::Switch *balancer_switch_;
  switch_::Switch *bluetooth_switch_;
  switch_::Switch *buzzer_switch_;

  text_sensor::TextSensor *charge_mosfet_status_text_sensor_;
  text_sensor::TextSensor *discharge_mosfet_status_text_sensor_;
  text_sensor::TextSensor *balancer_status_text_sensor_;
  text_sensor::TextSensor *total_runtime_formatted_text_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[32];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  bool enable_fake_traffic_;
  bool supports_new_commands_;
  std::string password_;

  void on_status_data_(const std::vector<uint8_t> &data);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(switch_::Switch *obj, const bool &state);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void authenticate_();

  float get_signed_float_(const uint32_t value) {
    if ((value & 0x80000000) == 0x80000000) {
      return (float) (value & 0x7FFFFFFF) * -1;
    }

    return (float) (value & 0x7FFFFFFF);
  }

  const std::string format_total_runtime_(const uint32_t value) {
    int seconds = (int) value;
    int years = seconds / (24 * 3600 * 365);
    seconds = seconds % (24 * 3600 * 365);
    int days = seconds / (24 * 3600);
    seconds = seconds % (24 * 3600);
    int hours = seconds / 3600;
    return (years ? to_string(years) + "y " : "") + (days ? to_string(days) + "d " : "") +
            (hours ? to_string(hours) + "h" : "");
  }
};

}  // namespace ant_bms
}  // namespace esphome
