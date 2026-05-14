#include <gtest/gtest.h>
#include "common.h"
#include "frames_16s_status.h"

namespace esphome::ant_bms_ble::testing {

// ── Total voltage ─────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, TotalVoltage) {
  TestableAntBmsBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(total.state, 52.84f, 0.01f);
}

// ── Battery strings ───────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, BatteryStrings) {
  TestableAntBmsBle bms;
  sensor::Sensor strings;
  bms.set_battery_strings_sensor(&strings);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(strings.state, 16.0f);
}

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, CellVoltages) {
  TestableAntBmsBle bms;
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(cells[0].state, 3.300f, 0.001f);   // cell 1 (min)
  EXPECT_NEAR(cells[1].state, 3.300f, 0.001f);   // cell 2
  EXPECT_NEAR(cells[2].state, 3.301f, 0.001f);   // cell 3
  EXPECT_NEAR(cells[3].state, 3.301f, 0.001f);   // cell 4
  EXPECT_NEAR(cells[4].state, 3.304f, 0.001f);   // cell 5
  EXPECT_NEAR(cells[7].state, 3.302f, 0.001f);   // cell 8
  EXPECT_NEAR(cells[15].state, 3.305f, 0.001f);  // cell 16 (max)
}

// ── Cell voltage statistics ───────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, CellVoltageStats) {
  TestableAntBmsBle bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(min_v.state, 3.300f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.305f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 1.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 16.0f);
  EXPECT_NEAR(delta.state, 0.005f, 0.001f);
  EXPECT_NEAR(avg.state, 3.302f, 0.001f);
}

// ── Current and SOC ───────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, CurrentAndSOC) {
  TestableAntBmsBle bms;
  sensor::Sensor current, soc;
  bms.set_current_sensor(&current);
  bms.set_soc_sensor(&soc);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(current.state, 0.3f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 91.0f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, Capacity) {
  TestableAntBmsBle bms;
  sensor::Sensor total_cap, cap_rem, cycle_cap;
  bms.set_total_battery_capacity_setting_sensor(&total_cap);
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_cycle_capacity_sensor(&cycle_cap);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(total_cap.state, 280.0f, 0.1f);
  EXPECT_NEAR(cap_rem.state, 252.602f, 0.01f);
  EXPECT_NEAR(cycle_cap.state, 4862.65f, 0.1f);
}

// ── Power ─────────────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, Power) {
  TestableAntBmsBle bms;
  sensor::Sensor power;
  bms.set_power_sensor(&power);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(power.state, 15.0f, 1.0f);
}

// ── Runtime ───────────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, TotalRuntime) {
  TestableAntBmsBle bms;
  sensor::Sensor runtime;
  text_sensor::TextSensor runtime_fmt;
  bms.set_total_runtime_sensor(&runtime);
  bms.set_total_runtime_formatted_text_sensor(&runtime_fmt);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(runtime.state, 36591632.0f);
  EXPECT_EQ(runtime_fmt.state, "1y 58d 12h");
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, Temperatures) {
  TestableAntBmsBle bms;
  sensor::Sensor t[4];
  for (int i = 0; i < 4; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(t[0].state, 1.0f);
  EXPECT_FLOAT_EQ(t[1].state, 2.0f);
  EXPECT_FLOAT_EQ(t[2].state, 2.0f);  // mosfet temp
  EXPECT_FLOAT_EQ(t[3].state, 7.0f);  // balancer temp
}

// ── MOSFET status ─────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, MosfetStatus) {
  TestableAntBmsBle bms;
  text_sensor::TextSensor charge_txt, discharge_txt, balancer_txt;
  sensor::Sensor charge_code, discharge_code, balancer_code;
  TestableSwitch charging, discharging, balancer;
  bms.set_charge_mosfet_status_text_sensor(&charge_txt);
  bms.set_discharge_mosfet_status_text_sensor(&discharge_txt);
  bms.set_balancer_status_text_sensor(&balancer_txt);
  bms.set_charge_mosfet_status_code_sensor(&charge_code);
  bms.set_discharge_mosfet_status_code_sensor(&discharge_code);
  bms.set_balancer_status_code_sensor(&balancer_code);
  bms.set_charging_switch(&charging);
  bms.set_discharging_switch(&discharging);
  bms.set_balancer_switch(&balancer);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_EQ(charge_txt.state, "On");
  EXPECT_EQ(discharge_txt.state, "On");
  EXPECT_EQ(balancer_txt.state, "Off");
  EXPECT_FLOAT_EQ(charge_code.state, 1.0f);
  EXPECT_FLOAT_EQ(discharge_code.state, 1.0f);
  EXPECT_FLOAT_EQ(balancer_code.state, 0.0f);
  EXPECT_TRUE(charging.state);
  EXPECT_TRUE(discharging.state);
  EXPECT_FALSE(balancer.state);
}

