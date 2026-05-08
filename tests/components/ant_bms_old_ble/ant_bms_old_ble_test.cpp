#include <gtest/gtest.h>
#include "common.h"
#include "frames_14s.h"

namespace esphome::ant_bms_old_ble::testing {

// ── Total voltage ─────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, TotalVoltage) {
  TestableAntBmsOldBle bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(total.state, 48.8f, 0.01f);
}

// ── Battery strings ───────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, BatteryStrings) {
  TestableAntBmsOldBle bms;
  sensor::Sensor strings;
  bms.set_battery_strings_sensor(&strings);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_FLOAT_EQ(strings.state, 14.0f);
}

// ── Cell voltages ─────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, CellVoltages) {
  TestableAntBmsOldBle bms;
  sensor::Sensor cells[14];
  for (int i = 0; i < 14; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(cells[0].state, 3.498f, 0.001f);   // cell 1
  EXPECT_NEAR(cells[1].state, 3.484f, 0.001f);   // cell 2
  EXPECT_NEAR(cells[6].state, 3.508f, 0.001f);   // cell 7
  EXPECT_NEAR(cells[8].state, 3.509f, 0.001f);   // cell 9 (max)
  EXPECT_NEAR(cells[13].state, 3.468f, 0.001f);  // cell 14 (min)
}

// ── Cell voltage statistics ───────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, CellVoltageStats) {
  TestableAntBmsOldBle bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(min_v.state, 3.468f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.509f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 14.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 9.0f);
  EXPECT_NEAR(delta.state, 0.041f, 0.001f);
  EXPECT_NEAR(avg.state, 3.487f, 0.001f);
}

// ── Current and SOC ───────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, CurrentAndSOC) {
  TestableAntBmsOldBle bms;
  sensor::Sensor current, soc;
  bms.set_current_sensor(&current);
  bms.set_soc_sensor(&soc);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(current.state, 8.0f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 41.0f);
}

// ── Capacity ──────────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, Capacity) {
  TestableAntBmsOldBle bms;
  sensor::Sensor total_cap, cap_rem, cycle_cap;
  bms.set_total_battery_capacity_setting_sensor(&total_cap);
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_cycle_capacity_sensor(&cycle_cap);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(total_cap.state, 170.0f, 0.1f);
  EXPECT_NEAR(cap_rem.state, 68.770f, 0.01f);
  EXPECT_NEAR(cycle_cap.state, 11109.391f, 0.1f);
}

// ── Power ─────────────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, Power) {
  TestableAntBmsOldBle bms;
  sensor::Sensor power;
  bms.set_power_sensor(&power);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_NEAR(power.state, 390.0f, 1.0f);
}

// ── Runtime ───────────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, TotalRuntime) {
  TestableAntBmsOldBle bms;
  sensor::Sensor runtime;
  text_sensor::TextSensor runtime_fmt;
  bms.set_total_runtime_sensor(&runtime);
  bms.set_total_runtime_formatted_text_sensor(&runtime_fmt);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_FLOAT_EQ(runtime.state, 16386097.0f);
  EXPECT_EQ(runtime_fmt.state, "189d 15h");
}

// ── Temperatures ──────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, Temperatures) {
  TestableAntBmsOldBle bms;
  sensor::Sensor t[6];
  for (int i = 0; i < 6; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_FLOAT_EQ(t[0].state, 22.0f);
  EXPECT_FLOAT_EQ(t[1].state, 21.0f);
  EXPECT_FLOAT_EQ(t[2].state, 21.0f);
  EXPECT_FLOAT_EQ(t[3].state, 21.0f);
  EXPECT_FLOAT_EQ(t[4].state, 21.0f);
  EXPECT_FLOAT_EQ(t[5].state, 21.0f);
}

// ── MOSFET status ─────────────────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, MosfetStatus) {
  TestableAntBmsOldBle bms;
  text_sensor::TextSensor charge_txt, discharge_txt, balancer_txt;
  sensor::Sensor charge_code, discharge_code, balancer_code;
  TestableSwitch charging, discharging;
  bms.set_charge_mosfet_status_text_sensor(&charge_txt);
  bms.set_discharge_mosfet_status_text_sensor(&discharge_txt);
  bms.set_balancer_status_text_sensor(&balancer_txt);
  bms.set_charge_mosfet_status_code_sensor(&charge_code);
  bms.set_discharge_mosfet_status_code_sensor(&discharge_code);
  bms.set_balancer_status_code_sensor(&balancer_code);
  bms.set_charging_switch(&charging);
  bms.set_discharging_switch(&discharging);

  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());

  EXPECT_EQ(charge_txt.state, "On");
  EXPECT_EQ(discharge_txt.state, "On");
  EXPECT_EQ(balancer_txt.state, "Off");
  EXPECT_FLOAT_EQ(charge_code.state, 1.0f);
  EXPECT_FLOAT_EQ(discharge_code.state, 1.0f);
  EXPECT_FLOAT_EQ(balancer_code.state, 0.0f);
  EXPECT_TRUE(charging.state);
  EXPECT_TRUE(discharging.state);
}

// ── Null sensors do not crash ─────────────────────────────────────────────────

TEST(AntBmsOldBleStatusDataTest, NullSensorsDoNotCrash) {
  TestableAntBmsOldBle bms;
  bms.assemble(STATUS_FRAME_14S.data(), STATUS_FRAME_14S.size());
}

}  // namespace esphome::ant_bms_old_ble::testing
