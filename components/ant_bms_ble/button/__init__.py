import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import CONF_FACTORY_RESET, CONF_RESTART, DEVICE_CLASS_RESTART

from .. import ANT_BMS_BLE_COMPONENT_SCHEMA, CONF_ANT_BMS_BLE_ID, ant_bms_ble_ns

DEPENDENCIES = ["ant_bms_ble"]

CODEOWNERS = ["@syssi"]

# CONF_RESTART = "restart"
CONF_SHUTDOWN = "shutdown"
# CONF_FACTORY_RESET = "factory_reset"
CONF_CLEAR_SYSTEM_LOG = "clear_system_log"
CONF_CURRENT_ZERO = "current_zero"
CONF_RESET_HARDWARE = "reset_hardware"
CONF_BLUETOOTH_INITIALIZATION = "bluetooth_initialization"
CONF_SAVE_CUSTOMER_DATA = "save_customer_data"
CONF_CLEAR_DISCHARGE_CYCLE_AH = "clear_discharge_cycle_ah"
CONF_CLEAR_CHARGE_CYCLE_AH = "clear_charge_cycle_ah"
CONF_CLEAR_DISCHARGE_TIME = "clear_discharge_time"
CONF_CLEAR_CHARGE_TIME = "clear_charge_time"
CONF_CLEAR_RUNNING_TIME = "clear_running_time"
CONF_CLEAR_PROTECT_TIME = "clear_protect_time"
CONF_BLUETOOTH_ON = "bluetooth_on"
CONF_BLUETOOTH_OFF = "bluetooth_off"

ICON_RESTART = "mdi:restart"
ICON_SHUTDOWN = "mdi:power"
ICON_FACTORY_RESET = "mdi:factory"
ICON_CLEAR_SYSTEM_LOG = "mdi:delete-clock"
ICON_CURRENT_ZERO = "mdi:current-dc"
ICON_RESET_HARDWARE = "mdi:restart-alert"
ICON_BLUETOOTH_INITIALIZATION = "mdi:bluetooth-settings"
ICON_SAVE_CUSTOMER_DATA = "mdi:content-save"
ICON_CLEAR_AH = "mdi:counter"
ICON_CLEAR_TIME = "mdi:timer-off"
ICON_BLUETOOTH_ON = "mdi:bluetooth-connect"
ICON_BLUETOOTH_OFF = "mdi:bluetooth-off"

# Buttons (from https://github.com/syssi/esphome-ant-bms/issues/29)
#
# Current Zero                7e a1 51 08 00 00 08 e7 aa 55
# Restart                     7e a1 51 09 00 00 59 27 aa 55
# Shutdown                    7e a1 51 0b 00 00 f8 e7 aa 55
# Factory reset               7e a1 51 0c 00 00 49 26 aa 55
# Clear System Log            7e a1 51 0f 00 00 b9 26 aa 55
# Bluetooth initialization    7e a1 51 10 00 00 88 e0 aa 55
# Clear Discharge Cycle Ah    7e a1 51 20 00 00 88 ef aa 55
# Clear Charge Cycle Ah       7e a1 51 21 00 00 d9 2f aa 55
# Clear Discharge Time        7e a1 51 22 00 00 29 2f aa 55
# Clear Charge Time           7e a1 51 23 00 00 78 ef aa 55
# Clear Running Time          7e a1 51 24 00 00 c9 2e aa 55
# Clear Protect Time          7e a1 51 25 00 00 98 ee aa 55
# Reset hardware              7e a1 51 2a 00 00 a8 ed aa 55
# Save Customer Data          7e a1 51 2c 00 00 48 ec aa 55
# Bluetooth on                7e a1 51 1d 00 00 19 23 aa 55
# Bluetooth off               7e a1 51 1c 00 00 48 e3 aa 55

