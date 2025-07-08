import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

from .. import ANT_BMS_OLD_COMPONENT_SCHEMA, CONF_ANT_BMS_OLD_ID, ant_bms_old_ns
from ..const import CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["ant_bms_old"]

CODEOWNERS = ["@syssi"]

# CONF_CHARGING = "charging"
# CONF_DISCHARGING = "discharging"

ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_CHARGING = "mdi:battery-charging-50"

# https://github.com/klotztech/VBMS/wiki/Serial-protocol#control-addresses
SWITCHES = {
    CONF_DISCHARGING: 0xF9,
    CONF_CHARGING: 0xFA,
}

AntSwitch = ant_bms_old_ns.class_("AntSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = ANT_BMS_OLD_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_DISCHARGING): switch.switch_schema(
            AntSwitch, icon=ICON_DISCHARGING
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CHARGING): switch.switch_schema(
            AntSwitch, icon=ICON_CHARGING
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_OLD_ID])
    for key, address in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
