#include <gtest/gtest.h>
#include "common.h"
#include "frames_8s.h"
#include "frames_16s.h"

namespace esphome::ant_bms::testing {

// ── Total voltage ────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, TotalVoltage8S) {
  TestableAntBms bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_NEAR(total.state, 26.7f, 0.001f);
}

TEST(AntBmsStatusDataTest, TotalVoltage16S) {
  TestableAntBms bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_NEAR(total.state, 63.7f, 0.001f);
}

// ── Cell voltages ────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, CellVoltages8S) {
  TestableAntBms bms;
  sensor::Sensor cells[8];
  for (int i = 0; i < 8; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_NEAR(cells[0].state, 3.340f, 0.001f);  // cell 1
  EXPECT_NEAR(cells[1].state, 3.340f, 0.001f);  // cell 2
  EXPECT_NEAR(cells[2].state, 3.339f, 0.001f);  // cell 3
  EXPECT_NEAR(cells[3].state, 3.338f, 0.001f);  // cell 4 (min)
  EXPECT_NEAR(cells[4].state, 3.340f, 0.001f);  // cell 5
  EXPECT_NEAR(cells[7].state, 3.340f, 0.001f);  // cell 8
}

TEST(AntBmsStatusDataTest, CellVoltages16S) {
  TestableAntBms bms;
  sensor::Sensor cells[16];
  for (int i = 0; i < 16; i++)
    bms.set_cell_voltage_sensor(i, &cells[i]);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_NEAR(cells[0].state, 3.983f, 0.001f);   // cell 1
  EXPECT_NEAR(cells[6].state, 3.984f, 0.001f);   // cell 7 (max)
  EXPECT_NEAR(cells[11].state, 3.980f, 0.001f);  // cell 12 (min)
  EXPECT_NEAR(cells[15].state, 3.983f, 0.001f);  // cell 16
}

// ── Cell voltage statistics ──────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, CellVoltageStats8S) {
  TestableAntBms bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_NEAR(min_v.state, 3.338f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.340f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 4.0f);   // cell 4
  EXPECT_FLOAT_EQ(max_cell.state, 1.0f);   // cell 1
  EXPECT_NEAR(delta.state, 0.002f, 0.001f);
  EXPECT_NEAR(avg.state, 3.339f, 0.001f);
}

TEST(AntBmsStatusDataTest, CellVoltageStats16S) {
  TestableAntBms bms;
  sensor::Sensor min_v, max_v, min_cell, max_cell, delta, avg;
  bms.set_min_cell_voltage_sensor(&min_v);
  bms.set_max_cell_voltage_sensor(&max_v);
  bms.set_min_voltage_cell_sensor(&min_cell);
  bms.set_max_voltage_cell_sensor(&max_cell);
  bms.set_delta_cell_voltage_sensor(&delta);
  bms.set_average_cell_voltage_sensor(&avg);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_NEAR(min_v.state, 3.980f, 0.001f);
  EXPECT_NEAR(max_v.state, 3.984f, 0.001f);
  EXPECT_FLOAT_EQ(min_cell.state, 16.0f);  // cell 16
  EXPECT_FLOAT_EQ(max_cell.state, 7.0f);   // cell 7
  EXPECT_NEAR(delta.state, 0.004f, 0.001f);
  EXPECT_NEAR(avg.state, 3.982f, 0.001f);
}

// ── Current ──────────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, DischargingCurrent8S) {
  TestableAntBms bms;
  sensor::Sensor current;
  bms.set_current_sensor(&current);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_NEAR(current.state, -10.7f, 0.01f);  // discharging
}

TEST(AntBmsStatusDataTest, ZeroCurrent16S) {
  TestableAntBms bms;
  sensor::Sensor current;
  bms.set_current_sensor(&current);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_FLOAT_EQ(current.state, 0.0f);
}

