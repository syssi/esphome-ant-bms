#include "ant_bms_old_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ant_bms_old_ble {

static const char *const TAG = "ant_bms_old_ble";

static const uint8_t MAX_NO_RESPONSE_COUNT = 10;

static const uint16_t ANT_BMS_OLD_SERVICE_UUID = 0xFFE0;
static const uint16_t ANT_BMS_OLD_CHARACTERISTIC_UUID = 0xFFE1;  // Handle 0x10

static const uint16_t STATUS_FRAME_LENGTH = 140;

static const uint8_t ANT_PKT_START_1 = 0xAA;
static const uint8_t ANT_PKT_START_2 = 0x55;
static const uint8_t ANT_PKT_START_3 = 0xAA;

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

void AntBmsOldBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                       esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;

      if (this->characteristic_handle_ != 0) {
        auto status = esp_ble_gattc_unregister_for_notify(
            this->parent()->get_gattc_if(), this->parent()->get_remote_bda(), this->characteristic_handle_);
        if (status) {
          ESP_LOGW(TAG, "esp_ble_gattc_unregister_for_notify failed, status=%d", status);
        }
      }
      this->characteristic_handle_ = 0;

      this->frame_buffer_.clear();

      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      // [esp32_ble_client:064]: [0] [AA:BB:CC:92:23:45] 0x00 Attempting BLE connection
      // [esp32_ble_client:129]: [0] [AA:BB:CC:92:23:45] ESP_GATTC_OPEN_EVT
      // [esp32_ble_client:189]: [0] [AA:BB:CC:92:23:45] ESP_GATTC_SEARCH_CMPL_EVT
      // [esp32_ble_client:192]: [0] [AA:BB:CC:92:23:45] Service UUID: 0x1800
      // [esp32_ble_client:194]: [0] [AA:BB:CC:92:23:45]  start_handle: 0x1  end_handle: 0x9
      // [esp32_ble_client:192]: [0] [AA:BB:CC:92:23:45] Service UUID: 0x1801
      // [esp32_ble_client:194]: [0] [AA:BB:CC:92:23:45]  start_handle: 0xa  end_handle: 0xd
      // [esp32_ble_client:192]: [0] [AA:BB:CC:92:23:45] Service UUID: 0xFFE0
      // [esp32_ble_client:194]: [0] [AA:BB:CC:92:23:45]  start_handle: 0xe  end_handle: 0xffff
      // [esp32_ble_client:196]: [0] [AA:BB:CC:92:23:45] Connected
      // [esp32_ble_client:069]: [0] [AA:BB:CC:92:23:45]  characteristic 0xFFE1, handle 0x10, properties 0x1c
      // [esp32_ble_client:069]: [0] [AA:BB:CC:92:23:45]  characteristic 0xFFE2, handle 0x13, properties 0xc
      auto *characteristic =
          this->parent_->get_characteristic(ANT_BMS_OLD_SERVICE_UUID, ANT_BMS_OLD_CHARACTERISTIC_UUID);
      if (characteristic == nullptr) {
        ESP_LOGE(TAG, "[%s] No notify service found at device, not an ANT BMS..?",
                 this->parent_->address_str().c_str());
        break;
      }
      this->characteristic_handle_ = characteristic->handle;

      auto status = esp_ble_gattc_register_for_notify(this->parent()->get_gattc_if(), this->parent()->get_remote_bda(),
                                                      characteristic->handle);
      if (status) {
        ESP_LOGW(TAG, "esp_ble_gattc_register_for_notify failed, status=%d", status);
      }

      break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
      this->node_state = espbt::ClientState::ESTABLISHED;
      break;
    }
    case ESP_GATTC_NOTIFY_EVT: {
      if (param->notify.handle != this->characteristic_handle_)
        break;

      ESP_LOGVV(TAG, "Notification received: %s",
                format_hex_pretty(param->notify.value, param->notify.value_len).c_str());

      this->assemble(param->notify.value, param->notify.value_len);

      break;
    }
    default:
      break;
  }
}

