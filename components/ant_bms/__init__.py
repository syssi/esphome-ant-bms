import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import CONF_ID

CODEOWNERS = ["@syssi"]

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_ANT_BMS_ID = "ant_bms_id"
CONF_RX_TIMEOUT = "rx_timeout"

ant_bms_ns = cg.esphome_ns.namespace("ant_bms")
AntBms = ant_bms_ns.class_("AntBms", cg.PollingComponent, uart.UARTDevice)

ANT_BMS_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ANT_BMS_ID): cv.use_id(AntBms),
    }
)

CONFIG_SCHEMA = cv.All(
    cv.require_esphome_version(2024, 12, 0),
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBms),
            cv.Optional(
                CONF_RX_TIMEOUT, default="50ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional("supports_new_commands"): cv.invalid(
                "The ant_bms component no longer supports the old 0xAA protocol. "
                "Remove this option. If your device uses the old protocol, switch to ant_bms_old."
            ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(uart.UART_DEVICE_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
