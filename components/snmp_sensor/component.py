import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_PORT

DEPENDENCIES = ['network']
AUTO_LOAD = ['sensor']

snmp_ns = cg.esphome_ns.namespace('snmp')
SNMPSensor = snmp_ns.class_('SNMPSensor', sensor.Sensor, cg.Component)

CONF_COMMUNITY = 'community'
CONF_OID = 'oid'
CONF_HOST = 'host'

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(SNMPSensor),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_COMMUNITY): cv.string,
    cv.Required(CONF_OID): cv.string,
    cv.Optional(CONF_PORT, default=161): cv.port,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    
    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))
    cg.add(var.set_oid(config[CONF_OID]))
    cg.add(var.set_port(config[CONF_PORT]))
