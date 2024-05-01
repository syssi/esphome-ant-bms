#include "ant_bms_ble.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ant_bms_ble {

static const char *const TAG = "ant_bms_ble";

static const uint8_t MAX_NO_RESPONSE_COUNT = 10;

static const uint16_t ANT_BMS_SERVICE_UUID = 0xFFE0;
static const uint16_t ANT_BMS_CHARACTERISTIC_UUID = 0xFFE1;  // Handle 0x10

static const uint16_t MAX_RESPONSE_SIZE = 192;

static const uint8_t ANT_PKT_START_1 = 0x7E;
static const uint8_t ANT_PKT_START_2 = 0xA1;
static const uint8_t ANT_PKT_END_1 = 0xAA;
static const uint8_t ANT_PKT_END_2 = 0x55;

static const uint8_t ANT_FRAME_TYPE_STATUS = 0x11;
static const uint8_t ANT_FRAME_TYPE_DEVICE_INFO = 0x12;
static const uint8_t ANT_FRAME_TYPE_SYSTEM_LOG = 0x13;
static const uint8_t ANT_FRAME_TYPE_PERMISSION = 0x14;
static const uint8_t ANT_FRAME_TYPE_SYSTEM_INFO = 0x15;
static const uint8_t ANT_FRAME_TYPE_GPS_DATA = 0x16;
static const uint8_t ANT_FRAME_TYPE_UNKNOWN1 = 0x42;
static const uint8_t ANT_FRAME_TYPE_UNKNOWN2 = 0x43;
static const uint8_t ANT_FRAME_TYPE_UNKNOWN3 = 0x61;

static const uint8_t ANT_COMMAND_STATUS = 0x01;
static const uint8_t ANT_COMMAND_DEVICE_INFO = 0x02;
static const uint8_t ANT_COMMAND_WRITE_REGISTER = 0x51;

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

