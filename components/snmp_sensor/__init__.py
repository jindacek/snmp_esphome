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

    cv.Optional("runtime_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("battery_capacity_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("battery_temp_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("battery_voltage_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("input_voltage_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("output_voltage_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("load_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("output_status_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("model_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("name_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("manufacture_date_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("last_battery_replacement_sensor"): cv.use_id(cg.Sensor),
    cv.Optional("last_start_time_sensor"): cv.use_id(cg.Sensor),

    cv.Optional(CONF_UPDATE_INTERVAL, default="10s"): cv.update_interval,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_HOST], config[CONF_COMMUNITY])
    await cg.register_component(var, config)

    if "runtime_sensor" in config:
        s = await cg.get_variable(config["runtime_sensor"])
        cg.add(var.set_runtime_sensor(s))
    if "battery_capacity_sensor" in config:
        s = await cg.get_variable(config["battery_capacity_sensor"])
        cg.add(var.set_battery_capacity_sensor(s))
    if "battery_temp_sensor" in config:
        s = await cg.get_variable(config["battery_temp_sensor"])
        cg.add(var.set_battery_temp_sensor(s))
    if "battery_voltage_sensor" in config:
        s = await cg.get_variable(config["battery_voltage_sensor"])
        cg.add(var.set_battery_voltage_sensor(s))
    if "input_voltage_sensor" in config:
        s = await cg.get_variable(config["input_voltage_sensor"])
        cg.add(var.set_input_voltage_sensor(s))
    if "output_voltage_sensor" in config:
        s = await cg.get_variable(config["output_voltage_sensor"])
        cg.add(var.set_output_voltage_sensor(s))
    if "load_sensor" in config:
        s = await cg.get_variable(config["load_sensor"])
        cg.add(var.set_load_sensor(s))
    if "output_status_sensor" in config:
        s = await cg.get_variable(config["output_status_sensor"])
        cg.add(var.set_output_status_sensor(s))
    if "model_sensor" in config:
        s = await cg.get_variable(config["model_sensor"])
        cg.add(var.set_model_sensor(s))
    if "name_sensor" in config:
        s = await cg.get_variable(config["name_sensor"])
        cg.add(var.set_name_sensor(s))
    if "manufacture_date_sensor" in config:
        s = await cg.get_variable(config["manufacture_date_sensor"])
        cg.add(var.set_manufacture_date_sensor(s))
    if "last_battery_replacement_sensor" in config:
        s = await cg.get_variable(config["last_battery_replacement_sensor"])
        cg.add(var.set_last_battery_replacement_sensor(s))
    if "last_start_time_sensor" in config:
        s = await cg.get_variable(config["last_start_time_sensor"])
        cg.add(var.set_last_start_time_sensor(s))
