#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include <array>

namespace esphome::ant_bms {

class AntBms : public uart::UARTDevice, public PollingComponent {
 public:
  void loop() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override;

  void set_online_status_binary_sensor(binary_sensor::BinarySensor *online_status_binary_sensor) {
    online_status_binary_sensor_ = online_status_binary_sensor;
  }

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
  void set_min_voltage_cell_sensor(sensor::Sensor *min_voltage_cell_sensor) {
    min_voltage_cell_sensor_ = min_voltage_cell_sensor;
  }
  void set_max_voltage_cell_sensor(sensor::Sensor *max_voltage_cell_sensor) {
    max_voltage_cell_sensor_ = max_voltage_cell_sensor;
  }
  void set_delta_cell_voltage_sensor(sensor::Sensor *delta_cell_voltage_sensor) {
    delta_cell_voltage_sensor_ = delta_cell_voltage_sensor;
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
  void set_state_of_health_sensor(sensor::Sensor *state_of_health_sensor) {
    state_of_health_sensor_ = state_of_health_sensor;
  }
  void set_battery_status_code_sensor(sensor::Sensor *battery_status_code_sensor) {
    battery_status_code_sensor_ = battery_status_code_sensor;
  }
  void set_total_discharging_capacity_sensor(sensor::Sensor *total_discharging_capacity_sensor) {
    total_discharging_capacity_sensor_ = total_discharging_capacity_sensor;
  }
  void set_total_charging_capacity_sensor(sensor::Sensor *total_charging_capacity_sensor) {
    total_charging_capacity_sensor_ = total_charging_capacity_sensor;
  }
  void set_total_discharging_time_sensor(sensor::Sensor *total_discharging_time_sensor) {
    total_discharging_time_sensor_ = total_discharging_time_sensor;
  }
  void set_total_charging_time_sensor(sensor::Sensor *total_charging_time_sensor) {
    total_charging_time_sensor_ = total_charging_time_sensor;
  }
  void set_balanced_cell_bitmask_sensor(sensor::Sensor *balanced_cell_bitmask_sensor) {
    balanced_cell_bitmask_sensor_ = balanced_cell_bitmask_sensor;
  }

  void set_device_model_text_sensor(text_sensor::TextSensor *device_model_text_sensor) {
    device_model_text_sensor_ = device_model_text_sensor;
  }
  void set_software_version_text_sensor(text_sensor::TextSensor *software_version_text_sensor) {
    software_version_text_sensor_ = software_version_text_sensor;
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
  void set_battery_status_text_sensor(text_sensor::TextSensor *battery_status_text_sensor) {
    battery_status_text_sensor_ = battery_status_text_sensor;
  }
  void set_total_discharging_time_formatted_text_sensor(
      text_sensor::TextSensor *total_discharging_time_formatted_text_sensor) {
    total_discharging_time_formatted_text_sensor_ = total_discharging_time_formatted_text_sensor;
  }
  void set_total_charging_time_formatted_text_sensor(
      text_sensor::TextSensor *total_charging_time_formatted_text_sensor) {
    total_charging_time_formatted_text_sensor_ = total_charging_time_formatted_text_sensor;
  }

  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }
  void set_balancer_switch(switch_::Switch *balancer_switch) { balancer_switch_ = balancer_switch; }
  void set_buzzer_switch(switch_::Switch *buzzer_switch) { buzzer_switch_ = buzzer_switch; }

  void set_rx_timeout(uint16_t rx_timeout) { rx_timeout_ = rx_timeout; }

  void on_ant_bms_data(const std::vector<uint8_t> &data);
  static std::array<uint8_t, 10> build_frame(uint8_t function, uint8_t address, uint16_t value);
  void write_register(uint8_t address, uint16_t value);

 protected:
  binary_sensor::BinarySensor *online_status_binary_sensor_{nullptr};

  sensor::Sensor *battery_strings_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *soc_sensor_{nullptr};
  sensor::Sensor *total_battery_capacity_setting_sensor_{nullptr};
  sensor::Sensor *capacity_remaining_sensor_{nullptr};
  sensor::Sensor *battery_cycle_capacity_sensor_{nullptr};
  sensor::Sensor *total_voltage_sensor_{nullptr};
  sensor::Sensor *total_runtime_sensor_{nullptr};
  sensor::Sensor *average_cell_voltage_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *min_cell_voltage_sensor_{nullptr};
  sensor::Sensor *max_cell_voltage_sensor_{nullptr};
  sensor::Sensor *min_voltage_cell_sensor_{nullptr};
  sensor::Sensor *max_voltage_cell_sensor_{nullptr};
  sensor::Sensor *delta_cell_voltage_sensor_{nullptr};
  sensor::Sensor *charge_mosfet_status_code_sensor_{nullptr};
  sensor::Sensor *discharge_mosfet_status_code_sensor_{nullptr};
  sensor::Sensor *balancer_status_code_sensor_{nullptr};
  sensor::Sensor *state_of_health_sensor_{nullptr};
  sensor::Sensor *battery_status_code_sensor_{nullptr};
  sensor::Sensor *total_discharging_capacity_sensor_{nullptr};
  sensor::Sensor *total_charging_capacity_sensor_{nullptr};
  sensor::Sensor *total_discharging_time_sensor_{nullptr};
  sensor::Sensor *total_charging_time_sensor_{nullptr};
  sensor::Sensor *balanced_cell_bitmask_sensor_{nullptr};

  switch_::Switch *charging_switch_{nullptr};
  switch_::Switch *discharging_switch_{nullptr};
  switch_::Switch *balancer_switch_{nullptr};
  switch_::Switch *buzzer_switch_{nullptr};

  text_sensor::TextSensor *charge_mosfet_status_text_sensor_{nullptr};
  text_sensor::TextSensor *discharge_mosfet_status_text_sensor_{nullptr};
  text_sensor::TextSensor *balancer_status_text_sensor_{nullptr};
  text_sensor::TextSensor *total_runtime_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *battery_status_text_sensor_{nullptr};
  text_sensor::TextSensor *total_discharging_time_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *total_charging_time_formatted_text_sensor_{nullptr};
  text_sensor::TextSensor *device_model_text_sensor_{nullptr};
  text_sensor::TextSensor *software_version_text_sensor_{nullptr};

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[32];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  std::vector<uint8_t> rx_buffer_;
  uint8_t no_response_count_{0};
  uint32_t last_byte_{0};
  uint16_t rx_timeout_{50};

  void on_ant_bms_data_(const uint8_t &function, const std::vector<uint8_t> &data);
  void on_status_data_(const std::vector<uint8_t> &data);
  void on_device_info_data_(const std::vector<uint8_t> &data);
  bool parse_ant_bms_byte_(uint8_t byte);
  void authenticate_();
  void authenticate_variable_(const uint8_t *data, uint8_t data_len);
  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(switch_::Switch *obj, const bool &state);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void read_registers_();
  void send_(uint8_t function, uint8_t address, uint16_t value);
  void publish_device_unavailable_();
  void reset_online_status_tracker_();
  void track_online_status_();

  static uint16_t crc16(const uint8_t *data, uint8_t len) {
    uint16_t crc = 0xFFFF;
    while (len--) {
      crc ^= *data++;
      for (uint8_t i = 0; i < 8; i++) {
        if ((crc & 0x01) != 0) {
          crc >>= 1;
          crc ^= 0xA001;
        } else {
          crc >>= 1;
        }
      }
    }
    return crc;
  }

  std::string format_total_runtime_(const uint32_t value) {
    int seconds = (int) value;
    int years = seconds / (24 * 3600 * 365);
    seconds = seconds % (24 * 3600 * 365);
    int days = seconds / (24 * 3600);
    seconds = seconds % (24 * 3600);
    int hours = seconds / 3600;

    char buf[16];
    int len = 0;
    if (years)
      len += snprintf(buf + len, sizeof(buf) - len, "%dy ", years);
    if (days)
      len += snprintf(buf + len, sizeof(buf) - len, "%dd ", days);
    if (hours)
      len += snprintf(buf + len, sizeof(buf) - len, "%dh", hours);

    return std::string(buf, len);
  }
};

}  // namespace esphome::ant_bms
