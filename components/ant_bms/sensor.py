import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    ICON_EMPTY,
    UNIT_PERCENT,
    UNIT_VOLT,
)

from . import CONF_ANT_BMS_ID, AntBms

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_CAPACITY_REMAINING = "capacity_remaining"
CONF_SOC = "soc"
CONF_TOTAL_VOLTAGE = "total_voltage"

ICON_CAPACITY_REMAINING = "mdi:battery-50"
ICON_SOC = "mdi:battery-50"

UNIT_AMPERE_HOURS = "Ah"

SENSORS = [
    CONF_CAPACITY_REMAINING,
    CONF_SOC,
    CONF_TOTAL_VOLTAGE,
]

# pylint: disable=too-many-function-args
CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
        cv.Optional(CONF_CAPACITY_REMAINING): sensor.sensor_schema(
            UNIT_AMPERE_HOURS,
            ICON_CAPACITY_REMAINING,
            2,
            DEVICE_CLASS_EMPTY,
            STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_SOC): sensor.sensor_schema(
            UNIT_PERCENT,
            ICON_SOC,
            0,
            DEVICE_CLASS_EMPTY,
            STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_TOTAL_VOLTAGE): sensor.sensor_schema(
            UNIT_VOLT, ICON_EMPTY, 2, DEVICE_CLASS_VOLTAGE, STATE_CLASS_MEASUREMENT
        ),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_ANT_BMS_ID])
    for key in SENSORS:
        if key in config:
            conf = config[key]
            sens = yield sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_sensor")(sens))
