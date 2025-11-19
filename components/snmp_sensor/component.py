import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

DEPENDENCIES = ['network']
AUTO_LOAD = ['sensor']

snmp_ns = cg.esphome_ns.namespace('snmp')
SnmpSensor = snmp_ns.class_('SnmpSensor', sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(SnmpSensor),
    cv.Required("host"): cv.string,
    cv.Required("community"): cv.string,
    cv.Required("oid"): cv.string,
    cv.Optional("port", default=161): cv.port,
}).extend(cv.polling_component_schema('60s'))

def to_code(config):
    var = cg.new_Pvariable(config["id"])
    yield cg.register_component(var, config)
    yield sensor.register_sensor(var, config)
    
    cg.add(var.set_host(config["host"]))
    cg.add(var.set_community(config["community"]))
    cg.add(var.set_oid(config["oid"]))
    cg.add(var.set_port(config["port"]))
    
    # Předání vlastností ze schématu senzoru
    if "unit_of_measurement" in config:
        cg.add(var.set_unit_of_measurement(config["unit_of_measurement"]))
    if "accuracy_decimals" in config:
        cg.add(var.set_accuracy_decimals(config["accuracy_decimals"]))
    if "state_class" in config:
        cg.add(var.set_state_class(config["state_class"]))
    if "device_class" in config:
        cg.add(var.set_device_class(config["device_class"]))
