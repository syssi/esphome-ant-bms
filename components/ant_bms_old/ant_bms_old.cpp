#include "ant_bms_old.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ant_bms_old {

static const char *const TAG = "ant_bms_old";

static const uint8_t MAX_NO_RESPONSE_COUNT = 5;

static const uint16_t STATUS_FRAME_LENGTH = 140;

static const uint8_t ANT_PKT_START_1 = 0xAA;
static const uint8_t ANT_PKT_START_2 = 0x55;
static const uint8_t ANT_PKT_START_3 = 0xAA;

static const uint8_t FUNCTION_READ_ALL = 0xFF;
static const uint8_t WRITE_SINGLE_REGISTER = 0xA5;
static const uint8_t REGISTER_APPLY_WRITE = 0xFF;

static const uint8_t CHARGE_MOSFET_STATUS_SIZE = 16;
static const char *const CHARGE_MOSFET_STATUS[CHARGE_MOSFET_STATUS_SIZE] = {
    "Off",                           // 0x00
    "On",                            // 0x01
    "Overcharge protection",         // 0x02
    "Over current protection",       // 0x03
    "Battery full",                  // 0x04
    "Total overpressure",            // 0x05
    "Battery over temperature",      // 0x06
    "MOSFET over temperature",       // 0x07
    "Abnormal current",              // 0x08
    "Balanced line dropped string",  // 0x09
    "Motherboard over temperature",  // 0x0A
    "Unknown",                       // 0x0B
    "Unknown",                       // 0x0C
    "Discharge MOSFET abnormality",  // 0x0D
    "Unknown",                       // 0x0E
    "Manually turned off",           // 0x0F
};

static const uint8_t DISCHARGE_MOSFET_STATUS_SIZE = 16;
static const char *const DISCHARGE_MOSFET_STATUS[DISCHARGE_MOSFET_STATUS_SIZE] = {
    "Off",                           // 0x00
    "On",                            // 0x01
    "Overdischarge protection",      // 0x02
    "Over current protection",       // 0x03
    "Unknown",                       // 0x04
    "Total pressure undervoltage",   // 0x05
    "Battery over temperature",      // 0x06
    "MOSFET over temperature",       // 0x07
    "Abnormal current",              // 0x08
    "Balanced line dropped string",  // 0x09
    "Motherboard over temperature",  // 0x0A
    "Charge MOSFET on",              // 0x0B
    "Short circuit protection",      // 0x0C
    "Discharge MOSFET abnormality",  // 0x0D
    "Start exception",               // 0x0E
    "Manually turned off",           // 0x0F
};

static const uint8_t BALANCER_STATUS_SIZE = 11;
static const char *const BALANCER_STATUS[BALANCER_STATUS_SIZE] = {
    "Off",                                   // 0x00
    "Exceeds the limit equilibrium",         // 0x01
    "Charge differential pressure balance",  // 0x02
    "Balanced over temperature",             // 0x03
    "Automatic equalization",                // 0x04
    "Unknown",                               // 0x05
    "Unknown",                               // 0x06
    "Unknown",                               // 0x07
    "Unknown",                               // 0x08
    "Unknown",                               // 0x09
    "Motherboard over temperature",          // 0x0A
};

void AntBmsOld::loop() {
  const uint32_t now = millis();
  if (now - this->last_byte_ > this->rx_timeout_) {
    this->rx_buffer_.clear();
    this->last_byte_ = now;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_ant_bms_old_byte_(byte)) {
      this->last_byte_ = now;
    } else {
      this->rx_buffer_.clear();
    }
  }
}

