import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_UNIT_OF_MEASUREMENT,
    CONF_ACCURACY_DECIMALS,
    CONF_UPDATE_INTERVAL,
)

from . import snmp_ns

CONF_HOST = "host"
CONF_COMMUNITY = "community"

# child numeric keys
CONF_RUNTIME = "runtime"
CONF_BATTERY_CAPACITY = "battery_capacity"
CONF_BATTERY_TEMP = "battery_temp"
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_INPUT_VOLTAGE = "input_voltage"
CONF_OUTPUT_VOLTAGE = "output_voltage"
CONF_LOAD = "load"
CONF_OUTPUT_STATUS = "output_status"
CONF_REMAINING_RUNTIME = "remaining_runtime"
CONF_SELF_TEST_RESULT = "self_test_result"
CONF_BATTERY_REPLACE_STATUS = "battery_replace_status"
CONF_OUTPUT_SOURCE = "output_source"
CONF_INPUT_FREQUENCY = "input_frequency"
CONF_OUTPUT_FREQUENCY = "output_frequency"

SnmpSensor = snmp_ns.class_("SnmpSensor", cg.PollingComponent)

NUM_SENSOR_SCHEMA = sensor.sensor_schema().extend(
    {
        cv.Optional(CONF_UNIT_OF_MEASUREMENT): cv.string,
        cv.Optional(CONF_ACCURACY_DECIMALS): cv.int_,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SnmpSensor),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_COMMUNITY): cv.string,

        cv.Optional(CONF_RUNTIME): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_BATTERY_CAPACITY): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_BATTERY_TEMP): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_BATTERY_VOLTAGE): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_INPUT_VOLTAGE): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_OUTPUT_VOLTAGE): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_LOAD): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_OUTPUT_STATUS): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_REMAINING_RUNTIME): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_SELF_TEST_RESULT): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_BATTERY_REPLACE_STATUS): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_OUTPUT_SOURCE): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_INPUT_FREQUENCY): NUM_SENSOR_SCHEMA,
        cv.Optional(CONF_OUTPUT_FREQUENCY): NUM_SENSOR_SCHEMA,
    }
).extend(cv.polling_component_schema("10s"))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))

    # helper to add child sensors
    async def add_child(key, setter):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(setter(sens))

    await add_child(CONF_RUNTIME, var.set_runtime_sensor)
    await add_child(CONF_BATTERY_CAPACITY, var.set_battery_capacity_sensor)
    await add_child(CONF_BATTERY_TEMP, var.set_battery_temp_sensor)
    await add_child(CONF_BATTERY_VOLTAGE, var.set_battery_voltage_sensor)
    await add_child(CONF_INPUT_VOLTAGE, var.set_input_voltage_sensor)
    await add_child(CONF_OUTPUT_VOLTAGE, var.set_output_voltage_sensor)
    await add_child(CONF_LOAD, var.set_load_sensor)
    await add_child(CONF_OUTPUT_STATUS, var.set_output_status_sensor)
    await add_child(CONF_REMAINING_RUNTIME, var.set_remaining_runtime_sensor)
    await add_child(CONF_SELF_TEST_RESULT, var.set_self_test_result_sensor)
    await add_child(CONF_BATTERY_REPLACE_STATUS, var.set_battery_replace_status_sensor)
    await add_child(CONF_OUTPUT_SOURCE, var.set_output_source_sensor)
    await add_child(CONF_INPUT_FREQUENCY, var.set_input_frequency_sensor)
    await add_child(CONF_OUTPUT_FREQUENCY, var.set_output_frequency_sensor)
