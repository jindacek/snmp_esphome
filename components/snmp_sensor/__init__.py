import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

DEPENDENCIES = ["network"]
AUTO_LOAD = ["sensor"]

snmp_sensor_ns = cg.esphome_ns.namespace("snmp_sensor")
SnmpSensor = snmp_sensor_ns.class_("SnmpSensor", cg.PollingComponent)

CONF_HOST = "host"
CONF_COMMUNITY = "community"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SnmpSensor),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_COMMUNITY): cv.string,
    cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_HOST], config[CONF_COMMUNITY])
    await cg.register_component(var, config)