// ── SOC ──────────────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, StateOfCharge) {
  TestableAntBms bms;
  sensor::Sensor soc;
  bms.set_soc_sensor(&soc);

  bms.on_ant_bms_data(STATUS_FRAME_8S);
  EXPECT_FLOAT_EQ(soc.state, 41.0f);

  bms.on_ant_bms_data(STATUS_FRAME_16S);
  EXPECT_FLOAT_EQ(soc.state, 84.0f);
}

// ── Capacity ─────────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, Capacity8S) {
  TestableAntBms bms;
  sensor::Sensor total_cap, cap_rem, cycle_cap;
  bms.set_total_battery_capacity_setting_sensor(&total_cap);
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_cycle_capacity_sensor(&cycle_cap);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(total_cap.state, 0.0f);
  EXPECT_NEAR(cap_rem.state, 48.487f, 0.001f);
  EXPECT_NEAR(cycle_cap.state, 19276.387f, 0.01f);
}

TEST(AntBmsStatusDataTest, Capacity16S) {
  TestableAntBms bms;
  sensor::Sensor total_cap, cap_rem, cycle_cap;
  bms.set_total_battery_capacity_setting_sensor(&total_cap);
  bms.set_capacity_remaining_sensor(&cap_rem);
  bms.set_battery_cycle_capacity_sensor(&cycle_cap);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_NEAR(total_cap.state, 234.0f, 0.001f);
  EXPECT_NEAR(cap_rem.state, 195.359f, 0.001f);
  EXPECT_NEAR(cycle_cap.state, 275.682f, 0.001f);
}

// ── Runtime ──────────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, TotalRuntime8S) {
  TestableAntBms bms;
  sensor::Sensor runtime;
  text_sensor::TextSensor runtime_fmt;
  bms.set_total_runtime_sensor(&runtime);
  bms.set_total_runtime_formatted_text_sensor(&runtime_fmt);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(runtime.state, 28405434.0f);
  EXPECT_EQ(runtime_fmt.state, "328d 18h");
}

TEST(AntBmsStatusDataTest, TotalRuntime16S) {
  TestableAntBms bms;
  sensor::Sensor runtime;
  text_sensor::TextSensor runtime_fmt;
  bms.set_total_runtime_sensor(&runtime);
  bms.set_total_runtime_formatted_text_sensor(&runtime_fmt);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_FLOAT_EQ(runtime.state, 1554278.0f);
  EXPECT_EQ(runtime_fmt.state, "17d 23h");
}

// ── Temperatures ─────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, Temperatures8S) {
  TestableAntBms bms;
  sensor::Sensor t[6];
  for (int i = 0; i < 6; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(t[0].state, 15.0f);
  EXPECT_FLOAT_EQ(t[1].state, 15.0f);
  EXPECT_FLOAT_EQ(t[2].state, 13.0f);
  EXPECT_FLOAT_EQ(t[3].state, 13.0f);
  EXPECT_FLOAT_EQ(t[4].state, 0.0f);
  EXPECT_FLOAT_EQ(t[5].state, 0.0f);
}

TEST(AntBmsStatusDataTest, Temperatures16S) {
  TestableAntBms bms;
  sensor::Sensor t[6];
  for (int i = 0; i < 6; i++)
    bms.set_temperature_sensor(i, &t[i]);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_FLOAT_EQ(t[0].state, 23.0f);
  EXPECT_FLOAT_EQ(t[1].state, 25.0f);
  EXPECT_FLOAT_EQ(t[2].state, 21.0f);
  EXPECT_FLOAT_EQ(t[3].state, 22.0f);
  EXPECT_FLOAT_EQ(t[4].state, -40.0f);
  EXPECT_FLOAT_EQ(t[5].state, -40.0f);
}

