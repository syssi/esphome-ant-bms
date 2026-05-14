"""Schema structure tests for ant_bms ESPHome component modules."""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import components.ant_bms as hub  # noqa: E402
from components.ant_bms import (  # noqa: E402  # noqa: E402
    binary_sensor,
    button,
    sensor,
    switch,
    text_sensor,
)
import components.ant_bms_ble as hub_ble  # noqa: E402
from components.ant_bms_ble import (  # noqa: E402
    binary_sensor as ble_binary_sensor,
    button as ble_button,  # noqa: E402
    sensor as ble_sensor,
    switch as ble_switch,  # noqa: E402
    text_sensor as ble_text_sensor,
)
import components.ant_bms_old as hub_old  # noqa: E402
import components.ant_bms_old_ble as hub_old_ble  # noqa: E402


class TestHubConstants:
    def test_conf_ids_defined(self):
        assert hub.CONF_ANT_BMS_ID == "ant_bms_id"
        assert hub_ble.CONF_ANT_BMS_BLE_ID == "ant_bms_ble_id"
        assert hub_old.CONF_ANT_BMS_OLD_ID == "ant_bms_old_id"
        assert hub_old_ble.CONF_ANT_BMS_OLD_BLE_ID == "ant_bms_old_ble_id"


class TestSensorLists:
    def test_cells_count(self):
        assert len(sensor.CELLS) == 32
        assert len(ble_sensor.CELLS) == 32

    def test_cells_naming(self):
        assert sensor.CELLS[0] == "cell_voltage_1"
        assert sensor.CELLS[31] == "cell_voltage_32"
        for i, key in enumerate(sensor.CELLS, 1):
            assert key == f"cell_voltage_{i}"

    def test_temperatures_count(self):
        assert len(sensor.TEMPERATURES) == 6
        assert len(ble_sensor.TEMPERATURES) == 6

    def test_temperatures_naming(self):
        assert sensor.TEMPERATURES[0] == "temperature_1"
        assert sensor.TEMPERATURES[5] == "temperature_6"
        for i, key in enumerate(sensor.TEMPERATURES, 1):
            assert key == f"temperature_{i}"

    def test_sensor_defs_completeness(self):
        assert sensor.CONF_TOTAL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_AVERAGE_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_MIN_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_MAX_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_DELTA_CELL_VOLTAGE in sensor.SENSOR_DEFS
        assert sensor.CONF_SOC in sensor.SENSOR_DEFS
        assert sensor.CONF_STATE_OF_HEALTH in sensor.SENSOR_DEFS
        assert sensor.CONF_BATTERY_STATUS_CODE in sensor.SENSOR_DEFS
        assert sensor.CONF_TOTAL_DISCHARGING_CAPACITY in sensor.SENSOR_DEFS
        assert sensor.CONF_TOTAL_CHARGING_CAPACITY in sensor.SENSOR_DEFS
        assert sensor.CONF_TOTAL_DISCHARGING_TIME in sensor.SENSOR_DEFS
        assert sensor.CONF_TOTAL_CHARGING_TIME in sensor.SENSOR_DEFS
        assert sensor.CONF_BALANCED_CELL_BITMASK in sensor.SENSOR_DEFS
        assert len(sensor.SENSOR_DEFS) == 25

    def test_no_cell_keys_in_sensor_defs(self):
        for key in sensor.SENSOR_DEFS:
            assert key not in sensor.CELLS
            assert key not in sensor.TEMPERATURES


class TestBinarySensorConstants:
    def test_binary_sensor_defs_dict(self):
        assert binary_sensor.CONF_ONLINE_STATUS in binary_sensor.BINARY_SENSOR_DEFS
        assert len(binary_sensor.BINARY_SENSOR_DEFS) == 1

    def test_ble_binary_sensor_defs_dict(self):
        assert (
            ble_binary_sensor.CONF_ONLINE_STATUS in ble_binary_sensor.BINARY_SENSOR_DEFS
        )
        assert len(ble_binary_sensor.BINARY_SENSOR_DEFS) == 1


class TestTextSensorConstants:
    def test_text_sensors_list(self):
        assert text_sensor.CONF_CHARGE_MOSFET_STATUS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_DISCHARGE_MOSFET_STATUS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_BALANCER_STATUS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_TOTAL_RUNTIME_FORMATTED in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_BATTERY_STATUS in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_DEVICE_MODEL in text_sensor.TEXT_SENSORS
        assert text_sensor.CONF_SOFTWARE_VERSION in text_sensor.TEXT_SENSORS
        assert (
            text_sensor.CONF_TOTAL_DISCHARGING_TIME_FORMATTED
            in text_sensor.TEXT_SENSORS
        )
        assert (
            text_sensor.CONF_TOTAL_CHARGING_TIME_FORMATTED in text_sensor.TEXT_SENSORS
        )
        assert len(text_sensor.TEXT_SENSORS) == 9

    def test_ble_text_sensors_list(self):
        assert ble_text_sensor.CONF_CHARGE_MOSFET_STATUS in ble_text_sensor.TEXT_SENSORS
        assert ble_text_sensor.CONF_DEVICE_MODEL in ble_text_sensor.TEXT_SENSORS
        assert ble_text_sensor.CONF_SOFTWARE_VERSION in ble_text_sensor.TEXT_SENSORS
        assert len(ble_text_sensor.TEXT_SENSORS) == 9


class TestSwitchConstants:
    def test_switches_dict(self):
        assert switch.CONF_DISCHARGING in switch.SWITCHES
        assert switch.CONF_CHARGING in switch.SWITCHES
        assert switch.CONF_BALANCER in switch.SWITCHES
        assert len(switch.SWITCHES) == 4

    def test_ble_switches_dict(self):
        assert ble_switch.CONF_DISCHARGING in ble_switch.SWITCHES
        assert ble_switch.CONF_CHARGING in ble_switch.SWITCHES
        assert ble_switch.CONF_BALANCER in ble_switch.SWITCHES
        assert len(ble_switch.SWITCHES) == 4


class TestButtonConstants:
    def test_buttons_dict(self):
        from esphome.const import CONF_FACTORY_RESET, CONF_RESTART

        assert button.CONF_SHUTDOWN in button.BUTTONS
        assert CONF_RESTART in button.BUTTONS
        assert CONF_FACTORY_RESET in button.BUTTONS
        assert button.CONF_CLEAR_SYSTEM_LOG in button.BUTTONS
        assert button.CONF_CURRENT_ZERO in button.BUTTONS
        assert button.CONF_RESET_HARDWARE in button.BUTTONS
        assert button.CONF_BLUETOOTH_INITIALIZATION in button.BUTTONS
        assert button.CONF_SAVE_CUSTOMER_DATA in button.BUTTONS
        assert button.CONF_BLUETOOTH_ON in button.BUTTONS
        assert button.CONF_BLUETOOTH_OFF in button.BUTTONS
        assert len(button.BUTTONS) == 16

    def test_button_addresses_are_unique(self):
        addresses = list(button.BUTTONS.values())
        assert len(addresses) == len(set(addresses))

    def test_ble_buttons_dict(self):
        from esphome.const import CONF_FACTORY_RESET, CONF_RESTART

        assert ble_button.CONF_SHUTDOWN in ble_button.BUTTONS
        assert CONF_RESTART in ble_button.BUTTONS
        assert CONF_FACTORY_RESET in ble_button.BUTTONS
        assert ble_button.CONF_BLUETOOTH_ON in ble_button.BUTTONS
        assert ble_button.CONF_BLUETOOTH_OFF in ble_button.BUTTONS
        assert len(ble_button.BUTTONS) == 16