void AntBmsBle::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                    esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT: {
      break;
    }
    case ESP_GATTC_DISCONNECT_EVT: {
      this->node_state = espbt::ClientState::IDLE;
      break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
      // [esp32_ble_client:064]: [0] [16:AA:22:02:23:45] 0x00 Attempting BLE connection
      // [esp32_ble_client:126]: [0] [16:AA:22:02:23:45] ESP_GATTC_OPEN_EVT
      // [esp32_ble_client:186]: [0] [16:AA:22:02:23:45] ESP_GATTC_SEARCH_CMPL_EVT
      // [esp32_ble_client:189]: [0] [16:AA:22:02:23:45] Service UUID: 0x1800
      // [esp32_ble_client:191]: [0] [16:AA:22:02:23:45]  start_handle: 0x1  end_handle: 0x9
      // [esp32_ble_client:189]: [0] [16:AA:22:02:23:45] Service UUID: 0x1801
      // [esp32_ble_client:191]: [0] [16:AA:22:02:23:45]  start_handle: 0xa  end_handle: 0xd
      // [esp32_ble_client:189]: [0] [16:AA:22:02:23:45] Service UUID: 0xFFE0
      // [esp32_ble_client:191]: [0] [16:AA:22:02:23:45]  start_handle: 0xe  end_handle: 0xffff
      // [esp32_ble_client:193]: [0] [16:AA:22:02:23:45] Connected
      // [esp32_ble_client:069]: [0] [16:AA:22:02:23:45]  characteristic 0xFFE1, handle 0x10, properties 0x1c
      // [esp32_ble_client:069]: [0] [16:AA:22:02:23:45]  characteristic 0xFFE2, handle 0x13, properties 0xc

      auto *characteristic = this->parent_->get_characteristic(ANT_BMS_SERVICE_UUID, ANT_BMS_CHARACTERISTIC_UUID);
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

      ESP_LOGI(TAG, "Request device info frame");
      // 0x7e 0xa1 0x02 0x6c 0x02 0x20 0x58 0xc4 0xaa 0x55
      this->send_(ANT_COMMAND_DEVICE_INFO, 0x026c, 0x20, false);

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

void AntBmsBle::assemble(const uint8_t *data, uint16_t length) {
  if (this->frame_buffer_.size() > MAX_RESPONSE_SIZE) {
    ESP_LOGW(TAG, "Maximum response size (%d bytes) exceeded", this->frame_buffer_.size());
    this->frame_buffer_.clear();
  }

  // Flush buffer on every preamble
  if (data[0] == ANT_PKT_START_1 && data[1] == ANT_PKT_START_2) {
    this->frame_buffer_.clear();
  }

  this->frame_buffer_.insert(this->frame_buffer_.end(), data, data + length);

  if (this->frame_buffer_.back() == ANT_PKT_END_2) {
    const uint8_t *raw = &this->frame_buffer_[0];

    uint8_t function = raw[2];
    uint16_t data_len = raw[5];
    uint16_t frame_len = 6 + data_len + 4;
    // It looks like the data_len value of the device info frame is wrong
    if (frame_len != this->frame_buffer_.size() && function != ANT_FRAME_TYPE_DEVICE_INFO) {
      ESP_LOGW(TAG, "Invalid frame length");
      this->frame_buffer_.clear();
      return;
    }

    uint16_t computed_crc = crc16_(raw + 1, frame_len - 5);
    uint16_t remote_crc = uint16_t(raw[frame_len - 3]) << 8 | (uint16_t(raw[frame_len - 4]) << 0);
    if (computed_crc != remote_crc) {
      ESP_LOGW(TAG, "CRC Check failed! %04X != %04X", computed_crc, remote_crc);
      this->frame_buffer_.clear();
      return;
    }

    std::vector<uint8_t> data(this->frame_buffer_.begin(), this->frame_buffer_.end());

    this->on_ant_bms_ble_data_(function, data);
    this->frame_buffer_.clear();
  }
}

void AntBmsBle::update() {
  this->track_online_status_();
  if (this->node_state != espbt::ClientState::ESTABLISHED) {
    ESP_LOGW(TAG, "[%s] Not connected", this->parent_->address_str().c_str());
    return;
  }

  // 0x7e 0xa1 0x01 0x00 0x00 0xbe 0x18 0x55 0xaa 0x55
  this->send_(ANT_COMMAND_STATUS, 0x0000, 0xbe, false);
}

void AntBmsBle::on_ant_bms_ble_data_(const uint8_t &function, const std::vector<uint8_t> &data) {
  this->reset_online_status_tracker_();

  switch (function) {
    case ANT_FRAME_TYPE_STATUS:
      this->on_status_data_(data);
      break;
    case ANT_FRAME_TYPE_DEVICE_INFO:
      this->on_device_info_data_(data);
      break;
    default:
      ESP_LOGW(TAG, "Unhandled response received (function 0x%02X): %s", function,
               format_hex_pretty(&data.front(), data.size()).c_str());
  }
}

void AntBmsBle::on_status_data_(const std::vector<uint8_t> &data) {
  auto ant_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 1]) << 8) | (uint16_t(data[i + 0]) << 0);
  };
  auto ant_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(ant_get_16bit(i + 2)) << 16) | (uint32_t(ant_get_16bit(i + 0)) << 0);
  };

  ESP_LOGI(TAG, "Status frame (%d bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  if (data.size() != (6 + data[5] + 4)) {
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
  ESP_LOGI(TAG, "  Battery status: %d", data[7]);

  //   8   1  0x04        Number of temperature sensors       max 4.
  uint8_t temperature_sensors = data[8];
  ESP_LOGI(TAG, "  Number of temperature sensors: %d", temperature_sensors);

  //   9   1  0x0E        Number of cells (14)                max 32
  uint8_t cells = data[9];
  this->publish_state_(this->battery_strings_sensor_, cells * 1.0f);

  //  10   8  0x02 0x00 0x00 0x00 0x00 0x00 0x00 0x00   Protection bitmask
  //  18   8  0x00 0x00 0x00 0x01 0x00 0x00 0x00 0x00   Warning bitmask
  //  26   8  0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00   Balancing? bitmask

  //  34   2  0x11 0x10   Cell voltage 1             uint16_t
  //  36   2  0x11 0x10   Cell voltage 2
  //  38   2  0x11 0x10   Cell voltage 3
  //  40   2  0x11 0x10   Cell voltage 4
  //  42   2  0x11 0x10   Cell voltage 5
  //  44   2  0x11 0x10   Cell voltage 6
  //  46   2  0x11 0x10   Cell voltage 7
  //  48   2  0x11 0x10   Cell voltage 8
  //  50   2  0x11 0x10   Cell voltage 9
  //  52   2  0x11 0x10   Cell voltage 10
  //  54   2  0x11 0x10   Cell voltage 11
  //  56   2  0x11 0x10   Cell voltage 12
  //  58   2  0x11 0x10   Cell voltage 13
  //  60   2  0x11 0x10   Cell voltage 14            uint16_t
  for (uint8_t i = 0; i < cells; i++) {
    this->publish_state_(this->cells_[i].cell_voltage_sensor_, ant_get_16bit(i * 2 + 34) * 0.001f);
  }

  uint8_t offset = cells * 2;

  //  62   2  0x1C 0x00   Temperature sensor 1        int16_t
  //  64   2  0x1C 0x00   Temperature sensor 2        int16_t
  //  66   2  0xD8 0xFF   Temperature sensor 3        int16_t
  //  68   2  0x1C 0x00   Temperature sensor 4        int16_t
  //  70   2  0x1C 0x00   Mosfet temperature          int16_t
  //  72   2  0x1C 0x00   Balancer temperature        int16_t
  for (uint8_t i = 0; i < temperature_sensors; i++) {
    this->publish_state_(this->temperatures_[i].temperature_sensor_,
                         ((int16_t) ant_get_16bit(i * 2 + 34 + offset)) * 1.0f);
  }

  offset = offset + (temperature_sensors * 2);

  //  70   2  0x1C 0x00   Mosfet temperature          int16_t
  this->publish_state_(this->temperatures_[temperature_sensors].temperature_sensor_,
                       ((int16_t) ant_get_16bit(34 + offset)) * 1.0f);

  //  72   2  0x1C 0x00   Balancer temperature        int16_t
  this->publish_state_(this->temperatures_[temperature_sensors + 1].temperature_sensor_,
                       ((int16_t) ant_get_16bit(36 + offset)) * 1.0f);

  //  74   2  0x7E 0x16   Total voltage              uint16_t
  this->publish_state_(this->total_voltage_sensor_, ant_get_16bit(38 + offset) * 0.01f);

  //  76   2  0x00 0x00   Current                     int16_t
  this->publish_state_(this->current_sensor_, ((int16_t) ant_get_16bit(40 + offset)) * 0.1f);

  //  78   2  0x60 0x00   State of charge            uint16_t
  this->publish_state_(this->soc_sensor_, ((int16_t) ant_get_16bit(42 + offset)) * 1.0f);

  //  80   2  0x64 0x00   State of health            uint16_t
  ESP_LOGI(TAG, "  State of health: %.0f %%", ant_get_16bit(44 + offset) * 1.0f);

  //  82   1  0x01        Charge MOS status
  uint8_t raw_charge_mosfet_status = data[46 + offset];
  this->publish_state_(this->charge_mosfet_status_code_sensor_, (float) raw_charge_mosfet_status);
  if (raw_charge_mosfet_status < CHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, CHARGE_MOSFET_STATUS[raw_charge_mosfet_status]);
  } else {
    this->publish_state_(this->charge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->charging_switch_, (bool) (raw_charge_mosfet_status == 0x01));

  //  83   1  0x02        Discharge MOS status
  uint8_t raw_discharge_mosfet_status = data[47 + offset];
  this->publish_state_(this->discharge_mosfet_status_code_sensor_, (float) raw_discharge_mosfet_status);
  if (raw_discharge_mosfet_status < DISCHARGE_MOSFET_STATUS_SIZE) {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_,
                         DISCHARGE_MOSFET_STATUS[raw_discharge_mosfet_status]);
  } else {
    this->publish_state_(this->discharge_mosfet_status_text_sensor_, "Unknown");
  }
  this->publish_state_(this->discharging_switch_, (bool) (raw_discharge_mosfet_status == 0x01));

  //  84   1  0x00        Balancer status
  uint8_t raw_balancer_status = data[48 + offset];
  this->publish_state_(this->balancer_status_code_sensor_, (float) raw_balancer_status);
  this->publish_state_(this->balancer_switch_, (bool) (raw_balancer_status == 0x04));
  if (raw_balancer_status < BALANCER_STATUS_SIZE) {
    this->publish_state_(this->balancer_status_text_sensor_, BALANCER_STATUS[raw_balancer_status]);
  } else {
    this->publish_state_(this->balancer_status_text_sensor_, "Unknown");
  }

  //  85   1  0x00        Reserved
  //  86   4  0x80 0xC3 0xC9 0x01    Battery capacity            uint32_t
  this->publish_state_(this->total_battery_capacity_setting_sensor_, ant_get_32bit(50 + offset) * 0.000001f);

  //  90   4  0x4F 0x55 0xB3 0x01    Battery capacity remaining  uint32_t
  this->publish_state_(this->capacity_remaining_sensor_, ant_get_32bit(54 + offset) * 0.000001f);

  //  94   4  0x08 0x53 0x00 0x00    Total battery cycles capacity     uint32_t
  this->publish_state_(this->battery_cycle_capacity_sensor_, ant_get_32bit(58 + offset) * 0.001f);

  //  98   4  0x00 0x00 0x00 0x00    Power
  this->publish_state_(this->power_sensor_, ((int32_t) ant_get_32bit(62 + offset)) * 1.0f);

  // 102   4  0x6B 0x28 0x12 0x00    Total runtime
  this->publish_state_(this->total_runtime_sensor_, (float) ant_get_32bit(66 + offset));

  if (this->total_runtime_formatted_text_sensor_ != nullptr) {
    this->publish_state_(this->total_runtime_formatted_text_sensor_, format_total_runtime_(ant_get_32bit(66 + offset)));
  }

  // 106   4  0x00 0x00 0x00 0x00    Balanced cell bitmask
  ESP_LOGI(TAG, "  Balanced cell bitmask: %d", ant_get_32bit(70 + offset));

  // 110   2  0x11 0x10              Maximum cell voltage
  this->publish_state_(this->max_cell_voltage_sensor_, ant_get_16bit(74 + offset) * 0.001f);

  // 112   2  0x01 0x00              Maximum voltage cell
  this->publish_state_(this->max_voltage_cell_sensor_, ant_get_16bit(76 + offset) * 1.0f);

  // 114   2  0x11 0x10              Minimum cell voltage
  this->publish_state_(this->min_cell_voltage_sensor_, ant_get_16bit(78 + offset) * 0.001f);

  // 116   2  0x01 0x00              Minimum voltage cell
  this->publish_state_(this->min_voltage_cell_sensor_, ant_get_16bit(80 + offset) * 1.0f);

  // 118   2  0x00 0x00              Delta cell voltage
  this->publish_state_(this->delta_cell_voltage_sensor_, ant_get_16bit(82 + offset) * 0.001f);

  // 120   2  0x11 0x10              Average cell voltage
  this->publish_state_(this->average_cell_voltage_sensor_, ant_get_16bit(84 + offset) * 0.001f);

  // 122   2  0x02 0x00              Discharge MOSFET, voltage between D-S
  // 124   2  0x70 0x00              Drive voltage (discharge MOSFET)
  // 126   2  0x03 0x00              Drive voltage (charge MOSFET)
  // 128   2  0xAC 0x02              F40com
  // 130   2  0xF1 0xFA              Battery type (0xfaf1: Ternary Lithium, 0xfaf2: Lithium Iron Phosphate,
  //                                               0xfaf3: Lithium Titanate, 0xfaf4: Custom)
  // 132   4  0x7D 0x2E 0x00 0x00    Accumulated discharging capacity
  ESP_LOGI(TAG, "  Accumulated discharging capacity: %.2f Ah", ant_get_32bit(96 + offset) * 0.001f);

  // 136   4  0x94 0x77 0x00 0x00    Accumulated charging capacity
  ESP_LOGI(TAG, "  Accumulated charging capacity: %.2f Ah", ant_get_32bit(100 + offset) * 0.001f);

  // 140   4  0xDE 0x07 0x00 0x00    Accumulated discharging time
  ESP_LOGI(TAG, "  Accumulated discharging time: %s", this->format_total_runtime_(ant_get_32bit(104 + offset)).c_str());

  // 144   4  0x77 0x76 0x00 0x00    Accumulated charging time
  ESP_LOGI(TAG, "  Accumulated charging time: %s", this->format_total_runtime_(ant_get_32bit(108 + offset)).c_str());

  // 148   2  0x35 0xE2              CRC
  // 150   2  0xAA 0x55              End of frame
}

