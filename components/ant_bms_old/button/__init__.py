import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_FACTORY_RESET, CONF_ID, CONF_RESTART

from .. import CONF_ANT_BMS_OLD_ID, AntBmsOld, ant_bms_old_ns
from ..const import CONF_BALANCER

DEPENDENCIES = ["ant_bms_old"]

CODEOWNERS = ["@syssi"]

CONF_SHUTDOWN = "shutdown"
CONF_CLEAR_COUNTER = "clear_counter"
# CONF_BALANCER = "balancer"
# CONF_FACTORY_RESET = "factory_reset"
# CONF_RESTART = "restart"

ICON_SHUTDOWN = "mdi:power"
ICON_CLEAR_COUNTER = "mdi:delete-clock"
ICON_BALANCER = "mdi:seesaw"
ICON_FACTORY_RESET = "mdi:factory"
ICON_RESTART = "mdi:restart"

# 0xEF 0x1A 0x01    Request upgrade command
# 0xF0 0x00 0x00    Apply LTO settings
# 0xF6 0x00 0x00    Change MAC address (randomize?)
# 0xFB 0x00 0x00    Apply LiFePo4 settings
#
# https://github.com/klotztech/VBMS/wiki/Serial-protocol#control-addresses
BUTTONS = {
    CONF_SHUTDOWN: 0xF7,
    CONF_CLEAR_COUNTER: 0xF8,
    CONF_BALANCER: 0xFC,
    CONF_FACTORY_RESET: 0xFD,
    CONF_RESTART: 0xFE,
}

AntButton = ant_bms_old_ns.class_("AntButton", button.Button, cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_OLD_ID): cv.use_id(AntBmsOld),
        cv.Optional(CONF_SHUTDOWN): button.button_schema(
            AntButton, icon=ICON_SHUTDOWN
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CLEAR_COUNTER): button.button_schema(
            AntButton, icon=ICON_CLEAR_COUNTER
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BALANCER): button.button_schema(
            AntButton, icon=ICON_BALANCER
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
    hub = await cg.get_variable(config[CONF_ANT_BMS_OLD_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await button.register_button(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
