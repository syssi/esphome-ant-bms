#include "ant_bms.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <array>

namespace esphome::ant_bms {

static const char *const TAG = "ant_bms";

static const uint8_t MAX_NO_RESPONSE_COUNT = 5;

static const uint8_t ANT_PKT_START_1 = 0x7E;
static const uint8_t ANT_PKT_START_2 = 0xA1;

static const uint8_t ANT_FRAME_TYPE_STATUS = 0x11;
static const uint8_t ANT_FRAME_TYPE_DEVICE_INFO = 0x12;

static const uint8_t ANT_COMMAND_STATUS = 0x01;
static const uint8_t ANT_COMMAND_DEVICE_INFO = 0x02;

static const uint8_t CHARGE_MOSFET_STATUS_SIZE = 21;
static constexpr const char *const CHARGE_MOSFET_STATUS[CHARGE_MOSFET_STATUS_SIZE] = {
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
    "Reserved",                      // 0x0B
    "Open failed",                   // 0x0C
    "Discharge MOSFET abnormality",  // 0x0D
    "Waiting",                       // 0x0E
    "Manually turned off",           // 0x0F
    "Two level exceed voltage",      // 0x10
    "Low temperature protection",    // 0x11
    "Voltage difference exceeded",   // 0x12
    "Reserved",                      // 0x13
    "Self detect error",             // 0x14
};

static const uint8_t DISCHARGE_MOSFET_STATUS_SIZE = 20;
static constexpr const char *const DISCHARGE_MOSFET_STATUS[DISCHARGE_MOSFET_STATUS_SIZE] = {
    "Off",                           // 0x00
    "On",                            // 0x01
    "Overdischarge protection",      // 0x02
    "Over current protection",       // 0x03
    "Two current exceeded",          // 0x04
    "Total pressure undervoltage",   // 0x05
    "Battery over temperature",      // 0x06
    "MOSFET over temperature",       // 0x07
    "Abnormal current",              // 0x08
    "Balanced line dropped string",  // 0x09
    "Motherboard over temperature",  // 0x0A
    "Charge MOSFET on",              // 0x0B
    "Short circuit protection",      // 0x0C
    "Discharge MOSFET abnormality",  // 0x0D
    "Open failed",                   // 0x0E
    "Manually turned off",           // 0x0F
    "Two level low voltage",         // 0x10
    "Low temperature protection",    // 0x11
    "Voltage difference exceeded",   // 0x12
    "Self detect error",             // 0x13
};

static const uint8_t BATTERY_STATUS_SIZE = 6;
static constexpr const char *const BATTERY_STATUS[BATTERY_STATUS_SIZE] = {
    "Unknown",    // 0x00
    "Idle",       // 0x01
    "Charge",     // 0x02
    "Discharge",  // 0x03
    "Standby",    // 0x04
    "Error",      // 0x05
};

static const uint8_t BALANCER_STATUS_SIZE = 11;
static constexpr const char *const BALANCER_STATUS[BALANCER_STATUS_SIZE] = {
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

struct SettingsRegister {
  uint16_t address;
  const char *name;
  float scale;
  const char *unit;
};
static const SettingsRegister SETTINGS_REGISTERS[] = {
    {0x0000, "CellOvervoltageProtection", 0.001f, "V"},
    {0x0002, "CellOvervoltageRecovery", 0.001f, "V"},
    {0x0004, "CellOvervoltageProtectionL2", 0.001f, "V"},
    {0x0006, "CellOvervoltageRecoveryL2", 0.001f, "V"},
    {0x0008, "PackOvervoltageProtection", 0.1f, "V"},
    {0x000A, "PackOvervoltageRecovery", 0.1f, "V"},
    {0x000C, "CellUndervoltageProtection", 0.001f, "V"},
    {0x000E, "CellUndervoltageRecovery", 0.001f, "V"},
    {0x0010, "CellUndervoltageProtectionL2", 0.001f, "V"},
    {0x0012, "CellUndervoltageRecoveryL2", 0.001f, "V"},
    {0x0014, "PackUndervoltageProtection", 0.1f, "V"},
    {0x0016, "PackUndervoltageRecovery", 0.1f, "V"},
    {0x0018, "CellVoltageDifferenceProtection", 0.001f, "V"},
    {0x001A, "CellVoltageDifferenceRecovery", 0.001f, "V"},
    {0x0020, "CellOvervoltageWarning", 0.001f, "V"},
    {0x0022, "CellOvervoltageWarningRecovery", 0.001f, "V"},
    {0x0024, "PackOvervoltageWarning", 0.1f, "V"},
    {0x0026, "PackOvervoltageWarningRecovery", 0.1f, "V"},
    {0x0028, "CellUndervoltageWarning", 0.001f, "V"},
    {0x002A, "CellUndervoltageWarningRecovery", 0.001f, "V"},
    {0x002C, "PackUndervoltageWarning", 0.1f, "V"},
    {0x002E, "PackUndervoltageWarningRecovery", 0.1f, "V"},
    {0x0030, "CellVoltageDifferenceWarning", 0.001f, "V"},
    {0x0032, "CellVoltageDifferenceWarningRecovery", 0.001f, "V"},
    {0x0068, "ChargeOvercurrentProtection", 0.1f, "A"},
    {0x006A, "ChargeOvercurrentProtectionDelay", 1.0f, "s"},
    {0x006C, "DischargeOvercurrentProtection", 0.1f, "A"},
    {0x006E, "DischargeOvercurrentProtectionDelay", 1.0f, "s"},
    {0x0070, "DischargeOvercurrentProtectionL2", 0.1f, "A"},
    {0x0072, "DischargeOvercurrentProtectionDelayL2", 1.0f, "ms"},
    {0x0074, "ShortCircuitProtection", 1.0f, "A"},
    {0x0076, "ShortCircuitProtectionDelay", 1.0f, "us"},
    {0x007C, "ChargeOvercurrentWarning", 0.1f, "A"},
    {0x007E, "ChargeOvercurrentWarningRecovery", 0.1f, "A"},
    {0x0080, "DischargeOvercurrentWarning", 0.1f, "A"},
    {0x0082, "DischargeOvercurrentWarningRecovery", 0.1f, "A"},
    {0x0084, "SOCLowLevel1Warning", 1.0f, "%"},
    {0x0086, "SOCLowLevel2Warning", 1.0f, "%"},
    {0x008C, "CellBalancingVoltage", 0.001f, "V"},
    {0x008E, "CellBalancingStartVoltage", 0.001f, "V"},
    {0x0090, "CellVoltageDifferenceBalancingOn", 0.001f, "V"},
    {0x0092, "CellVoltageDifferenceBalancingOff", 0.001f, "V"},
    {0x0094, "BalancingCurrent", 1.0f, "mA"},
    {0x0096, "BalancingChargingCurrent", 0.1f, "A"},
    {0x0098, "CellType", 1.0f, ""},
    {0x009A, "CellNumber", 1.0f, "S"},
    {0x009C, "CellInternalResistanceCalibration", 1.0f, "mOhm"},
    {0x009E, "ShutdownVoltage", 0.001f, "V"},
    {0x00A0, "RequestChargeCurrent", 0.1f, "A"},
    {0x00A2, "NominalCapacity", 1.0f, "Ah"},
    {0x00A6, "RemainingCapacity", 1.0f, "Ah"},
    {0x00AA, "TotalCycleCapacity", 1.0f, "Ah"},
    {0x00C4, "StateOfChargeMethod", 1.0f, ""},
    {0x017A, "TireLength", 1.0f, "mm"},
    {0x017C, "PulseValue", 1.0f, ""},
    {0x017E, "SecondaryModuleNum", 1.0f, ""},
};

void AntBms::loop() {
  const uint32_t now = millis();
  if (now - this->last_byte_ > this->rx_timeout_) {
    this->rx_buffer_.clear();
    this->last_byte_ = now;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    if (this->parse_ant_bms_byte_(byte)) {
      this->last_byte_ = now;
    } else {
      this->rx_buffer_.clear();
    }
  }
}

// New protocol response frame
//
// 0x7E 0xA1 0x43 0x6A 0x01 0x02 0x05 0x00 0x1E 0x5C 0xAA 0x55    Password ACK
// 0x7E 0xA1 0x61 0x04 0x00 0x02 0x01 0x00 0xF2 0x2B 0xAA 0x55    Register 0x04 ACK
// 0x7E 0xA1 0x61 0x06 0x00 0x02 0x01 0x00 0x8B 0xEB 0xAA 0x55    Register 0x06 ACK
//  0    1    2    3    4    5    6    7    8    9    10   11
// head add  func val  val  len  data data crc  crc  end  end

bool AntBms::parse_ant_bms_byte_(uint8_t byte) {
  size_t at = this->rx_buffer_.size();
  this->rx_buffer_.push_back(byte);
  const uint8_t *raw = &this->rx_buffer_[0];

  if (at == 0)
    return true;

  // Byte 0...3: Header
  if (at < 4)
    return true;

  if (raw[0] != ANT_PKT_START_1 || raw[1] != ANT_PKT_START_2) {
    ESP_LOGW(TAG, "Invalid header");
    return false;
  }

  // Byte 0...5
  if (at < 6)
    return true;

  uint16_t frame_len = 6 + raw[5] + 4;

  if (at < frame_len - 1)
    return true;

  uint8_t function = raw[2];

  uint16_t computed_crc = crc16(raw + 1, frame_len - 5);
  uint16_t remote_crc = uint16_t(raw[frame_len - 3]) << 8 | (uint16_t(raw[frame_len - 4]) << 0);
  if (computed_crc != remote_crc) {
    ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
    return false;
  }

  ESP_LOGVV(TAG, "RX <- %s", format_hex_pretty(raw, at + 1).c_str());  // NOLINT

  std::vector<uint8_t> data(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);

  this->on_ant_bms_data_(function, data);

  // return false to reset buffer
  return false;
}

void AntBms::on_ant_bms_data(const std::vector<uint8_t> &data) {
  if (data.size() < 3)
    return;
  this->on_ant_bms_data_(data[2], data);
}

void AntBms::on_ant_bms_data_(const uint8_t &function, const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  switch (function) {
    case ANT_FRAME_TYPE_STATUS:
      this->on_status_data_(data);
      break;
    case ANT_FRAME_TYPE_DEVICE_INFO: {
      if (data.size() < 5)
        break;
      uint16_t addr = data[3] | (uint16_t(data[4]) << 8);
      if (addr == 0x026c) {
        this->on_device_info_data_(data);
      } else {
        this->on_settings_data_(data);
      }
      break;
    }
    default:
      ESP_LOGW(TAG, "Unhandled response received (function 0x%02X): %s", function,
               format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT
  }
}

void AntBms::on_status_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 1]) << 8) | (uint16_t(data[i + 0]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 2)) << 16) | (uint32_t(ant_get_16bit(i + 0)) << 0);
  };

  ESP_LOGI(TAG, "Status frame (%zu bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT

  if (data.size() < 6 || data.size() != (6 + data[5] + 4)) {
    ESP_LOGW(TAG, "Skipping status frame because of invalid length");
    return;
  }

  // Status request
  // -> 0x7e 0xa1 0x01 0x00 0x00 0xbe 0x18 0x55 0xaa 0x55
  //
  // Status response
  //
  // Byte Len Payload     Description                      Unit  Precision
  //   0   2  0x7E 0xA1   Start of frame
  //   2   1  0x11        Function
  //   3   2  0x00 0x00   Address
  //   5   1  0x8E        Data length
  //   6   1  0x05        Permissions
  ESP_LOGI(TAG, "  Permissions: %d", data[6]);

  //   7   1  0x01        Battery status (0: Unknown, 1: Idle, 2: Charge, 3: Discharge, 4: Standby, 5: Error)
  uint8_t raw_battery_status = data[7];
  this->publish_state_(this->battery_status_code_sensor_, (float) raw_battery_status);
  if (raw_battery_status < BATTERY_STATUS_SIZE) {
    this->publish_state_(this->battery_status_text_sensor_, BATTERY_STATUS[raw_battery_status]);
  } else {
    this->publish_state_(this->battery_status_text_sensor_, "Unknown");
  }

  //   8   1  0x04        Number of temperature sensors       max 4.
  uint8_t temperature_sensors = data[8];
  ESP_LOGI(TAG, "  Number of temperature sensors: %d", temperature_sensors);

  //   9   1  0x0E        Number of cells (14)                max 32
  uint8_t cells = data[9];
  this->publish_state_(this->battery_strings_sensor_, cells * 1.0f);

  //  10   8  0x02 0x00 0x00 0x00 0x00 0x00 0x00 0x00   Protection bitmask
  //  18   8  0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00   Warning bitmask
  //  26   8  0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00   Balancing? bitmask

  //  34   2  Cell voltage 1             uint16_t
  for (uint8_t i = 0; i < cells; i++) {
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, ant_get_16bit(i * 2 + 34) * 0.001f);
  }

  uint8_t offset = cells * 2;

  for (uint8_t i = 0; i < temperature_sensors; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_,
                         ((int16_t) ant_get_16bit(i * 2 + 34 + offset)) * 1.0f);
  }

  offset = offset + (temperature_sensors * 2);

  //  Mosfet temperature          int16_t
  this->publish_state_(this->temperatures_[temperature_sensors].temperature_sensor_,
                       ((int16_t) ant_get_16bit(34 + offset)) * 1.0f);

  //  Balancer temperature        int16_t
  this->publish_state_(this->temperatures_[temperature_sensors + 1].temperature_sensor_,
                       ((int16_t) ant_get_16bit(36 + offset)) * 1.0f);

  //  Total voltage              uint16_t
  this->publish_state_(this->total_voltage_sensor_, ant_get_16bit(38 + offset) * 0.01f);

  //  Current                     int16_t
  this->publish_state_(this->current_sensor_, ((int16_t) ant_get_16bit(40 + offset)) * 0.1f);

  //  State of charge            uint16_t
  this->publish_state_(this->soc_sensor_, ((int16_t) ant_get_16bit(42 + offset)) * 1.0f);

  //  State of health            uint16_t
  this->publish_state_(this->state_of_health_sensor_, ant_get_16bit(44 + offset) * 1.0f);

  //  Charge MOS status
  uint8_t raw_charge_mosfet_status = data[46 + offset];
  this->publish_state_(this->charge_mosfet_status_code_sensor_, (float) raw_charge_mosfet_status);
  if (raw_charge_mosfet_status < CHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, CHARGE_MOSFET_STATUS[raw_charge_mosfet_status]);
  } else {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->charging_switch_, (bool) (raw_charge_mosfet_status == 0x01));

  //  Discharge MOS status
  uint8_t raw_discharge_mosfet_status = data[47 + offset];
  this->publish_state_(this->discharge_mosfet_status_code_sensor_, (float) raw_discharge_mosfet_status);
  if (raw_discharge_mosfet_status < DISCHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_,
                         DISCHARGE_MOSFET_STATUS[raw_discharge_mosfet_status]);
  } else {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->discharging_switch_, (bool) (raw_discharge_mosfet_status == 0x01));

  //  Balancer status
  uint8_t raw_balancer_status = data[48 + offset];
  this->publish_state_(this->balancer_status_code_sensor_, (float) raw_balancer_status);
  this->publish_state_(this->balancer_switch_, (bool) (raw_balancer_status == 0x04));
  if (raw_balancer_status < BALANCER_STATUS_SIZE) {
    this->publish_state_(this->balancer_status_text_sensor_, BALANCER_STATUS[raw_balancer_status]);
  } else {
    this->publish_state_(this->balancer_status_text_sensor_, "Unknown");
  }

  //  Reserved
  //  Battery capacity            uint32_t
  this->publish_state_(this->total_battery_capacity_setting_sensor_, ant_get_32bit(50 + offset) * 0.000001f);

  //  Battery capacity remaining  uint32_t
  this->publish_state_(this->capacity_remaining_sensor_, ant_get_32bit(54 + offset) * 0.000001f);

  //  Total battery cycles capacity     uint32_t
  this->publish_state_(this->battery_cycle_capacity_sensor_, ant_get_32bit(58 + offset) * 0.001f);

  //  Power
  this->publish_state_(this->power_sensor_, ((int32_t) ant_get_32bit(62 + offset)) * 1.0f);

  //  Total runtime
  uint32_t raw_total_runtime = ant_get_32bit(66 + offset);
  this->publish_state_(this->total_runtime_sensor_, (float) raw_total_runtime);
  this->publish_state_(this->total_runtime_formatted_text_sensor_, format_total_runtime_(raw_total_runtime));

  //  Balanced cell bitmask
  this->publish_state_(this->balanced_cell_bitmask_sensor_, (float) ant_get_32bit(70 + offset));

  //  Maximum cell voltage
  this->publish_state_(this->max_cell_voltage_sensor_, ant_get_16bit(74 + offset) * 0.001f);

  //  Maximum voltage cell
  this->publish_state_(this->max_voltage_cell_sensor_, ant_get_16bit(76 + offset) * 1.0f);

  //  Minimum cell voltage
  this->publish_state_(this->min_cell_voltage_sensor_, ant_get_16bit(78 + offset) * 0.001f);

  //  Minimum voltage cell
  this->publish_state_(this->min_voltage_cell_sensor_, ant_get_16bit(80 + offset) * 1.0f);

  //  Delta cell voltage
  this->publish_state_(this->delta_cell_voltage_sensor_, ant_get_16bit(82 + offset) * 0.001f);

  //  Average cell voltage
  this->publish_state_(this->average_cell_voltage_sensor_, ant_get_16bit(84 + offset) * 0.001f);

  //  Discharge MOSFET, voltage between D-S
  //  Drive voltage (discharge MOSFET)
  //  Drive voltage (charge MOSFET)
  //  F40com
  //  Battery type
  //  Total discharging capacity
  this->publish_state_(this->total_discharging_capacity_sensor_, ant_get_32bit(96 + offset) * 0.001f);

  //  Total charging capacity
  this->publish_state_(this->total_charging_capacity_sensor_, ant_get_32bit(100 + offset) * 0.001f);

  //  Total discharging time
  uint32_t raw_total_discharging_time = ant_get_32bit(104 + offset);
  this->publish_state_(this->total_discharging_time_sensor_, (float) raw_total_discharging_time);
  this->publish_state_(this->total_discharging_time_formatted_text_sensor_,
                       format_total_runtime_(raw_total_discharging_time));

  //  Total charging time
  uint32_t raw_total_charging_time = ant_get_32bit(108 + offset);
  this->publish_state_(this->total_charging_time_sensor_, (float) raw_total_charging_time);
  this->publish_state_(this->total_charging_time_formatted_text_sensor_,
                       format_total_runtime_(raw_total_charging_time));
}