void AntBmsBle::on_device_info_data_(const std::vector<uint8_t> &data) {
  ESP_LOGI(TAG, "Device info frame (%d bytes):", data.size());
  ESP_LOGD(TAG, "  %s", format_hex_pretty(&data.front(), data.size()).c_str());

  // Status request
  // -> 0x7e 0xa1 0x02 0x6c 0x02 0x20 0x58 0xc4 0xaa 0x55
  //
  // Device info response
  //
  // Byte Len Payload     Description                      Unit  Precision
  //   0   2  0x7E 0xA1   Start of frame
  //   2   1  0x12        Function
  //   3   2  0x6C 0x02   Address
  //   5   1  0x20        Data length (32 bytes!)
  //   6  16  0x31 0x36 0x5A 0x4D 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00    Hardware version
  this->publish_state_(this->device_model_text_sensor_, std::string(data.begin() + 6, data.begin() + 6 + 16));

  //  22  16  0x31 0x36 0x5A 0x4D 0x55 0x42 0x30 0x30 0x2D 0x32 0x31 0x31 0x30 0x32 0x36 0x41    Software version
  this->publish_state_(this->software_version_text_sensor_, std::string(data.begin() + 22, data.begin() + 22 + 16));

  //  38   2  0x72 0x08   CRC
  //  40   1  0xFF        Reserved
  //  41   1  0x0B        Reserved
  //  42   1  0x00        Reserved
  //  43   1  0x00        Reserved
  //  44   2  0x41 0xF2   CRC unused
  //  46   2  0xAA 0x55   End of frame
}

