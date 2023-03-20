import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID

AUTO_LOAD = ["binary_sensor", "button", "sensor", "switch", "text_sensor"]
CODEOWNERS = ["@syssi"]
MULTI_CONF = True

CONF_ANT_BMS_BLE_ID = "ant_bms_ble_id"
CONF_ENABLE_FAKE_TRAFFIC = "enable_fake_traffic"

ant_bms_ble_ns = cg.esphome_ns.namespace("ant_bms_ble")
AntBmsBle = ant_bms_ble_ns.class_(
    "AntBmsBle", ble_client.BLEClientNode, cg.PollingComponent
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBmsBle),
            cv.Optional(CONF_ENABLE_FAKE_TRAFFIC, default=False): cv.boolean,
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    cg.add(var.set_enable_fake_traffic(config[CONF_ENABLE_FAKE_TRAFFIC]))