// ── Battery status ────────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, BatteryStatus) {
  TestableAntBmsBle bms;
  sensor::Sensor code;
  text_sensor::TextSensor txt;
  bms.set_battery_status_code_sensor(&code);
  bms.set_battery_status_text_sensor(&txt);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(code.state, 1.0f);
  EXPECT_EQ(txt.state, "Idle");
}

// ── State of health ───────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, StateOfHealth) {
  TestableAntBmsBle bms;
  sensor::Sensor soh;
  bms.set_state_of_health_sensor(&soh);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(soh.state, 100.0f);
}

// ── Balanced cell bitmask ─────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, BalancedCellBitmask) {
  TestableAntBmsBle bms;
  sensor::Sensor bitmask;
  bms.set_balanced_cell_bitmask_sensor(&bitmask);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(bitmask.state, 0.0f);
}

// ── Total capacity ──────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, AccumulatedCapacity) {
  TestableAntBmsBle bms;
  sensor::Sensor dis_cap, chg_cap;
  bms.set_total_discharging_capacity_sensor(&dis_cap);
  bms.set_total_charging_capacity_sensor(&chg_cap);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_NEAR(dis_cap.state, 3902.649f, 0.01f);
  EXPECT_NEAR(chg_cap.state, 5822.651f, 0.01f);
}

// ── Total time ──────────────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, AccumulatedTime) {
  TestableAntBmsBle bms;
  sensor::Sensor dis_time, chg_time;
  text_sensor::TextSensor dis_fmt, chg_fmt;
  bms.set_total_discharging_time_sensor(&dis_time);
  bms.set_total_charging_time_sensor(&chg_time);
  bms.set_total_discharging_time_formatted_text_sensor(&dis_fmt);
  bms.set_total_charging_time_formatted_text_sensor(&chg_fmt);

  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());

  EXPECT_FLOAT_EQ(dis_time.state, 4402650.0f);
  EXPECT_FLOAT_EQ(chg_time.state, 4830952.0f);
  EXPECT_EQ(dis_fmt.state, "50d 22h");
  EXPECT_EQ(chg_fmt.state, "55d 21h");
}

// ── Device info ───────────────────────────────────────────────────────────────

TEST(AntBmsBleDeviceInfoTest, DeviceModel) {
  TestableAntBmsBle bms;
  text_sensor::TextSensor model, version;
  bms.set_device_model_text_sensor(&model);
  bms.set_software_version_text_sensor(&version);

  bms.assemble(DEVICE_INFO_FRAME.data(), DEVICE_INFO_FRAME.size());

  EXPECT_EQ(model.state, "16ZM");
  EXPECT_EQ(version.state, "16ZMUB00-211026A");
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(AntBmsBleStatusDataTest, NullSensorsDoNotCrash) {
  TestableAntBmsBle bms;
  bms.assemble(STATUS_FRAME_16S.data(), STATUS_FRAME_16S.size());
  bms.assemble(DEVICE_INFO_FRAME.data(), DEVICE_INFO_FRAME.size());
}

// -- Frame builder ------------------------------------------------------------

TEST(AntBmsBleFrameBuilderTest, CurrentZero) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0008, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x08, 0x00, 0x00, 0x08, 0xe7, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, Restart) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0009, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x09, 0x00, 0x00, 0x59, 0x27, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, Shutdown) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x000b, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x0b, 0x00, 0x00, 0xf8, 0xe7, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, BluetoothOff) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x001c, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x1c, 0x00, 0x00, 0x48, 0xe3, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, BluetoothOn) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x001d, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x1d, 0x00, 0x00, 0x19, 0x23, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ResetHardware) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x002a, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x2a, 0x00, 0x00, 0xa8, 0xed, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, SaveCustomerData) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x002c, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x2c, 0x00, 0x00, 0x48, 0xec, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, FactoryReset) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x000c, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x0c, 0x00, 0x00, 0x49, 0x26, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearSystemLog) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x000f, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x0f, 0x00, 0x00, 0xb9, 0x26, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, BluetoothInitialization) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0010, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x10, 0x00, 0x00, 0x88, 0xe0, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearDischargeCycleAh) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0020, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x20, 0x00, 0x00, 0x88, 0xef, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearChargeCycleAh) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0021, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x21, 0x00, 0x00, 0xd9, 0x2f, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearDischargeTime) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0022, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x22, 0x00, 0x00, 0x29, 0x2f, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearChargeTime) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0023, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x23, 0x00, 0x00, 0x78, 0xef, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearRunningTime) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0024, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x24, 0x00, 0x00, 0xc9, 0x2e, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

TEST(AntBmsBleFrameBuilderTest, ClearProtectTime) {
  auto frame = AntBmsBle::build_frame_(0x51, 0x0025, 0x00);
  std::vector<uint8_t> expected = {0x7e, 0xa1, 0x51, 0x25, 0x00, 0x00, 0x98, 0xee, 0xaa, 0x55};
  EXPECT_EQ(std::vector<uint8_t>(frame.begin(), frame.end()), expected);
}

}  // namespace esphome::ant_bms_ble::testing