void AntBms::on_device_info_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Device info frame (%zu bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());  // NOLINT

  if (data.size() < 38) {
    ESP_LOGW(TAG, "Skipping device info frame because of invalid length");
    return;
  }

  auto device_model_begin = data.begin() + 6;
  this->publish_state_(this->device_model_text_sensor_,
                       std::string(device_model_begin, std::find(device_model_begin, data.begin() + 6 + 16, '\0')));

  auto software_version_begin = data.begin() + 22;
  this->publish_state_(
      this->software_version_text_sensor_,
      std::string(software_version_begin, std::find(software_version_begin, data.begin() + 22 + 16, '\0')));
}

void AntBms::on_settings_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 1]) << 8) | (uint16_t(data[i + 0]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 2)) << 16) | (uint32_t(ant_get_16bit(i + 0)) << 0);
  };

  uint16_t address = ant_get_16bit(3);
  uint8_t data_len = data[5];

  for (const auto &p : SETTINGS_REGISTERS) {
    if (p.address != address)
      continue;
    float value = (data_len == 4 ? ant_get_32bit(6) : ant_get_16bit(6)) * p.scale;
    ESP_LOGI(TAG, "  %s: %.3f %s", p.name, value, p.unit);
    return;
  }
  ESP_LOGW(TAG, "Unknown settings address: 0x%04X", address);
}

