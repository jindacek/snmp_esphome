import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.const import (
    CONF_ID,
    CONF_UPDATE_INTERVAL,
)

from esphome.components import sensor

DEPENDENCIES = ["network"]
AUTO_LOAD = ["sensor"]

snmp_sensor_ns = cg.esphome_ns.namespace("snmp_sensor")
SnmpSensor = snmp_sensor_ns.class_("SnmpSensor", cg.PollingComponent)

CONF_HOST = "host"
CONF_COMMUNITY = "community"

CONF_RUNTIME = "runtime_sensor"
CONF_CAPACITY = "battery_capacity_sensor"
CONF_TEMP = "battery_temp_sensor"
CONF_BATT_V = "battery_voltage_sensor"
CONF_INPUT_V = "input_voltage_sensor"
CONF_OUTPUT_V = "output_voltage_sensor"
CONF_LOAD = "load_sensor"
CONF_STATUS = "output_status_sensor"
CONF_MODEL = "model_sensor"
CONF_NAME = "name_sensor"
CONF_DATE = "manufacture_date_sensor"
CONF_REPL = "last_battery_replacement_sensor"
CONF_START = "last_start_time_sensor"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SnmpSensor),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_COMMUNITY): cv.string,
    cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,

    cv.Optional(CONF_RUNTIME): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_CAPACITY): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_BATT_V): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_INPUT_V): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_OUTPUT_V): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_LOAD): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_STATUS): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_MODEL): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_NAME): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_DATE): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_REPL): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_START): cv.use_id(sensor.Sensor),
})

async def to_code(config):
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_HOST],
        config[CONF_COMMUNITY],
    )

    await cg.register_component(var, config)

    if CONF_RUNTIME in config:
        cg.add(var.set_runtime_sensor(config[CONF_RUNTIME]))
    if CONF_CAPACITY in config:
        cg.add(var.set_battery_capacity_sensor(config[CONF_CAPACITY]))
    if CONF_TEMP in config:
        cg.add(var.set_battery_temp_sensor(config[CONF_TEMP]))
    if CONF_BATT_V in config:
        cg.add(var.set_battery_voltage_sensor(config[CONF_BATT_V]))
    if CONF_INPUT_V in config:
        cg.add(var.set_input_voltage_sensor(config[CONF_INPUT_V]))
    if CONF_OUTPUT_V in config:
        cg.add(var.set_output_voltage_sensor(config[CONF_OUTPUT_V]))
    if CONF_LOAD in config:
        cg.add(var.set_load_sensor(config[CONF_LOAD]))
    if CONF_STATUS in config:
        cg.add(var.set_output_status_sensor(config[CONF_STATUS]))
    if CONF_MODEL in config:
        cg.add(var.set_model_sensor(config[CONF_MODEL]))
    if CONF_NAME in config:
        cg.add(var.set_name_sensor(config[CONF_NAME]))
    if CONF_DATE in config:
        cg.add(var.set_manufacture_date_sensor(config[CONF_DATE]))
    if CONF_REPL in config:
        cg.add(var.set_last_battery_replacement_sensor(config[CONF_REPL]))
    if CONF_START in config:
        cg.add(var.set_last_start_time_sensor(config[CONF_START]))
