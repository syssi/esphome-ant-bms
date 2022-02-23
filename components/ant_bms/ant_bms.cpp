#include "ant_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ant_bms {

static const char *const TAG = "ant_bms";

static const uint8_t FUNCTION_READ_ALL = 0xFF;

void AntBms::on_ant_modbus_data(const uint8_t &function, const std::vector<uint8_t> &data) {
  if (data.size() == 140) {
    this->on_status_data_(data);
    return;
  }

  ESP_LOGW(TAG, "Invalid size (%zu) for ANT BMS frame!", data.size());
}

void AntBms::on_status_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 0)) << 16) | (uint32_t(ant_get_16bit(i + 2)) << 0);
  };

  ESP_LOGI(TAG, "Status frame received");

  // Status request
  // -> 0x5A 0x5A 0x00 0x00 0x01 0x01
  //
  // Status response
  // <- 0xAA 0x55 0xAA 0xFF: Header
  //      0    1    2    3
  // *Data*
  //
  // Byte   Address Content: Description      Decoded content                         Coeff./Unit

  //   4    0x02 0x1D: Total voltage         541 * 0.1 = 54.1V                        0.1 V
  this->publish_state_(this->total_voltage_sensor_, (float) ant_get_16bit(4) * 0.1f);
  //   6    0x10 0x2A: Cell voltage 1        4138 * 0.001 = 4.138V                    0.001 V
  //   8    0x10 0x2A: Cell voltage 2                4138 * 0.001 = 4.138V            0.001 V
  //  10    0x10 0x27: Cell voltage 3                4135 * 0.001 = 4.135V            0.001 V
  //  12    0x10 0x2A: Cell voltage 4                                                 0.001 V
  //  ...
  //  ...
  //  66    0x00 0x00: Cell voltage 31                                                0.001 V
  //  68    0x00 0x00: Cell voltage 32                                                0.001 V
  uint8_t cells = data[123];
  for (uint8_t i = 0; i < cells; i++) {
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, (float) ant_get_16bit(i * 2 + 6) * 0.001f);
  }
  //  70    0x00 0x00 0x00 0x00: Current               0.0 A                          0.1 A
  this->publish_state_(this->current_sensor_, (float) get_signed_float(ant_get_32bit(75)) * 0.1f);
  //  74    0x64: SOC                                  100 %                          1.0 %
  this->publish_state_(this->soc_sensor_, (float) data[74]);
  //  75    0x02 0x53 0x17 0xC0: Total Battery Capacity Setting   39000000            0.000001 Ah
  this->publish_state_(this->total_battery_capacity_setting_sensor_, (float) ant_get_32bit(75) * 0.000001f);
  //  79    0x02 0x53 0x06 0x11: Battery Capacity Remaining                           0.000001 Ah
  this->publish_state_(this->capacity_remaining_sensor_, (float) ant_get_32bit(79) * 0.000001f);
  //  83    0x00 0x08 0xC7 0x8E: Battery Cycle Capacity                               0.001 Ah
  this->publish_state_(this->battery_cycle_capacity_sensor_, (float) ant_get_32bit(83) * 0.001f);
  //  87    0x00 0x08 0x57 0x20: Uptime in seconds     546.592 s / 3600 = 151.83 h    s
  //  91    0x00 0x15: Temperature 1                   21°C                           1.0 °C
  //  93    0x00 0x15: Temperature 2                   21°C                           1.0 °C
  //  95    0x00 0x14: Temperature 3                   20°C                           1.0 °C
  //  97    0x00 0x14: Temperature 4                   20°C                           1.0 °C
  //  99    0x00 0x14: Temperature 5                   20°C                           1.0 °C
  //  101   0x00 0x14: Temperature 6                   20°C                           1.0 °C
  for (uint8_t i = 0; i < 6; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_, (float) ant_get_16bit(i * 2 + 91));
  }

  //  103   0x01: Charge MOSFET Status
  //  104   0x01: Discharge MOSFET Status
  //  105   0x00: Balancer Status
  //  106   0x03 0xE8: Tire length                                                    mm
  //  108   0x00 0x17: Number of pulses per week
  //  110   0x01: Relay switch
  //  111   0x00 0x00 0x00 0x00: Current power         0W                             1.0 W
  this->publish_state_(this->power_sensor_, (float) get_signed_float(ant_get_32bit(111)) * 0.1f);
  //  115   0x0D: Cell with the highest voltage        Cell 13
  //  116   0x10 0x2C: Maximum cell voltage            4140 * 0.001 = 4.140V          0.001 V
  this->publish_state_(this->min_cell_voltage_sensor_, (float) ant_get_16bit(116) * 0.001f);
  //  118   0x09: Cell with the lowest voltage         Cell 9
  //  119   0x10 0x26: Minimum cell voltage            4134 * 0.001 = 4.134V          0.001 V
  this->publish_state_(this->max_cell_voltage_sensor_, (float) ant_get_16bit(119) * 0.001f);
  //  121   0x10 0x28: Average cell voltage            4136 * 0.001 = 4.136V          0.001 V
  this->publish_state_(this->average_cell_voltage_sensor_, (float) ant_get_16bit(121) * 0.001f);
  //  123   0x0D: Battery strings                      13
  this->publish_state_(this->battery_strings_sensor_, (float) data[123]);
  //  124   0x00 0x00: Discharge MOSFET, voltage between D-S                          0.1 V
  //  126   0x00 0x73: Drive voltage (discharge MOSFET)                               0.1 V
  //  128   0x00 0x6F: Drive voltage (charge MOSFET)                                  0.1 V
  //  130   0x02 0xA7: When the detected current is 0, the initial value of the comparator
  //  132   0x00 0x00 0x00 0x00: Battery is in balance bitmask (Bit 1 = Cell 1, Bit 2 = Cell 2, ...)
  //  136   0x11 0x62: System log / overall status bitmask?
  //  138   0x0B 0x00: CRC
}

