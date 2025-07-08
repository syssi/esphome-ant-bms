import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_FACTORY_RESET, CONF_ID, CONF_RESTART

from .. import ANT_BMS_BLE_COMPONENT_SCHEMA, CONF_ANT_BMS_BLE_ID, ant_bms_ble_ns

DEPENDENCIES = ["ant_bms_ble"]

CODEOWNERS = ["@syssi"]

# CONF_RESTART = "restart"
CONF_SHUTDOWN = "shutdown"
# CONF_FACTORY_RESET = "factory_reset"
CONF_CLEAR_SYSTEM_LOG = "clear_system_log"

ICON_RESTART = "mdi:restart"
ICON_SHUTDOWN = "mdi:power"
ICON_FACTORY_RESET = "mdi:factory"
ICON_CLEAR_SYSTEM_LOG = "mdi:delete-clock"

# Not implemented yet
#
# Current Zero                7e a1 51 08 00 00 08 e7 aa 55
# Bluetooth initialization    7e a1 51 10 00 00 88 e0 aa 55
# Clear Discharge Cycle Ah    7e a1 51 20 00 00 88 ef aa 55
# Clear Charge Cycle Ah       7e a1 51 21 00 00 d9 2f aa 55
# Clear Discharge Time        7e a1 51 22 00 00 29 2f aa 55
# Clear Charge Time           7e a1 51 23 00 00 78 ef aa 55
# Clear Running Time          7e a1 51 24 00 00 c9 2e aa 55
# Clear Protect Time          7e a1 51 25 00 00 98 ee aa 55
# Reset hardware              7e a1 51 2a 00 00 a8 ed aa 55
# Save Customer Data          7e a1 51 2c 00 00 48 ec aa 55
#
# https://github.com/syssi/esphome-ant-bms/issues/20
# https://github.com/syssi/esphome-ant-bms/issues/29

BUTTONS = {
    CONF_RESTART: 0x0009,
    CONF_SHUTDOWN: 0x000B,
    CONF_FACTORY_RESET: 0x000C,
    CONF_CLEAR_SYSTEM_LOG: 0x000F,
}

# Available as button entities
#
# Restart                     7e a1 51 09 00 00 59 27 aa 55
# Shutdown                    7e a1 51 0b 00 00 f8 e7 aa 55
# Factory reset               7e a1 51 0c 00 00 49 26 aa 55
# Clear System Log            7e a1 51 0f 00 00 b9 26 aa 55

AntButton = ant_bms_ble_ns.class_("AntButton", button.Button, cg.Component)

CONFIG_SCHEMA = ANT_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_SHUTDOWN): button.button_schema(
            AntButton, icon=ICON_SHUTDOWN
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CLEAR_SYSTEM_LOG): button.button_schema(
            AntButton, icon=ICON_CLEAR_SYSTEM_LOG
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_FACTORY_RESET): button.button_schema(
            AntButton, icon=ICON_FACTORY_RESET
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_RESTART): button.button_schema(
            AntButton, icon=ICON_RESTART
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_BLE_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await button.register_button(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