void AntBmsBle::track_online_status_() {
  if (this->no_response_count_ < MAX_NO_RESPONSE_COUNT) {
    this->no_response_count_++;
  }
  if (this->no_response_count_ == MAX_NO_RESPONSE_COUNT) {
    this->publish_device_unavailable_();
    this->no_response_count_++;
  }
}

void AntBmsBle::reset_online_status_tracker_() {
  this->no_response_count_ = 0;
  this->publish_state_(this->online_status_binary_sensor_, true);
}

void AntBmsBle::publish_device_unavailable_() {
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

void AntBmsBle::dump_config() {  // NOLINT(google-readability-function-size,readability-function-size)
  ESP_LOGCONFIG(TAG, "AntBmsBle:");

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
  LOG_TEXT_SENSOR("", "Device Model", this->device_model_text_sensor_);
  LOG_TEXT_SENSOR("", "Software Version", this->software_version_text_sensor_);
}

void AntBmsBle::publish_state_(binary_sensor::BinarySensor *binary_sensor, const bool &state) {
  if (binary_sensor == nullptr)
    return;

  binary_sensor->publish_state(state);
}

void AntBmsBle::publish_state_(sensor::Sensor *sensor, float value) {
  if (sensor == nullptr)
    return;

  sensor->publish_state(value);
}

void AntBmsBle::publish_state_(switch_::Switch *obj, const bool &state) {
  if (obj == nullptr)
    return;

  obj->publish_state(state);
}

void AntBmsBle::publish_state_(text_sensor::TextSensor *text_sensor, const std::string &state) {
  if (text_sensor == nullptr)
    return;

  text_sensor->publish_state(state);
}

void AntBmsBle::write_register(uint16_t address, uint8_t value) {
  this->send_(ANT_COMMAND_WRITE_REGISTER, address, value, false);
}

bool AntBmsBle::authenticate_() {
  uint8_t frame[22];

  frame[0] = 0x7e;   // header
  frame[1] = 0xa1;   // header
  frame[2] = 0x23;   // control
  frame[3] = 0x6a;   // address
  frame[4] = 0x01;   // address
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

  auto crc = crc16_(frame + 1, 17);
  frame[18] = crc >> 0;  // CRC
  frame[19] = crc >> 8;  // CRC

  frame[20] = 0xaa;
  frame[21] = 0x55;

  ESP_LOGVV(TAG, "Send command: %s", format_hex_pretty(frame, sizeof(frame)).c_str());
  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->characteristic_handle_, sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP,
                                         ESP_GATT_AUTH_REQ_NONE);

  if (status)
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);

  return (status == 0);
}

