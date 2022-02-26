import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_ANT_BMS_ID, AntBms, ant_bms_ns

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_SHUTDOWN = "shutdown"
CONF_CLEAR_COUNTER = "clear_counter"
CONF_BALANCER = "balancer"
CONF_FACTORY_RESET = "factory_reset"
CONF_RESTART = "restart"

ICON_SHUTDOWN = "mdi:power"
ICON_CLEAR_COUNTER = "mdi:delete-clock"
ICON_BALANCER = "mdi:seesaw"
ICON_FACTORY_RESET = "mdi:factory"
ICON_RESTART = "mdi:restart"

# 0xF8 0x00 0x00 -> clearBmsCurrentCmd
# 0xEF 0x1A 0x01 -> requestUpgradeCmd
#
# https://github.com/klotztech/VBMS/wiki/Serial-protocol#control-addresses
BUTTONS = {
    CONF_SHUTDOWN: 0xF7,
    CONF_CLEAR_COUNTER: 0xF8,
    CONF_BALANCER: 0xFC,
    CONF_FACTORY_RESET: 0xFD,
    CONF_RESTART: 0xFE,
}

AntButton = ant_bms_ns.class_("AntButton", button.Button, cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
        cv.Optional(CONF_SHUTDOWN): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntButton),
                cv.Optional(CONF_ICON, default=ICON_SHUTDOWN): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CLEAR_COUNTER): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntButton),
                cv.Optional(CONF_ICON, default=ICON_CLEAR_COUNTER): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BALANCER): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntButton),
                cv.Optional(CONF_ICON, default=ICON_BALANCER): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_FACTORY_RESET): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntButton),
                cv.Optional(CONF_ICON, default=ICON_FACTORY_RESET): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_RESTART): button.BUTTON_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntButton),
                cv.Optional(CONF_ICON, default=ICON_RESTART): cv.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_ANT_BMS_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            yield cg.register_component(var, conf)
            yield button.register_button(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
