import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PASSWORD

CODEOWNERS = ["@syssi"]

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_ANT_BMS_ID = "ant_bms_id"
CONF_RX_TIMEOUT = "rx_timeout"
CONF_SUPPORTS_NEW_COMMANDS = "supports_new_commands"

ant_bms_ns = cg.esphome_ns.namespace("ant_bms")
AntBms = ant_bms_ns.class_("AntBms", cg.PollingComponent, uart.UARTDevice)

ANT_BMS_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
    }
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBms),
            cv.Optional(
                CONF_RX_TIMEOUT, default="50ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_SUPPORTS_NEW_COMMANDS, default=False): cv.boolean,
            cv.Optional(CONF_PASSWORD, default=""): cv.Any(
                cv.All(cv.string_strict, cv.Length(min=8, max=8)),
                cv.All(cv.string_strict, cv.Length(min=0, max=0)),
            ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    cg.add(var.set_supports_new_commands(config[CONF_SUPPORTS_NEW_COMMANDS]))

    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))
    else:
        cg.add(var.set_password(""))
