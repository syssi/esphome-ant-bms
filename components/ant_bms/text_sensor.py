import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC, ICON_TIMELAPSE

from . import ANT_BMS_COMPONENT_SCHEMA, CONF_ANT_BMS_ID

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_CHARGE_MOSFET_STATUS = "charge_mosfet_status"
CONF_DISCHARGE_MOSFET_STATUS = "discharge_mosfet_status"
CONF_BALANCER_STATUS = "balancer_status"
CONF_TOTAL_RUNTIME_FORMATTED = "total_runtime_formatted"

ICON_CHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_DISCHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_BALANCER_STATUS = "mdi:heart-pulse"

TEXT_SENSORS = [
    CONF_CHARGE_MOSFET_STATUS,
    CONF_DISCHARGE_MOSFET_STATUS,
    CONF_BALANCER_STATUS,
    CONF_TOTAL_RUNTIME_FORMATTED,
]

CONFIG_SCHEMA = ANT_BMS_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_CHARGE_MOSFET_STATUS,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_DISCHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_DISCHARGE_MOSFET_STATUS,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_BALANCER_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_BALANCER_STATUS,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_TOTAL_RUNTIME_FORMATTED): text_sensor.text_sensor_schema(
            icon=ICON_TIMELAPSE,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
