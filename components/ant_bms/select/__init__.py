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
    0x0000: "CellOvervoltageProtection",
    0x0002: "CellOvervoltageRecovery",
    0x0004: "CellOvervoltageProtectionL2",
    0x0006: "CellOvervoltageRecoveryL2",
    0x0008: "PackOvervoltageProtection",
    0x000A: "PackOvervoltageRecovery",
    0x000C: "CellUndervoltageProtection",
    0x000E: "CellUndervoltageRecovery",
    0x0010: "CellUndervoltageProtectionL2",
    0x0012: "CellUndervoltageRecoveryL2",
    0x0014: "PackUndervoltageProtection",
    0x0016: "PackUndervoltageRecovery",
    0x0018: "CellVoltageDifferenceProtection",
    0x001A: "CellVoltageDifferenceRecovery",
    0x0020: "CellOvervoltageWarning",
    0x0022: "CellOvervoltageWarningRecovery",
    0x0024: "PackOvervoltageWarning",
    0x0026: "PackOvervoltageWarningRecovery",
    0x0028: "CellUndervoltageWarning",
    0x002A: "CellUndervoltageWarningRecovery",
    0x002C: "PackUndervoltageWarning",
    0x002E: "PackUndervoltageWarningRecovery",
    0x0030: "CellVoltageDifferenceWarning",
    0x0032: "CellVoltageDifferenceWarningRecovery",
    0x0068: "ChargeOvercurrentProtection",
    0x006A: "ChargeOvercurrentProtectionDelay",
    0x006C: "DischargeOvercurrentProtection",
    0x006E: "DischargeOvercurrentProtectionDelay",
    0x0070: "DischargeOvercurrentProtectionL2",
    0x0072: "DischargeOvercurrentProtectionDelayL2",
    0x0074: "ShortCircuitProtection",
    0x0076: "ShortCircuitProtectionDelay",
    0x007C: "ChargeOvercurrentWarning",
    0x007E: "ChargeOvercurrentWarningRecovery",
    0x0080: "DischargeOvercurrentWarning",
    0x0082: "DischargeOvercurrentWarningRecovery",
    0x0084: "SOCLowLevel1Warning",
    0x0086: "SOCLowLevel2Warning",
    0x008C: "CellBalancingVoltage",
    0x008E: "CellBalancingStartVoltage",
    0x0090: "CellVoltageDifferenceBalancingOn",
    0x0092: "CellVoltageDifferenceBalancingOff",
    0x0094: "BalancingCurrent",
    0x0096: "BalancingChargingCurrent",
    0x0098: "CellType",
    0x009A: "CellNumber",
    0x009C: "CellInternalResistanceCalibration",
    0x009E: "ShutdownVoltage",
    0x00A0: "RequestChargeCurrent",
    0x00A2: "NominalCapacity",
    0x00A6: "RemainingCapacity",
    0x00AA: "TotalCycleCapacity",
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