void AntBms::update() {
  this->track_online_status_();
  this->read_registers_();
}

void AntBms::write_register(uint8_t address, uint16_t value) { this->send_(0x51, address, value); }

std::array<uint8_t, 10> AntBms::build_settings_frame(uint16_t address, uint8_t data_len) {
  std::array<uint8_t, 10> frame{};
  frame[0] = 0x7e;
  frame[1] = 0xa1;
  frame[2] = ANT_COMMAND_DEVICE_INFO;
  frame[3] = address & 0xFF;
  frame[4] = address >> 8;
  frame[5] = data_len;
  auto crc = crc16(frame.data() + 1, 5);
  frame[6] = crc >> 0;
  frame[7] = crc >> 8;
  frame[8] = 0xaa;
  frame[9] = 0x55;
  return frame;
}

void AntBms::read_settings(uint16_t address) {
  static const uint16_t UINT32_ADDRESSES[] = {0x00a2, 0x00a6, 0x00aa};
  uint8_t data_len = 0x02;
  for (auto a : UINT32_ADDRESSES) {
    if (a == address) {
      data_len = 0x04;
      break;
    }
  }
  auto frame = build_settings_frame(address, data_len);
  this->write_array(frame.data(), frame.size());
  this->flush();
}

void AntBms::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void AntBms::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void AntBms::publish_device_unavailable_() {
  this->publish_state_(this->online_status_binary_sensor_, false);
  this->publish_state_(this->discharge_mosfet_status_text_sensor_, "Offline");
  this->publish_state_(this->charge_mosfet_status_text_sensor_, "Offline");
  this->publish_state_(this->balancer_status_text_sensor_, "Offline");
  this->publish_state_(this->total_runtime_formatted_text_sensor_, "Offline");
  this->publish_state_(this->battery_status_text_sensor_, "Offline");
  this->publish_state_(this->total_discharging_time_formatted_text_sensor_, "Offline");
  this->publish_state_(this->total_charging_time_formatted_text_sensor_, "Offline");

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
  this->publish_state_(state_of_health_sensor_, NAN);
  this->publish_state_(battery_status_code_sensor_, NAN);
  this->publish_state_(total_discharging_capacity_sensor_, NAN);
  this->publish_state_(total_charging_capacity_sensor_, NAN);
  this->publish_state_(total_discharging_time_sensor_, NAN);
  this->publish_state_(total_charging_time_sensor_, NAN);
  this->publish_state_(balanced_cell_bitmask_sensor_, NAN);

  for (auto &temperature : this->temperatures_) {
    this->publish_state_(temperature.temperature_sensor_, NAN);
  }

  for (auto &cell : this->cells_) {
    this->publish_state_(cell.cell_voltage_sensor_, NAN);
  }
}

