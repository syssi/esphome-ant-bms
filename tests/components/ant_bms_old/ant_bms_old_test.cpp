#include <gtest/gtest.h>
#include "common.h"
#include "frames_8s.h"

namespace esphome::ant_bms_old::testing {

// ── Total voltage ────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, TotalVoltage) {
  TestableAntBmsOld bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_NEAR(total.state, 26.7f, 0.001f);
}

// ── Cell voltages ────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, CellVoltages) {
  TestableAntBmsOld bms;
  sensor::Sensor cells[8];
  for (int i = 0; i < 8; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_NEAR(cells[0].state, 3.340f, 0.001f);  // cell 1
  EXPECT_NEAR(cells[1].state, 3.340f, 0.001f);  // cell 2
  EXPECT_NEAR(cells[2].state, 3.339f, 0.001f);  // cell 3
  EXPECT_NEAR(cells[3].state, 3.338f, 0.001f);  // cell 4 (min)
  EXPECT_NEAR(cells[7].state, 3.340f, 0.001f);  // cell 8
}

// ── Cell voltage statistics ──────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, CellVoltageStats) {
  TestableAntBmsOld bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_NEAR(min_v.state, 3.338f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.340f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 4.0f);
  EXPECT_FLOAT_EQ(max_cell.state, 1.0f);
  EXPECT_NEAR(delta.state, 0.002f, 0.001f);
  EXPECT_NEAR(avg.state, 3.339f, 0.001f);
}

// ── Current and SOC ──────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, CurrentAndSOC) {
  TestableAntBmsOld bms;
  sensor::Sensor current, soc;
  bms.set_current_sensor(&current);
  bms.set_soc_sensor(&soc);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_NEAR(current.state, -10.7f, 0.01f);
  EXPECT_FLOAT_EQ(soc.state, 41.0f);
}

// ── Capacity ─────────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, Capacity) {
  TestableAntBmsOld bms;
  sensor::Sensor total_cap, cap_rem, cycle_cap;
  bms.set_total_battery_capacity_setting_sensor(&total_cap);
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_cycle_capacity_sensor(&cycle_cap);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(total_cap.state, 0.0f);
  EXPECT_NEAR(cap_rem.state, 48.487f, 0.001f);
  EXPECT_NEAR(cycle_cap.state, 19276.387f, 0.01f);
}

// ── Runtime ──────────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, TotalRuntime) {
  TestableAntBmsOld bms;
  sensor::Sensor runtime;
  text_sensor::TextSensor runtime_fmt;
  bms.set_total_runtime_sensor(&runtime);
  bms.set_total_runtime_formatted_text_sensor(&runtime_fmt);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(runtime.state, 28405434.0f);
  EXPECT_EQ(runtime_fmt.state, "328d 18h");
}

// ── Temperatures ─────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, Temperatures) {
  TestableAntBmsOld bms;
  sensor::Sensor t[6];
  for (int i = 0; i < 6; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(t[0].state, 15.0f);
  EXPECT_FLOAT_EQ(t[1].state, 15.0f);
  EXPECT_FLOAT_EQ(t[2].state, 13.0f);
  EXPECT_FLOAT_EQ(t[3].state, 13.0f);
  EXPECT_FLOAT_EQ(t[4].state, 0.0f);
  EXPECT_FLOAT_EQ(t[5].state, 0.0f);
}

// ── MOSFET status ────────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, MosfetStatus) {
  TestableAntBmsOld bms;
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

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

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

// ── Battery strings ──────────────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, BatteryStrings) {
  TestableAntBmsOld bms;
  sensor::Sensor strings;
  bms.set_battery_strings_sensor(&strings);

  bms.on_ant_bms_old_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(strings.state, 8.0f);
}

// ── Null sensors do not crash ────────────────────────────────────────────────

TEST(AntBmsOldStatusDataTest, NullSensorsDoNotCrash) {
  TestableAntBmsOld bms;
  bms.on_ant_bms_old_data(STATUS_FRAME_8S);
}

}  // namespace esphome::ant_bms_old::testing
