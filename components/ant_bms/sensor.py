import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_CURRENT,
    CONF_POWER,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ICON_EMPTY,
    ICON_TIMELAPSE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_EMPTY,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
)

from . import ANT_BMS_COMPONENT_SCHEMA, CONF_ANT_BMS_ID

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_BATTERY_STRINGS = "battery_strings"
CONF_SOC = "soc"
CONF_TOTAL_BATTERY_CAPACITY_SETTING = "total_battery_capacity_setting"
CONF_CAPACITY_REMAINING = "capacity_remaining"
CONF_BATTERY_CYCLE_CAPACITY = "battery_cycle_capacity"
CONF_TOTAL_VOLTAGE = "total_voltage"
CONF_TOTAL_RUNTIME = "total_runtime"

CONF_AVERAGE_CELL_VOLTAGE = "average_cell_voltage"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_VOLTAGE_CELL = "min_voltage_cell"
CONF_MAX_VOLTAGE_CELL = "max_voltage_cell"
CONF_DELTA_CELL_VOLTAGE = "delta_cell_voltage"

CONF_CHARGE_MOSFET_STATUS_CODE = "charge_mosfet_status_code"
CONF_DISCHARGE_MOSFET_STATUS_CODE = "discharge_mosfet_status_code"
CONF_BALANCER_STATUS_CODE = "balancer_status_code"
CONF_STATE_OF_HEALTH = "state_of_health"
CONF_BATTERY_STATUS_CODE = "battery_status_code"
CONF_TOTAL_DISCHARGING_CAPACITY = "total_discharging_capacity"
CONF_TOTAL_CHARGING_CAPACITY = "total_charging_capacity"
CONF_TOTAL_DISCHARGING_TIME = "total_discharging_time"
CONF_TOTAL_CHARGING_TIME = "total_charging_time"
CONF_BALANCED_CELL_BITMASK = "balanced_cell_bitmask"

ICON_CURRENT_DC = "mdi:current-dc"
ICON_BATTERY_STRINGS = "mdi:car-battery"
ICON_CAPACITY_REMAINING = "mdi:battery-50"
ICON_TOTAL_BATTERY_CAPACITY_SETTING = "mdi:battery-50"
ICON_BATTERY_CYCLE_CAPACITY = "mdi:battery-50"
ICON_CHARGE_MOSFET_STATUS_CODE = "mdi:heart-pulse"
ICON_DISCHARGE_MOSFET_STATUS_CODE = "mdi:heart-pulse"
ICON_BALANCER_STATUS_CODE = "mdi:heart-pulse"

ICON_STATE_OF_HEALTH = "mdi:battery-heart"
ICON_BATTERY_STATUS_CODE = "mdi:heart-pulse"
ICON_TOTAL_DISCHARGING_CAPACITY = "mdi:battery-minus"
ICON_TOTAL_CHARGING_CAPACITY = "mdi:battery-plus"
ICON_TOTAL_DISCHARGING_TIME = "mdi:timer-outline"
ICON_TOTAL_CHARGING_TIME = "mdi:timer-outline"
ICON_BALANCED_CELL_BITMASK = "mdi:battery-sync"

ICON_MIN_CELL_VOLTAGE = "mdi:battery-minus-outline"
ICON_MAX_CELL_VOLTAGE = "mdi:battery-plus-outline"
ICON_MIN_VOLTAGE_CELL = "mdi:battery-minus-outline"
ICON_MAX_VOLTAGE_CELL = "mdi:battery-plus-outline"

UNIT_SECONDS = "s"
UNIT_AMPERE_HOURS = "Ah"

CELLS = [f"cell_voltage_{i}" for i in range(1, 33)]
TEMPERATURES = [f"temperature_{i}" for i in range(1, 7)]

