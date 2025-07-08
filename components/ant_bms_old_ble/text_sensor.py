import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ID, ICON_TIMELAPSE

from . import ANT_BMS_OLD_BLE_COMPONENT_SCHEMA, CONF_ANT_BMS_OLD_BLE_ID

DEPENDENCIES = ["ant_bms_old_ble"]

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

CONFIG_SCHEMA = ANT_BMS_OLD_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_CHARGE_MOSFET_STATUS
        ),
        cv.Optional(CONF_DISCHARGE_MOSFET_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_DISCHARGE_MOSFET_STATUS
        ),
        cv.Optional(CONF_BALANCER_STATUS): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_BALANCER_STATUS
        ),
        cv.Optional(CONF_TOTAL_RUNTIME_FORMATTED): text_sensor.text_sensor_schema(
            text_sensor.TextSensor, icon=ICON_TIMELAPSE
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_OLD_BLE_ID])
    for key in TEXT_SENSORS:
        if key in config:
            conf = config[key]
            sens = cg.new_Pvariable(conf[CONF_ID])
            await text_sensor.register_text_sensor(sens, conf)
            cg.add(getattr(hub, f"set_{key}_text_sensor")(sens))
