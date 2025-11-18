import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import (
    CONF_HOST,
    CONF_ID,
    CONF_NAME,
    CONF_UPDATE_INTERVAL,
    CONF_UNIT_OF_MEASUREMENT,
)
from . import snmp_ns

SNMPSensor = snmp_ns.class_("SNMPSensor", cg.PollingComponent, cg.Component)

CONF_COMMUNITY = "community"
CONF_OID = "oid"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SNMPSensor),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_COMMUNITY): cv.string,
        cv.Required(CONF_OID): cv.string,
        cv.Optional(CONF_NAME): cv.string,
        cv.Optional(CONF_UNIT_OF_MEASUREMENT): cv.string,
        cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,
    }
)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))
    cg.add(var.set_oid(config[CONF_OID]))

    if CONF_NAME in config:
        cg.add(var.set_name(config[CONF_NAME]))

    if CONF_UNIT_OF_MEASUREMENT in config:
        cg.add(var.set_unit(config[CONF_UNIT_OF_MEASUREMENT]))

    yield cg.register_component(var, config)