void AntBmsOldBle::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > STATUS_FRAME_LENGTH) {
    ESP_LOGW(TAG, "Maximum response size (%d bytes) exceeded", this->frame_buffer_.size());
    this->frame_buffer_.clear();
  }

  // Flush buffer on every preamble
  if (data[0] == ANT_PKT_START_1 && data[1] == ANT_PKT_START_2 && data[2] == ANT_PKT_START_3) {
    this->frame_buffer_.clear();
  }

  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  if (this->frame_buffer_.size() == STATUS_FRAME_LENGTH) {
    const uint8_t *raw = &this->frame_buffer_[0];

    uint8_t function = raw[3];
    uint16_t computed_crc = chksum_(raw, STATUS_FRAME_LENGTH - 2);
    uint16_t remote_crc = uint16_t(raw[STATUS_FRAME_LENGTH - 2]) << 8 | (uint16_t(raw[STATUS_FRAME_LENGTH - 1]) << 0);
    if (computed_crc != remote_crc) {
      ESP_LOGW(TAG, "CRC check failed! 0x%04X != 0x%04X", computed_crc, remote_crc);
      this->frame_buffer_.clear();
      return;
    }

    std::vector<uint8_t> data(this->frame_buffer_.begin(), this->frame_buffer_.end());

    this->on_ant_bms_old_ble_data_(function, data);
    this->frame_buffer_.clear();
  }
}

void AntBmsOldBle::update() {
  this->track_online_status_();
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  // 0xdb 0xdb 0x00 0x00 0x00 0x00
  this->read_registers_();
}

void AntBmsOldBle::on_ant_bms_old_ble_data_(const uint8_t &function, const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  if (data.size() == STATUS_FRAME_LENGTH) {
    this->on_status_data_(data);
    return;
  }

  ESP_LOGW(TAG, "Unhandled response (%zu bytes) received: %s", data.size(),
           format_hex_pretty(&data.front(), data.size()).c_str());
}

void AntBmsOldBle::on_status_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 0)) << 16) | (uint32_t(ant_get_16bit(i + 2)) << 0);
  };

  ESP_LOGI(TAG, "Status frame (%d bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Status request
  // -> 0xDB 0xDB 0x00 0x00 0x00 0x00
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

void AntBmsOldBle::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void AntBmsOldBle::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void AntBmsOldBle::publish_device_unavailable_() {
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

void AntBmsOldBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBmsOldBle:");

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
}

void AntBmsOldBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void AntBmsOldBle::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBmsOldBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void AntBmsOldBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void AntBmsOldBle::write_register(uint8_t address, uint16_t value) {
  this->authenticate_();
  this->send_(WRITE_SINGLE_REGISTER, address, value);
  this->send_(WRITE_SINGLE_REGISTER, REGISTER_APPLY_WRITE, 0x00);
}

void AntBmsOldBle::authenticate_() {
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

bool AntBmsOldBle::send_(uint8_t function, uint8_t address, uint16_t value) {
  uint8_t frame[6];
  frame[0] = function;    // 0xA5 (write), 0x5A (read), 0xDB (read via BLE)
  frame[1] = function;    // 0xA5 (write), 0x5A (read), 0xDB (read via BLE)
  frame[2] = address;     // 0xFA (Charge MOSFET)
  frame[3] = value >> 8;  // 0x00
  frame[4] = value >> 0;  // 0x01 (On)
  frame[5] = frame[2] + frame[3] + frame[4];

  ESP_LOGVV(TAG, "Send command: %s", format_hex_pretty(frame, sizeof(frame)).c_str());
  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->characteristic_handle_, sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP,
                                         ESP_GATT_AUTH_REQ_NONE);

  if (status)
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);

  return (status == 0);
}

bool AntBmsOldBle::read_registers_() { return this->send_(0xDB, 0x00, 0x0000); }

}  // namespace ant_bms_old_ble
}  // namespace esphome
