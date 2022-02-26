import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ant_modbus
from esphome.const import CONF_ID

AUTO_LOAD = ["ant_modbus", "button", "sensor", "switch", "text_sensor"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_ANT_BMS_ID = "ant_bms_id"
CONF_ENABLE_FAKE_TRAFFIC = "enable_fake_traffic"

ant_bms_ns = cg.esphome_ns.namespace("ant_bms")
AntBms = ant_bms_ns.class_("AntBms", cg.PollingComponent, ant_modbus.AntModbusDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBms),
            cv.Optional(CONF_ENABLE_FAKE_TRAFFIC, default=False): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(ant_modbus.ant_modbus_device_schema(0xAA))
)


def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield ant_modbus.register_ant_modbus_device(var, config)

    cg.add(var.set_enable_fake_traffic(config[CONF_ENABLE_FAKE_TRAFFIC]))
