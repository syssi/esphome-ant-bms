import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ICON_TIMELAPSE

from . import CONF_ANT_BMS_BLE_ID, AntBmsBle

DEPENDENCIES = ["ant_bms_ble"]

CODEOWNERS = ["@syssi"]

CONF_CHARGE_MOSFET_STATUS = "charge_mosfet_status"
CONF_DISCHARGE_MOSFET_STATUS = "discharge_mosfet_status"
CONF_BALANCER_STATUS = "balancer_status"
CONF_DEVICE_MODEL = "device_model"
CONF_SOFTWARE_VERSION = "software_version"
CONF_TOTAL_RUNTIME_FORMATTED = "total_runtime_formatted"

ICON_CHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_DISCHARGE_MOSFET_STATUS = "mdi:heart-pulse"
ICON_BALANCER_STATUS = "mdi:heart-pulse"
ICON_DEVICE_MODEL = "mdi:identifier"
ICON_SOFTWARE_VERSION = "mdi:chip"
# ICON_TOTAL_RUNTIME_FORMATTED = ICON_TIMELAPSE

TEXT_SENSORS = [
    CONF_CHARGE_MOSFET_STATUS,
    CONF_DISCHARGE_MOSFET_STATUS,
    CONF_BALANCER_STATUS,
    CONF_DEVICE_MODEL,
    CONF_SOFTWARE_VERSION,
    CONF_TOTAL_RUNTIME_FORMATTED,
]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_BLE_ID): cv.use_id(AntBmsBle),
        cv.Optional(CONF_CHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_CHARGE_MOSFET_STATUS
        ),
        cv.Optional(CONF_DISCHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_DISCHARGE_MOSFET_STATUS
        ),
        cv.Optional(CONF_BALANCER_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_BALANCER_STATUS
        ),
        cv.Optional(CONF_DEVICE_MODEL): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_DEVICE_MODEL
        ),
        cv.Optional(CONF_SOFTWARE_VERSION): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_SOFTWARE_VERSION
        ),
        cv.Optional(CONF_TOTAL_RUNTIME_FORMATTED): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_TIMELAPSE
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_BLE_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
