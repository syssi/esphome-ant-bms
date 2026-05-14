import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC, ICON_TIMELAPSE

from . import ANT_BMS_BLE_COMPONENT_SCHEMA, CONF_ANT_BMS_BLE_ID

DEPENDENCIES = ["ant_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_CHARGE_MOSFET_STATUS = "charge_mosfet_status"
CONF_DISCHARGE_MOSFET_STATUS = "discharge_mosfet_status"
CONF_BALANCER_STATUS = "balancer_status"
CONF_BATTERY_STATUS = "battery_status"
CONF_DEVICE_MODEL = "device_model"
CONF_SOFTWARE_VERSION = "software_version"
CONF_TOTAL_RUNTIME_FORMATTED = "total_runtime_formatted"
CONF_TOTAL_DISCHARGING_TIME_FORMATTED = "total_discharging_time_formatted"
CONF_TOTAL_CHARGING_TIME_FORMATTED = "total_charging_time_formatted"

ICON_CHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_DISCHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_BALANCER_STATUS = "mdi:heart-pulse"
ICON_BATTERY_STATUS = "mdi:heart-pulse"
ICON_DEVICE_MODEL = "mdi:identifier"
ICON_SOFTWARE_VERSION = "mdi:chip"
# ICON_TOTAL_RUNTIME_FORMATTED = ICON_TIMELAPSE

TEXT_SENSORS = [
    CONF_CHARGE_MOSFET_STATUS,
    CONF_DISCHARGE_MOSFET_STATUS,
    CONF_BALANCER_STATUS,
    CONF_BATTERY_STATUS,
    CONF_DEVICE_MODEL,
    CONF_SOFTWARE_VERSION,
    CONF_TOTAL_RUNTIME_FORMATTED,
    CONF_TOTAL_DISCHARGING_TIME_FORMATTED,
    CONF_TOTAL_CHARGING_TIME_FORMATTED,
]

CONFIG_SCHEMA = ANT_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_CHARGE_MOSFET_STATUS,
        ),
        cv.Optional(CONF_DISCHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_DISCHARGE_MOSFET_STATUS,
        ),
        cv.Optional(CONF_BALANCER_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_BALANCER_STATUS,
        ),
        cv.Optional(CONF_BATTERY_STATUS): text_sensor.text_sensor_schema(
            icon=ICON_BATTERY_STATUS,
        ),
        cv.Optional(CONF_DEVICE_MODEL): text_sensor.text_sensor_schema(
            icon=ICON_DEVICE_MODEL,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_SOFTWARE_VERSION): text_sensor.text_sensor_schema(
            icon=ICON_SOFTWARE_VERSION,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_TOTAL_RUNTIME_FORMATTED): text_sensor.text_sensor_schema(
            icon=ICON_TIMELAPSE,
        ),
        cv.Optional(
            CONF_TOTAL_DISCHARGING_TIME_FORMATTED
        ): text_sensor.text_sensor_schema(
            icon=ICON_TIMELAPSE,
        ),
        cv.Optional(CONF_TOTAL_CHARGING_TIME_FORMATTED): text_sensor.text_sensor_schema(
            icon=ICON_TIMELAPSE,
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_BLE_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
