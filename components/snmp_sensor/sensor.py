import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID
from . import snmp_ns

SnmpSensor = snmp_ns.class_("SnmpSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = (
    sensor.sensor_schema()
    .extend({
        cv.GenerateID(): cv.declare_id(SnmpSensor),
    })
    .extend(cv.polling_component_schema("10s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)
