import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_UPDATE_INTERVAL,
)

from . import snmp_ns

CONF_HOST = "host"
CONF_COMMUNITY = "community"
CONF_OID = "oid"

SnmpSensor = snmp_ns.class_("SnmpSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema()
    .extend({
        cv.GenerateID(): cv.declare_id(SnmpSensor),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_COMMUNITY): cv.string,
        cv.Required(CONF_OID): cv.string,
    })
    .extend(cv.polling_component_schema("10s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    # základní vlastnosti senzoru (název, jednotky)
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))
    cg.add(var.set_oid(config[CONF_OID]))

