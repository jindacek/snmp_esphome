import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

DEPENDENCIES = ['network']
AUTO_LOAD = ['sensor']

snmp_ns = cg.esphome_ns.namespace('snmp')
# ZMĚNA: SnmpSensor místo SNMPSensor
SnmpSensor = snmp_ns.class_('SnmpSensor', sensor.Sensor, cg.Component)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(SnmpSensor),
    cv.Required("host"): cv.string,
    cv.Required("community"): cv.string,
    cv.Required("oid"): cv.string,
    cv.Optional("port", default=161): cv.port,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config["id"])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    
    cg.add(var.set_host(config["host"]))
    cg.add(var.set_community(config["community"]))
    cg.add(var.set_oid(config["oid"]))
    cg.add(var.set_port(config["port"]))