bool AntBmsOld::parse_ant_bms_old_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];
  uint16_t frame_len = 140;

  // 0xAA 0x55 0xAA 0xFF 0x02 0x1D 0x10 0x2A ...
  //   0    1    2    3  ^^^^^ data ^^^^^^^^

  // Byte 0: Ant BMS start byte (match all)
  if (at == 0) {
    if (raw[0] != ANT_PKT_START_1) {
      ESP_LOGW(TAG, "Invalid header: 0x%02X", raw[0]);

      // return false to reset buffer
      return false;
    }

    return true;
  }

  // Byte 0...3: Header
  if (at < 3)
    return true;

  if (raw[0] != ANT_PKT_START_1 || raw[1] != ANT_PKT_START_2 || raw[2] != ANT_PKT_START_3 ||
      raw[3] != FUNCTION_READ_ALL) {
    ESP_LOGW(TAG, "Invalid header: 0x%02X 0x%02X 0x%02X 0x%02X", raw[0], raw[1], raw[2], raw[3]);

    // return false to reset buffer
    return false;
  }

  // Byte 0...139
  if (at < frame_len - 1)
    return true;

  uint16_t computed_crc = chksum_(raw, frame_len - 2);
  uint16_t remote_crc = uint16_t(raw[frame_len - 2]) << 8 | (uint16_t(raw[frame_len - 1]) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
    return false;
  }

  ESP_LOGVV(TAG, "RX <- %s", format_hex_pretty(raw, at + 1).c_str());

  std::vector<uint8_t> data(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);

  this->on_ant_bms_old_data(data);

  // return false to reset buffer
  return false;
}

void AntBmsOld::update() {
  this->track_online_status_();
  this->read_registers_();
}

void AntBmsOld::on_ant_bms_old_data(const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  if (data.size() == STATUS_FRAME_LENGTH) {
    this->on_status_data_(data);
    return;
  }

  ESP_LOGW(TAG, "Unhandled response (%zu bytes) received: %s", data.size(),
           format_hex_pretty(&data.front(), data.size()).c_str());
}

