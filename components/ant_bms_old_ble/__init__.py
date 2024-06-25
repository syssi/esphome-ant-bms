import esphome.codegen as cg
from esphome.components import ble_client
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_PASSWORD

CODEOWNERS = ["@syssi"]

AUTO_LOAD = ["binary_sensor", "button", "sensor", "switch", "text_sensor"]
MULTI_CONF = True

CONF_ANT_BMS_OLD_BLE_ID = "ant_bms_old_ble_id"

ant_bms_old_ble_ns = cg.esphome_ns.namespace("ant_bms_old_ble")
AntBmsOldBle = ant_bms_old_ble_ns.class_(
    "AntBmsOldBle", ble_client.BLEClientNode, cg.PollingComponent
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AntBmsOldBle),
            cv.Optional(CONF_PASSWORD, default="12345678"): cv.Any(
                cv.All(cv.string_strict, cv.Length(min=8, max=8)),
                cv.All(cv.string_strict, cv.Length(min=0, max=0)),
            ),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await ble_client.register_ble_node(var, config)

    if CONF_PASSWORD in config:
        cg.add(var.set_password(config[CONF_PASSWORD]))
    else:
        cg.add(var.set_password(""))