# key: sensor_schema kwargs
SENSOR_DEFS = {
    CONF_BATTERY_STRINGS: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BATTERY_STRINGS,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CURRENT: {
        "unit_of_measurement": UNIT_AMPERE,
        "icon": ICON_CURRENT_DC,
        "accuracy_decimals": 1,
        "device_class": DEVICE_CLASS_CURRENT,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_SOC: {
        "unit_of_measurement": UNIT_PERCENT,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_BATTERY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_BATTERY_CAPACITY_SETTING: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_TOTAL_BATTERY_CAPACITY_SETTING,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CAPACITY_REMAINING: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_CAPACITY_REMAINING,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BATTERY_CYCLE_CAPACITY: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_BATTERY_CYCLE_CAPACITY,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_RUNTIME: {
        "unit_of_measurement": UNIT_SECONDS,
        "icon": ICON_TIMELAPSE,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_AVERAGE_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_POWER: {
        "unit_of_measurement": UNIT_WATT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 2,
        "device_class": DEVICE_CLASS_POWER,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_MIN_CELL_VOLTAGE,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_MAX_CELL_VOLTAGE,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MIN_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_MIN_VOLTAGE_CELL,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_MAX_VOLTAGE_CELL: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_MAX_VOLTAGE_CELL,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DELTA_CELL_VOLTAGE: {
        "unit_of_measurement": UNIT_VOLT,
        "icon": ICON_EMPTY,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_VOLTAGE,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_CHARGE_MOSFET_STATUS_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_CHARGE_MOSFET_STATUS_CODE,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_DISCHARGE_MOSFET_STATUS_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_DISCHARGE_MOSFET_STATUS_CODE,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BALANCER_STATUS_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BALANCER_STATUS_CODE,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_STATE_OF_HEALTH: {
        "unit_of_measurement": UNIT_PERCENT,
        "icon": ICON_STATE_OF_HEALTH,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_BATTERY_STATUS_CODE: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BATTERY_STATUS_CODE,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
    CONF_TOTAL_DISCHARGING_CAPACITY: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_TOTAL_DISCHARGING_CAPACITY,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_TOTAL_CHARGING_CAPACITY: {
        "unit_of_measurement": UNIT_AMPERE_HOURS,
        "icon": ICON_TOTAL_CHARGING_CAPACITY,
        "accuracy_decimals": 3,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_TOTAL_DISCHARGING_TIME: {
        "unit_of_measurement": UNIT_SECONDS,
        "icon": ICON_TOTAL_DISCHARGING_TIME,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_TOTAL_CHARGING_TIME: {
        "unit_of_measurement": UNIT_SECONDS,
        "icon": ICON_TOTAL_CHARGING_TIME,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_TOTAL_INCREASING,
    },
    CONF_BALANCED_CELL_BITMASK: {
        "unit_of_measurement": UNIT_EMPTY,
        "icon": ICON_BALANCED_CELL_BITMASK,
        "accuracy_decimals": 0,
        "device_class": DEVICE_CLASS_EMPTY,
        "state_class": STATE_CLASS_MEASUREMENT,
    },
}

_CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    icon=ICON_EMPTY,
    accuracy_decimals=3,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)
_TEMPERATURE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_CELSIUS,
    icon=ICON_EMPTY,
    accuracy_decimals=0,
    device_class=DEVICE_CLASS_TEMPERATURE,
    state_class=STATE_CLASS_MEASUREMENT,
)

CONFIG_SCHEMA = (
    ANT_BMS_COMPONENT_SCHEMA.extend(
        {
            cv.Optional(key): sensor.sensor_schema(**kwargs)
            for key, kwargs in SENSOR_DEFS.items()
        }
    )
    .extend({cv.Optional(key): _CELL_VOLTAGE_SCHEMA for key in CELLS})
    .extend({cv.Optional(key): _TEMPERATURE_SCHEMA for key in TEMPERATURES})
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_ID])
    for i, key in enumerate(TEMPERATURES):
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(hub.set_temperature_sensor(i, sens))
    for i, key in enumerate(CELLS):
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(hub.set_cell_voltage_sensor(i, sens))
    for key in SENSOR_DEFS:
        if key in config:
            conf = config[key]
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