void AntBmsOld::on_status_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 0)) << 16) | (uint32_t(ant_get_16bit(i + 2)) << 0);
  };

  ESP_LOGI(TAG, "Status frame (%d bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

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
  float total_voltage = ant_get_16bit(4) * 0.1f;
  this->publish_state_(this->total_voltage_sensor_, total_voltage);
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
  float current = ((int32_t) ant_get_32bit(70)) * 0.1f;
  this->publish_state_(this->current_sensor_, current);
  //  74    0x64: SOC                                  100 %                          1.0 %
  this->publish_state_(this->soc_sensor_, (float) data[74]);
  //  75    0x02 0x53 0x17 0xC0: Total Battery Capacity Setting   39000000            0.000001 Ah
  this->publish_state_(this->total_battery_capacity_setting_sensor_, (float) ant_get_32bit(75) * 0.000001f);
  //  79    0x02 0x53 0x06 0x11: Battery Capacity Remaining                           0.000001 Ah
  this->publish_state_(this->capacity_remaining_sensor_, (float) ant_get_32bit(79) * 0.000001f);
  //  83    0x00 0x08 0xC7 0x8E: Battery Cycle Capacity                               0.001 Ah
  this->publish_state_(this->battery_cycle_capacity_sensor_, (float) ant_get_32bit(83) * 0.001f);
  //  87    0x00 0x08 0x57 0x20: Uptime in seconds     546.592s                       1.0 s
  this->publish_state_(this->total_runtime_sensor_, (float) ant_get_32bit(87));

  if (this->total_runtime_formatted_text_sensor_ != nullptr) {
    this->publish_state_(this->total_runtime_formatted_text_sensor_, format_total_runtime_(ant_get_32bit(87)));
  }
  //  91    0x00 0x15: Temperature 1                   21°C                           1.0 °C
  //  93    0x00 0x15: Temperature 2                   21°C                           1.0 °C
  //  95    0x00 0x14: Temperature 3                   20°C                           1.0 °C
  //  97    0x00 0x14: Temperature 4                   20°C                           1.0 °C
  //  99    0x00 0x14: Temperature 5                   20°C                           1.0 °C
  //  101   0x00 0x14: Temperature 6                   20°C                           1.0 °C
  for (uint8_t i = 0; i < 6; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_, (float) ((int16_t) ant_get_16bit(i * 2 + 91)));
  }

  //  103   0x01: Charge MOSFET Status
  uint8_t raw_charge_mosfet_status = data[103];
  this->publish_state_(this->charge_mosfet_status_code_sensor_, (float) raw_charge_mosfet_status);
  if (raw_charge_mosfet_status < CHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, CHARGE_MOSFET_STATUS[raw_charge_mosfet_status]);
  } else {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->charging_switch_, (bool) (raw_charge_mosfet_status == 0x01));

  //  104   0x01: Discharge MOSFET Status
  uint8_t raw_discharge_mosfet_status = data[104];
  this->publish_state_(this->discharge_mosfet_status_code_sensor_, (float) raw_discharge_mosfet_status);
  if (raw_discharge_mosfet_status < DISCHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_,
                         DISCHARGE_MOSFET_STATUS[raw_discharge_mosfet_status]);
  } else {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->discharging_switch_, (bool) (raw_discharge_mosfet_status == 0x01));

  //  105   0x00: Balancer Status
  uint8_t raw_balancer_status = data[105];
  this->publish_state_(this->balancer_status_code_sensor_, (float) raw_balancer_status);
  this->publish_state_(this->balancer_switch_, (bool) (raw_balancer_status == 0x04));
  if (raw_balancer_status < BALANCER_STATUS_SIZE) {
    this->publish_state_(this->balancer_status_text_sensor_, BALANCER_STATUS[raw_balancer_status]);
  } else {
    this->publish_state_(this->balancer_status_text_sensor_, "Unknown");
  }

  //  106   0x03 0xE8: Tire length                                                    mm
  //  108   0x00 0x17: Number of pulses per week
  //  110   0x01: Relay switch
  //  111   0x00 0x00 0x00 0x00: Current power         0W                             1.0 W
  this->publish_state_(this->power_sensor_, (float) (int32_t) ant_get_32bit(111));
  //  115   0x0D: Cell with the highest voltage        Cell 13
  this->publish_state_(this->max_voltage_cell_sensor_, (float) data[115]);
  //  116   0x10 0x2C: Maximum cell voltage            4140 * 0.001 = 4.140V          0.001 V
  float max_cell_voltage = ant_get_16bit(116) * 0.001f;
  this->publish_state_(this->max_cell_voltage_sensor_, max_cell_voltage);
  //  118   0x09: Cell with the lowest voltage         Cell 9
  this->publish_state_(this->min_voltage_cell_sensor_, (float) data[118]);
  //  119   0x10 0x26: Minimum cell voltage            4134 * 0.001 = 4.134V          0.001 V
  float min_cell_voltage = ant_get_16bit(119) * 0.001f;
  this->publish_state_(this->min_cell_voltage_sensor_, min_cell_voltage);
  this->publish_state_(this->delta_cell_voltage_sensor_, max_cell_voltage - min_cell_voltage);
  //  121   0x10 0x28: Average cell voltage            4136 * 0.001 = 4.136V          0.001 V
  this->publish_state_(this->average_cell_voltage_sensor_, ant_get_16bit(121) * 0.001f);
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

void AntBmsOld::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void AntBmsOld::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void AntBmsOld::publish_device_unavailable_() {
  this->publish_state_(this->online_status_binary_sensor_, false);
  this->publish_state_(this->discharge_mosfet_status_text_sensor_, "Offline");
  this->publish_state_(this->charge_mosfet_status_text_sensor_, "Offline");
  this->publish_state_(this->balancer_status_text_sensor_, "Offline");
  this->publish_state_(this->total_runtime_formatted_text_sensor_, "Offline");

  this->publish_state_(battery_strings_sensor_, NAN);
  this->publish_state_(current_sensor_, NAN);
  this->publish_state_(soc_sensor_, NAN);
  this->publish_state_(total_battery_capacity_setting_sensor_, NAN);
  this->publish_state_(capacity_remaining_sensor_, NAN);
  this->publish_state_(battery_cycle_capacity_sensor_, NAN);
  this->publish_state_(total_voltage_sensor_, NAN);
  this->publish_state_(total_runtime_sensor_, NAN);
  this->publish_state_(average_cell_voltage_sensor_, NAN);
  this->publish_state_(power_sensor_, NAN);
  this->publish_state_(min_cell_voltage_sensor_, NAN);
  this->publish_state_(max_cell_voltage_sensor_, NAN);
  this->publish_state_(min_voltage_cell_sensor_, NAN);
  this->publish_state_(max_voltage_cell_sensor_, NAN);
  this->publish_state_(charge_mosfet_status_code_sensor_, NAN);
  this->publish_state_(discharge_mosfet_status_code_sensor_, NAN);
  this->publish_state_(balancer_status_code_sensor_, NAN);

  for (auto &temperature : this->temperatures_) {
    this->publish_state_(temperature.temperature_sensor_, NAN);
  }

  for (auto &cell : this->cells_) {
    this->publish_state_(cell.cell_voltage_sensor_, NAN);
  }
}

