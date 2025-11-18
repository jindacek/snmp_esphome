import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components import sensor
from esphome.const import CONF_NAME, CONF_ID, CONF_UPDATE_INTERVAL

from . import snmp_ns

CONF_HOST = "host"
CONF_OID = "oid"
CONF_COMMUNITY = "community"

CONFIG_SCHEMA = sensor.sensor_schema().extend({
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_OID): cv.string,
    cv.Optional(CONF_COMMUNITY, default="public"): cv.string,
}).extend(cv.polling_component_schema("2s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_oid(config[CONF_OID]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))
