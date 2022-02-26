import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ADDRESS, CONF_ID

DEPENDENCIES = ["uart"]

ant_modbus_ns = cg.esphome_ns.namespace("ant_modbus")
AntModbus = ant_modbus_ns.class_("AntModbus", cg.Component, uart.UARTDevice)
AntModbusDevice = ant_modbus_ns.class_("AntModbusDevice")
MULTI_CONF = True

CONF_ANT_MODBUS_ID = "ant_modbus_id"
CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntModbus),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    cg.add_global(ant_modbus_ns.using)
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    await uart.register_uart_device(var, config)


def ant_modbus_device_schema(default_address):
    schema = {
        cv.GenerateID(CONF_ANT_MODBUS_ID): cv.use_id(AntModbus),
    }
    if default_address is None:
        schema[cv.Required(CONF_ADDRESS)] = cv.hex_uint8_t
    else:
        schema[cv.Optional(CONF_ADDRESS, default=default_address)] = cv.hex_uint8_t
    return cv.Schema(schema)


async def register_ant_modbus_device(var, config):
    parent = await cg.get_variable(config[CONF_ANT_MODBUS_ID])
    cg.add(var.set_parent(parent))
    cg.add(var.set_address(config[CONF_ADDRESS]))
    cg.add(parent.register_device(var))