void AntBmsOld::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBmsOld:");
  ESP_LOGCONFIG(TAG, "  RX timeout: %d ms", this->rx_timeout_);

  LOG_SENSOR("", "Battery Strings", this->battery_strings_sensor_);
  LOG_SENSOR("", "Total Voltage", this->total_voltage_sensor_);
  LOG_SENSOR("", "Total Runtime", this->total_runtime_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "SoC", this->soc_sensor_);
  LOG_SENSOR("", "Total Battery Capacity Setting", this->total_battery_capacity_setting_sensor_);
  LOG_SENSOR("", "Capacity Remaining", this->capacity_remaining_sensor_);
  LOG_SENSOR("", "Battery Cycle Capacity", this->battery_cycle_capacity_sensor_);
  LOG_SENSOR("", "Average cell voltage sensor", this->average_cell_voltage_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Minimum cell voltage", this->min_cell_voltage_sensor_);
  LOG_SENSOR("", "Maximum cell voltage", this->max_cell_voltage_sensor_);
  LOG_SENSOR("", "Min Voltage Cell", this->min_voltage_cell_sensor_);
  LOG_SENSOR("", "Max Voltage Cell", this->max_voltage_cell_sensor_);
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
  LOG_SENSOR("", "Charge Mosfet Status Code", this->charge_mosfet_status_code_sensor_);
  LOG_SENSOR("", "Discharge Mosfet Status Code", this->discharge_mosfet_status_code_sensor_);
  LOG_SENSOR("", "Balancer Status Code", this->balancer_status_code_sensor_);

  LOG_TEXT_SENSOR("", "Discharge Mosfet Status", this->discharge_mosfet_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Charge Mosfet Status", this->charge_mosfet_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Balancer Status", this->balancer_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Total Runtime Formatted", this->total_runtime_formatted_text_sensor_);

  this->check_uart_settings(19200);
}

float AntBmsOld::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void AntBmsOld::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void AntBmsOld::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBmsOld::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void AntBmsOld::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void AntBmsOld::write_register(uint8_t address, uint16_t value) {
  this->authenticate_();
  this->send_(WRITE_SINGLE_REGISTER, address, value);
  this->send_(WRITE_SINGLE_REGISTER, REGISTER_APPLY_WRITE, 0x00);
}

void AntBmsOld::authenticate_() {
  if (this->password_.empty()) {
    ESP_LOGI(TAG, "Authentication skipped because there was no password provided");
    return;
  }

  ESP_LOGD(TAG, "Authenticate write command");
  const uint8_t *password = reinterpret_cast<const uint8_t *>(this->password_.c_str());
  for (uint8_t i = 0; i < 4; i++) {
    this->send_(WRITE_SINGLE_REGISTER, 0xF1 + i, (uint16_t(password[i * 2]) << 8) | uint16_t(password[(i * 2) + 1]));
  }
}

void AntBmsOld::send_(uint8_t function, uint8_t address, uint16_t value) {
  uint8_t frame[6];
  frame[0] = function;    // 0xA5 (write), 0x5A (read), 0xDB (read via BLE)
  frame[1] = function;    // 0xA5 (write), 0x5A (read), 0xDB (read via BLE)
  frame[2] = address;     // 0xFA (Charge MOSFET)
  frame[3] = value >> 8;  // 0x00
  frame[4] = value >> 0;  // 0x01 (On)
  frame[5] = frame[2] + frame[3] + frame[4];

  this->write_array(frame, 6);
  this->flush();
}

void AntBmsOld::read_registers_() { this->send_(0x5A, 0x00, 0x0001); }

}  // namespace ant_bms_old
}  // namespace esphome