void AntBms::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBms:");
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
  LOG_SENSOR("", "Delta Cell Voltage", this->delta_cell_voltage_sensor_);
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
  LOG_SENSOR("", "State Of Health", this->state_of_health_sensor_);
  LOG_SENSOR("", "Battery Status Code", this->battery_status_code_sensor_);
  LOG_SENSOR("", "Total Discharging Capacity", this->total_discharging_capacity_sensor_);
  LOG_SENSOR("", "Total Charging Capacity", this->total_charging_capacity_sensor_);
  LOG_SENSOR("", "Total Discharging Time", this->total_discharging_time_sensor_);
  LOG_SENSOR("", "Total Charging Time", this->total_charging_time_sensor_);
  LOG_SENSOR("", "Balanced Cell Bitmask", this->balanced_cell_bitmask_sensor_);

  LOG_TEXT_SENSOR("", "Discharge Mosfet Status", this->discharge_mosfet_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Charge Mosfet Status", this->charge_mosfet_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Balancer Status", this->balancer_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Total Runtime Formatted", this->total_runtime_formatted_text_sensor_);
  LOG_TEXT_SENSOR("", "Battery Status", this->battery_status_text_sensor_);
  LOG_TEXT_SENSOR("", "Total Discharging Time Formatted", this->total_discharging_time_formatted_text_sensor_);
  LOG_TEXT_SENSOR("", "Total Charging Time Formatted", this->total_charging_time_formatted_text_sensor_);
  LOG_TEXT_SENSOR("", "Device Model", this->device_model_text_sensor_);
  LOG_TEXT_SENSOR("", "Software Version", this->software_version_text_sensor_);

  this->check_uart_settings(19200);
}

