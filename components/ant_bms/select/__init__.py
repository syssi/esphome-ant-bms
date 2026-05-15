import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv

from .. import ANT_BMS_COMPONENT_SCHEMA, CONF_ANT_BMS_ID, ant_bms_ns

DEPENDENCIES = ["ant_bms"]

CODEOWNERS = ["@syssi"]

CONF_READ_SETTINGS_REGISTER = "read_settings_register"

AntSelect = ant_bms_ns.class_("AntSelect", select.Select, cg.Component)

# 2021 protocol settings registers (address → name)
# Source: https://github.com/syssi/esphome-ant-bms/issues/18
SETTINGS_REGISTERS = {
    0x0000: "CellHighProtect",
    0x0002: "CellHighRecover",
    0x0004: "CellHighProtectL2",
    0x0006: "CellHighRecoverL2",
    0x0008: "PackVoltHighProtect",
    0x000A: "PackVoltHighRecover",
    0x000C: "CellLowProtect",
    0x000E: "CellLowRecover",
    0x0010: "CellLowProtectL2",
    0x0012: "CellLowRecoverL2",
    0x0014: "PackVoltLowProtect",
    0x0016: "PackVoltLowRecover",
    0x0018: "CellDiffProtect",
    0x001A: "CellDiffRecover",
    0x0020: "CellHighWarning",
    0x0022: "CellHighWarningRecover",
    0x0024: "PackVoltHighWarning",
    0x0026: "PackVoltHighWarningRecover",
    0x0028: "CellLowWarning",
    0x002A: "CellLowWarningRecover",
    0x002C: "PackVoltLowWarning",
    0x002E: "PackVoltLowWarningRecover",
    0x0030: "CellDiffWarning",
    0x0032: "CellDiffWarningRecover",
    0x0068: "ChargeOverCurProtect",
    0x006A: "ChargeOverCurProtectDelay",
    0x006C: "DischargeOverCurProtect",
    0x006E: "DischargeOverCurProtectDelay",
    0x0070: "DischargeOverCurProtectL2",
    0x0072: "DischargeOverCurProtectDelayL2",
    0x0074: "ShortCircuitProtect",
    0x0076: "ShortCircuitProtectDelay",
    0x007C: "ChargeOverCurWarning",
    0x007E: "ChargeOverCurWarningRecover",
    0x0080: "DischargeOverCurWarning",
    0x0082: "DischargeOverCurWarningRecover",
    0x0084: "SOCLowLevel1Warning",
    0x0086: "SOCLowLevel2Warning",
    0x008C: "CellBalLimit",
    0x008E: "ChargingCellBalStart",
    0x0090: "CellDiffOn",
    0x0092: "CellDiffOff",
    0x0094: "BalanceCurrent",
    0x0096: "BalanceChargingCurrent",
    0x0098: "CellType",
    0x009A: "CellNumber",
    0x009C: "CellLowR_Cal",
    0x009E: "ShutDownVolt",
    0x00A0: "RequestChargeCurrent",
    0x00A2: "PhysicalAH",
    0x00A6: "RemainingAH",
    0x00AA: "TotalCycleAH",
    0x00C4: "StateOfChargeMethod",
    0x017A: "TireLength",
    0x017C: "PulseValue",
    0x017E: "SecondaryModuleNum",
}

SELECTS = {CONF_READ_SETTINGS_REGISTER}

ANT_SELECT_SCHEMA = select.select_schema(AntSelect, icon="mdi:cog").extend(
    cv.COMPONENT_SCHEMA
)

CONFIG_SCHEMA = ANT_BMS_COMPONENT_SCHEMA.extend(
    {cv.Optional(CONF_READ_SETTINGS_REGISTER): ANT_SELECT_SCHEMA}
)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_ANT_BMS_ID])
    for key in SELECTS:
        if key in config:
            conf = config[key]
            options = list(SETTINGS_REGISTERS.values())
            addresses = list(SETTINGS_REGISTERS.keys())
            var = await select.new_select(conf, options=options)
            await cg.register_component(var, conf)
            cg.add(var.set_select_mappings(addresses))
            cg.add(var.set_parent(hub))
