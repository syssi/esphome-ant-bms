#pragma once

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/esp32_ble_tracker/esp32_ble_tracker.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"

#ifdef USE_ESP32

#include <esp_gattc_api.h>

namespace esphome {
namespace ant_bms_ble {

namespace espbt = esphome::esp32_ble_tracker;

class AntBmsBle : public esphome::ble_client::BLEClientNode, public PollingComponent {
 public:
  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                           esp_ble_gattc_cb_param_t *param) override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

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

  void set_charging_switch(switch_::Switch *charging_switch) { charging_switch_ = charging_switch; }
  void set_discharging_switch(switch_::Switch *discharging_switch) { discharging_switch_ = discharging_switch; }
  void set_balancer_switch(switch_::Switch *balancer_switch) { balancer_switch_ = balancer_switch; }
  void set_bluetooth_switch(switch_::Switch *bluetooth_switch) { bluetooth_switch_ = bluetooth_switch; }
  void set_buzzer_switch(switch_::Switch *buzzer_switch) { buzzer_switch_ = buzzer_switch; }

  void set_enable_fake_traffic(bool enable_fake_traffic) { enable_fake_traffic_ = enable_fake_traffic; }
  void set_password(const std::string &password) { this->password_ = password; }

  void write_register(uint16_t address, uint8_t value);

 protected:
  binary_sensor::BinarySensor *online_status_binary_sensor_;

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
  sensor::Sensor *min_voltage_cell_sensor_;
  sensor::Sensor *max_voltage_cell_sensor_;
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
  text_sensor::TextSensor *device_model_text_sensor_;
  text_sensor::TextSensor *software_version_text_sensor_;

  struct Cell {
    sensor::Sensor *cell_voltage_sensor_{nullptr};
  } cells_[32];

  struct Temperature {
    sensor::Sensor *temperature_sensor_{nullptr};
  } temperatures_[6];

  bool enable_fake_traffic_;
  std::string password_;

  std::vector<uint8_t> frame_buffer_;
  uint8_t no_response_count_{0};
  uint16_t characteristic_handle_;

  void assemble_(const uint8_t *data, uint16_t length);
  void on_ant_bms_ble_data_(const uint8_t &function, const std::vector<uint8_t> &data);
  void on_status_data_(const std::vector<uint8_t> &data);
  void on_device_info_data_(const std::vector<uint8_t> &data);
  bool send_(uint8_t function, uint16_t address, uint8_t value, bool authenticate);
  bool authenticate_();
  bool authenticate_variable_(const uint8_t *data, uint8_t data_len);
  void publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state);
  void publish_state_(sensor::Sensor *sensor, float value);
  void publish_state_(switch_::Switch *obj, const bool &state);
  void publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state);
  void publish_device_unavailable_();
  void reset_online_status_tracker_();
  void track_online_status_();

  uint16_t crc16_(const uint8_t *data, uint8_t len) {
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
    return (years ? to_string(years) + "y " : "") + (days ? to_string(days) + "d " : "") +
           (hours ? to_string(hours) + "h" : "");
  }
};

}  // namespace ant_bms_ble
}  // namespace esphome

#endif
