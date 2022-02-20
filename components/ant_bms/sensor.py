import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_PERCENT,
)

from . import CONF_ANT_BMS_ID, AntBms

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_CAPACITY_REMAINING = "capacity_remaining"
ICON_CAPACITY_REMAINING = "mdi:battery-50"

SENSORS = [
    CONF_CAPACITY_REMAINING,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
        # pylint: disable=too-many-function-args
        cv.Optional(CONF_CAPACITY_REMAINING): sensor.sensor_schema(
            UNIT_PERCENT,
            ICON_CAPACITY_REMAINING,
            0,
            DEVICE_CLASS_EMPTY,
            STATE_CLASS_MEASUREMENT,
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
