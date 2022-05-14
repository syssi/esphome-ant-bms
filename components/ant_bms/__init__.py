import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ant_modbus
from esphome.const import CONF_ID, CONF_PASSWORD

AUTO_LOAD = ["ant_modbus", "button", "sensor", "switch", "text_sensor"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_ANT_BMS_ID = "ant_bms_id"
CONF_ENABLE_FAKE_TRAFFIC = "enable_fake_traffic"
CONF_SUPPORTS_NEW_COMMANDS = "supports_new_commands"

ant_bms_ns = cg.esphome_ns.namespace("ant_bms")
AntBms = ant_bms_ns.class_("AntBms", cg.PollingComponent, ant_modbus.AntModbusDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBms),
            cv.Optional(CONF_ENABLE_FAKE_TRAFFIC, default=False): cv.boolean,
            cv.Optional(CONF_SUPPORTS_NEW_COMMANDS, default=False): cv.boolean,
            cv.Optional(CONF_PASSWORD, default=""): cv.Any(
                cv.All(cv.string_strict, cv.Length(min=8, max=8)),
                cv.All(cv.string_strict, cv.Length(min=0, max=0)),
            ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(ant_modbus.ant_modbus_device_schema(0xAA))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ant_modbus.register_ant_modbus_device(var, config)

    cg.add(var.set_enable_fake_traffic(config[CONF_ENABLE_FAKE_TRAFFIC]))
    cg.add(var.set_supports_new_commands(config[CONF_SUPPORTS_NEW_COMMANDS]))

    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))
    else:
        cg.add(var.set_password(""))