// ── MOSFET and balancer status ────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, MosfetStatusCodes8S) {
  TestableAntBms bms;
  sensor::Sensor charge_code, discharge_code, balancer_code;
  bms.set_charge_mosfet_status_code_sensor(&charge_code);
  bms.set_discharge_mosfet_status_code_sensor(&discharge_code);
  bms.set_balancer_status_code_sensor(&balancer_code);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_FLOAT_EQ(charge_code.state, 1.0f);    // On
  EXPECT_FLOAT_EQ(discharge_code.state, 1.0f); // On
  EXPECT_FLOAT_EQ(balancer_code.state, 0.0f);  // Off
}

TEST(AntBmsStatusDataTest, MosfetStatusText8S) {
  TestableAntBms bms;
  text_sensor::TextSensor charge_txt, discharge_txt, balancer_txt;
  bms.set_charge_mosfet_status_text_sensor(&charge_txt);
  bms.set_discharge_mosfet_status_text_sensor(&discharge_txt);
  bms.set_balancer_status_text_sensor(&balancer_txt);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_EQ(charge_txt.state, "On");
  EXPECT_EQ(discharge_txt.state, "On");
  EXPECT_EQ(balancer_txt.state, "Off");
}

// ── MOSFET switches ──────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, SwitchStates8S) {
  TestableAntBms bms;
  switch_::Switch charging, discharging, balancer;
  bms.set_charging_switch(&charging);
  bms.set_discharging_switch(&discharging);
  bms.set_balancer_switch(&balancer);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_TRUE(charging.state);
  EXPECT_TRUE(discharging.state);
  EXPECT_FALSE(balancer.state);  // balancer_status == 0x04 → on; 0x00 → off
}

// ── Power ────────────────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, PowerFromFrame8S) {
  TestableAntBms bms;
  sensor::Sensor power;
  bms.set_power_sensor(&power);
  bms.set_supports_new_commands(false);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  EXPECT_NEAR(power.state, -285.0f, 0.5f);
}

TEST(AntBmsStatusDataTest, PowerFromFrame16S) {
  TestableAntBms bms;
  sensor::Sensor power;
  bms.set_power_sensor(&power);
  bms.set_supports_new_commands(false);

  bms.on_ant_bms_data(STATUS_FRAME_16S);

  EXPECT_FLOAT_EQ(power.state, 0.0f);
}

TEST(AntBmsStatusDataTest, PowerComputedFromVoltageAndCurrent) {
  TestableAntBms bms;
  sensor::Sensor power;
  bms.set_power_sensor(&power);
  bms.set_supports_new_commands(true);

  bms.on_ant_bms_data(STATUS_FRAME_8S);

  // 26.7V × (-10.7A) = -285.69W
  EXPECT_NEAR(power.state, -285.69f, 0.1f);
}

// ── Battery strings ──────────────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, BatteryStrings) {
  TestableAntBms bms;
  sensor::Sensor strings;
  bms.set_battery_strings_sensor(&strings);

  bms.on_ant_bms_data(STATUS_FRAME_8S);
  EXPECT_FLOAT_EQ(strings.state, 8.0f);

  bms.on_ant_bms_data(STATUS_FRAME_16S);
  EXPECT_FLOAT_EQ(strings.state, 16.0f);
}

// ── Null sensors do not crash ────────────────────────────────────────────────

TEST(AntBmsStatusDataTest, NullSensorsDoNotCrash) {
  TestableAntBms bms;
  bms.on_ant_bms_data(STATUS_FRAME_8S);
  bms.on_ant_bms_data(STATUS_FRAME_16S);
}

// ── Wrong-length frame is ignored ────────────────────────────────────────────

TEST(AntBmsStatusDataTest, ShortFrameIgnored) {
  TestableAntBms bms;
  sensor::Sensor total;
  bms.set_total_voltage_sensor(&total);

  bms.on_ant_bms_data({0xAA, 0x55, 0xAA, 0xFF, 0x01, 0x0B});

  EXPECT_TRUE(std::isnan(total.state));
}

}  // namespace esphome::ant_bms::testing
