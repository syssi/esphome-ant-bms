import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ICON, CONF_ID

from .. import CONF_ANT_BMS_ID, AntBms, ant_bms_ns

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_CHARGING = "charging"
CONF_DISCHARGING = "discharging"
CONF_BALANCER = "balancer"
CONF_RESTART = "restart"

ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_CHARGING = "mdi:battery-charging-50"
ICON_BALANCER = "mdi:seesaw"
ICON_RESTART = "mdi:restart"

# https://github.com/klotztech/VBMS/wiki/Serial-protocol#control-addresses
SWITCHES = {
    CONF_DISCHARGING: 0xF9,
    CONF_CHARGING: 0xFA,
}

AntSwitch = ant_bms_ns.class_("AntSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
        cv.Optional(CONF_DISCHARGING): switch.SWITCH_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntSwitch),
                cv.Optional(CONF_ICON, default=ICON_DISCHARGING): switch.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CHARGING): switch.SWITCH_SCHEMA.extend(
            {
                cv.GenerateID(): cv.declare_id(AntSwitch),
                cv.Optional(CONF_ICON, default=ICON_CHARGING): switch.icon,
            }
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


def to_code(config):
    hub = yield cg.get_variable(config[CONF_ANT_BMS_ID])
    for key, address in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            yield cg.register_component(var, conf)
            yield switch.register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