float AntBms::get_setup_priority() const {
  // After UART bus
  return setup_priority::BUS - 1.0f;
}

void AntBms::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void AntBms::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBms::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void AntBms::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void AntBms::read_registers_() {
  uint8_t frame[10];
  frame[0] = 0x7e;
  frame[1] = 0xa1;
  frame[2] = ANT_COMMAND_STATUS;
  frame[3] = 0x00;
  frame[4] = 0x00;
  frame[5] = 0xbe;
  auto crc = crc16(frame + 1, 5);
  frame[6] = crc >> 0;
  frame[7] = crc >> 8;
  frame[8] = 0xaa;
  frame[9] = 0x55;

  this->write_array(frame, 10);
  this->flush();
}

void AntBms::authenticate_() {
  uint8_t frame[22];

  frame[0] = 0x7e;  // header
  frame[1] = 0xa1;  // header
  frame[2] = 0x23;  // control
  frame[3] = 0x6a;  // address
  frame[4] = 0x01;
  frame[5] = 0x0c;   // data len
  frame[6] = 0x31;   // 1
  frame[7] = 0x32;   // 2
  frame[8] = 0x33;   // 3
  frame[9] = 0x34;   // 4
  frame[10] = 0x35;  // 5
  frame[11] = 0x36;  // 6
  frame[12] = 0x37;  // 7
  frame[13] = 0x38;  // 8
  frame[14] = 0x39;  // 9
  frame[15] = 0x61;  // a
  frame[16] = 0x62;  // b
  frame[17] = 0x63;  // c

  auto crc = crc16(frame + 1, 17);
  frame[18] = crc >> 0;  // CRC
  frame[19] = crc >> 8;  // CRC

  frame[20] = 0xaa;
  frame[21] = 0x55;

  this->write_array(frame, 22);
  this->flush();
}