void AntBms::update() {
  this->read_registers();

  if (this->enable_fake_traffic_) {
    this->on_ant_modbus_data(
        FUNCTION_READ_ALL,
        {
            0xAA, 0x55, 0xAA, 0xFF, 0x02, 0x1D, 0x10, 0x2A, 0x10, 0x2A, 0x10, 0x27, 0x10, 0x2A, 0x10, 0x2A, 0x10, 0x28,
            0x10, 0x28, 0x10, 0x28, 0x10, 0x26, 0x10, 0x29, 0x10, 0x28, 0x10, 0x29, 0x10, 0x2C, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x64, 0x02, 0x53, 0x17, 0xC0, 0x02, 0x53, 0x06, 0x11, 0x00, 0x08, 0xC7, 0x8E, 0x00, 0x08, 0x57,
            0x20, 0x00, 0x15, 0x00, 0x15, 0x00, 0x14, 0x00, 0x14, 0x00, 0x14, 0x00, 0x14, 0x01, 0x01, 0x00, 0x03, 0xE8,
            0x00, 0x17, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x10, 0x2C, 0x09, 0x10, 0x26, 0x10, 0x28, 0x0D, 0x00, 0x00,
            0x00, 0x73, 0x00, 0x6F, 0x02, 0xA7, 0x00, 0x00, 0x00, 0x00, 0x11, 0x62, 0x0B, 0x24,
        });
  }
}

void AntBms::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBms::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBms:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);

  LOG_SENSOR("", "Battery Strings", this->battery_strings_sensor_);
  LOG_SENSOR("", "Total Voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "SoC", this->soc_sensor_);
  LOG_SENSOR("", "Total Battery Capacity Setting", this->total_battery_capacity_setting_sensor_);
  LOG_SENSOR("", "Capacity Remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Battery Cycle Capacity", this->battery_cycle_capacity_sensor_);
  LOG_SENSOR("", "Average cell voltage sensor", this->average_cell_voltage_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Minimum cell voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Maximum cell voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Temperature 1", this->temperatures_[0].temperature_sensor_);
  LOG_SENSOR("", "Temperature 2", this->temperatures_[1].temperature_sensor_);
  LOG_SENSOR("", "Temperature 3", this->temperatures_[2].temperature_sensor_);
  LOG_SENSOR("", "Temperature 4", this->temperatures_[3].temperature_sensor_);
  LOG_SENSOR("", "Temperature 5", this->temperatures_[4].temperature_sensor_);
  LOG_SENSOR("", "Temperature 6", this->temperatures_[5].temperature_sensor_);
  LOG_SENSOR("", "Cell Voltage 1", this->cells_[0].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 2", this->cells_[1].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 3", this->cells_[2].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 4", this->cells_[3].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 5", this->cells_[4].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 6", this->cells_[5].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 7", this->cells_[6].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 8", this->cells_[7].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 9", this->cells_[8].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 10", this->cells_[9].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 11", this->cells_[10].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 12", this->cells_[11].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 13", this->cells_[12].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 14", this->cells_[13].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 15", this->cells_[14].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 16", this->cells_[15].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 17", this->cells_[16].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 18", this->cells_[17].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 19", this->cells_[18].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 20", this->cells_[19].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 21", this->cells_[20].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 22", this->cells_[21].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 23", this->cells_[22].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 24", this->cells_[23].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 25", this->cells_[24].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 26", this->cells_[25].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 27", this->cells_[26].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 28", this->cells_[27].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 29", this->cells_[28].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 30", this->cells_[29].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 31", this->cells_[30].cell_voltage_sensor_);
  LOG_SENSOR("", "Cell Voltage 32", this->cells_[31].cell_voltage_sensor_);
}

}  // namespace ant_bms
}  // namespace esphome