bool AntBmsBle::authenticate_variable_(const uint8_t *data, uint8_t data_len) {
  std::vector<uint8_t> frame = {0x7E, 0xA1, 0x23, 0x6A, 0x01};
  frame.push_back(data_len);
  for (int i = 0; i < data_len; i++) {
    frame.push_back(data[i]);
  }
  auto crc = crc16_(frame.data() + 1, frame.size());
  frame.push_back(crc >> 0);
  frame.push_back(crc >> 8);
  frame.push_back(0xAA);
  frame.push_back(0x55);

  ESP_LOGVV(TAG, "Send command: %s", format_hex_pretty(&frame.front(), frame.size()).c_str());
  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->characteristic_handle_, frame.size(), &frame.front(),
                                         ESP_GATT_WRITE_TYPE_NO_RSP, ESP_GATT_AUTH_REQ_NONE);

  if (status)
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);

  return (status == 0);
}

bool AntBmsBle::send_(uint8_t function, uint16_t address, uint8_t value, bool authenticate = true) {
  if (authenticate) {
    this->authenticate_();
  }

  uint8_t frame[10];
  frame[0] = 0x7e;          // header
  frame[1] = 0xa1;          // header
  frame[2] = function;      // control
  frame[3] = address >> 0;  // address
  frame[4] = address >> 8;  // address
  frame[5] = value;         // value
  auto crc = crc16_(frame + 1, 5);
  frame[6] = crc >> 0;  // CRC
  frame[7] = crc >> 8;  // CRC
  frame[8] = 0xaa;      // footer
  frame[9] = 0x55;      // footer

  ESP_LOGVV(TAG, "Send command: %s", format_hex_pretty(frame, sizeof(frame)).c_str());
  auto status = esp_ble_gattc_write_char(this->parent_->get_gattc_if(), this->parent_->get_conn_id(),
                                         this->characteristic_handle_, sizeof(frame), frame, ESP_GATT_WRITE_TYPE_NO_RSP,
                                         ESP_GATT_AUTH_REQ_NONE);

  if (status)
    ESP_LOGW(TAG, "[%s] esp_ble_gattc_write_char failed, status=%d", this->parent_->address_str().c_str(), status);

  return (status == 0);
}

}  // namespace ant_bms_ble
}  // namespace esphome