void AntBms::authenticate_variable_(const uint8_t *data, uint8_t data_len) {
  std::vector<uint8_t> frame = {0x7E, 0xA1, 0x23, 0x6A, 0x01};
  frame.push_back(data_len);
  for (int i = 0; i < data_len; i++) {
    frame.push_back(data[i]);
  }
  auto crc = crc16(frame.data() + 1, frame.size());
  frame.push_back(crc >> 0);
  frame.push_back(crc >> 8);
  frame.push_back(0xAA);
  frame.push_back(0x55);

  this->write_array(frame.data(), frame.size());
  this->flush();
}

std::array<uint8_t, 10> AntBms::build_frame(uint8_t function, uint8_t address, uint16_t value) {
  std::array<uint8_t, 10> frame{};
  frame[0] = 0x7e;
  frame[1] = 0xa1;
  frame[2] = function;
  frame[3] = address;
  frame[4] = value >> 8;
  frame[5] = value >> 0;
  auto crc = crc16(frame.data() + 1, 5);
  frame[6] = crc >> 0;
  frame[7] = crc >> 8;
  frame[8] = 0xaa;
  frame[9] = 0x55;
  return frame;
}

void AntBms::send_(uint8_t function, uint8_t address, uint16_t value) {
  this->authenticate_();
  auto frame = build_frame(function, address, value);
  this->write_array(frame.data(), frame.size());
  this->flush();
}

}  // namespace esphome::ant_bms
