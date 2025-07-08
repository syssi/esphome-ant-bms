import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import CONF_ID

try:
    from esphome.const import CONF_BLUETOOTH
except ImportError:
    CONF_BLUETOOTH = "bluetooth"

from .. import ANT_BMS_BLE_COMPONENT_SCHEMA, CONF_ANT_BMS_BLE_ID, ant_bms_ble_ns
from ..const import CONF_BALANCER, CONF_CHARGING, CONF_DISCHARGING

DEPENDENCIES = ["ant_bms_ble"]

CODEOWNERS = ["@syssi"]

# CONF_BLUETOOTH = "bluetooth"
CONF_BUZZER = "buzzer"

ICON_DISCHARGING = "mdi:battery-charging-50"
ICON_CHARGING = "mdi:battery-charging-50"
ICON_BALANCER = "mdi:seesaw"
ICON_BLUETOOTH = "mdi:bluetooth"
ICON_BUZZER = "mdi:volume-high"

SWITCHES = {
    # turn on register, turn off register
    CONF_DISCHARGING: [0x0003, 0x0001],
    CONF_CHARGING: [0x0006, 0x0004],
    CONF_BALANCER: [0x000D, 0x000E],
    CONF_BLUETOOTH: [0x001D, 0x001C],
    CONF_BUZZER: [0x001E, 0x001F],
}

# Discharge Mosfet off    7e a1 51 01 00 00 d8 e5 aa 55
# Discharge Mosfet on     7e a1 51 03 00 00 79 25 aa 55
# Charge Mosfet off       7e a1 51 04 00 00 c8 e4 aa 55
# Charge Mosfet on        7e a1 51 06 00 00 69 24 aa 55
# Manual Balance on       7e a1 51 0d 00 00 18 e6 aa 55
# Manual Balance off      7e a1 51 0e 00 00 e8 e6 aa 55
# Bluetooth off           7e a1 51 1c 00 00 48 e3 aa 55
# Bluetooth on            7e a1 51 1d 00 00 19 23 aa 55
# Buzzer on               7e a1 51 1e 00 00 e9 23 aa 55
# Buzzer off              7e a1 51 1f 00 00 b8 e3 aa 55
#
# https://github.com/syssi/esphome-ant-bms/issues/20

AntSwitch = ant_bms_ble_ns.class_("AntSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = ANT_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_DISCHARGING): switch.switch_schema(
            AntSwitch, icon=ICON_DISCHARGING
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_CHARGING): switch.switch_schema(
            AntSwitch, icon=ICON_CHARGING
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BALANCER): switch.switch_schema(
            AntSwitch, icon=ICON_BALANCER
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BLUETOOTH): switch.switch_schema(
            AntSwitch, icon=ICON_BLUETOOTH
        ).extend(cv.COMPONENT_SCHEMA),
        cv.Optional(CONF_BUZZER): switch.switch_schema(
            AntSwitch, icon=ICON_BUZZER
        ).extend(cv.COMPONENT_SCHEMA),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_BLE_ID])
    for key, address in SWITCHES.items():
        if key in config:
            conf = config[key]
            var = cg.new_Pvariable(conf[CONF_ID])
            await cg.register_component(var, conf)
            await switch.register_switch(var, conf)
            cg.add(getattr(hub, f"set_{key}_switch")(var))
            cg.add(var.set_parent(hub))
            cg.add(var.set_turn_on_register(address[0]))
            cg.add(var.set_turn_off_register(address[1]))