BUTTONS = {
    CONF_CURRENT_ZERO: 0x0008,
    CONF_RESTART: 0x0009,
    CONF_SHUTDOWN: 0x000B,
    CONF_FACTORY_RESET: 0x000C,
    CONF_CLEAR_SYSTEM_LOG: 0x000F,
    CONF_BLUETOOTH_INITIALIZATION: 0x0010,
    CONF_BLUETOOTH_OFF: 0x001C,
    CONF_BLUETOOTH_ON: 0x001D,
    CONF_CLEAR_DISCHARGE_CYCLE_AH: 0x0020,
    CONF_CLEAR_CHARGE_CYCLE_AH: 0x0021,
    CONF_CLEAR_DISCHARGE_TIME: 0x0022,
    CONF_CLEAR_CHARGE_TIME: 0x0023,
    CONF_CLEAR_RUNNING_TIME: 0x0024,
    CONF_CLEAR_PROTECT_TIME: 0x0025,
    CONF_RESET_HARDWARE: 0x002A,
    CONF_SAVE_CUSTOMER_DATA: 0x002C,
}

AntButton = ant_bms_ble_ns.class_("AntButton", button.Button, cg.Component)

CONFIG_SCHEMA = ANT_BMS_BLE_COMPONENT_SCHEMA.extend(
    {
        cv.Optional(CONF_CURRENT_ZERO): button.button_schema(
            AntButton, icon=ICON_CURRENT_ZERO
        ),
        cv.Optional(CONF_RESTART): button.button_schema(
            AntButton, icon=ICON_RESTART, device_class=DEVICE_CLASS_RESTART
        ),
        cv.Optional(CONF_SHUTDOWN): button.button_schema(AntButton, icon=ICON_SHUTDOWN),
        cv.Optional(CONF_FACTORY_RESET): button.button_schema(
            AntButton, icon=ICON_FACTORY_RESET, device_class=DEVICE_CLASS_RESTART
        ),
        cv.Optional(CONF_CLEAR_SYSTEM_LOG): button.button_schema(
            AntButton, icon=ICON_CLEAR_SYSTEM_LOG
        ),
        cv.Optional(CONF_BLUETOOTH_INITIALIZATION): button.button_schema(
            AntButton, icon=ICON_BLUETOOTH_INITIALIZATION
        ),
        cv.Optional(CONF_BLUETOOTH_OFF): button.button_schema(
            AntButton, icon=ICON_BLUETOOTH_OFF
        ),
        cv.Optional(CONF_BLUETOOTH_ON): button.button_schema(
            AntButton, icon=ICON_BLUETOOTH_ON
        ),
        cv.Optional(CONF_CLEAR_DISCHARGE_CYCLE_AH): button.button_schema(
            AntButton, icon=ICON_CLEAR_AH
        ),
        cv.Optional(CONF_CLEAR_CHARGE_CYCLE_AH): button.button_schema(
            AntButton, icon=ICON_CLEAR_AH
        ),
        cv.Optional(CONF_CLEAR_DISCHARGE_TIME): button.button_schema(
            AntButton, icon=ICON_CLEAR_TIME
        ),
        cv.Optional(CONF_CLEAR_CHARGE_TIME): button.button_schema(
            AntButton, icon=ICON_CLEAR_TIME
        ),
        cv.Optional(CONF_CLEAR_RUNNING_TIME): button.button_schema(
            AntButton, icon=ICON_CLEAR_TIME
        ),
        cv.Optional(CONF_CLEAR_PROTECT_TIME): button.button_schema(
            AntButton, icon=ICON_CLEAR_TIME
        ),
        cv.Optional(CONF_RESET_HARDWARE): button.button_schema(
            AntButton, icon=ICON_RESET_HARDWARE
        ),
        cv.Optional(CONF_SAVE_CUSTOMER_DATA): button.button_schema(
            AntButton, icon=ICON_SAVE_CUSTOMER_DATA
        ),
    }
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_BLE_ID])
    for key, address in BUTTONS.items():
        if key in config:
            conf = config[key]
            var = await button.new_button(conf)
            await cg.register_component(var, conf)
            cg.add(var.set_parent(hub))
            cg.add(var.set_holding_register(address))
