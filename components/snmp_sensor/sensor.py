import esphome.codegen as cg
import esphome.config_validation as cv

from esphome.components import sensor, text_sensor
from esphome.const import (
    CONF_ID,
    CONF_HOST,
    CONF_UPDATE_INTERVAL,
)

from . import snmp_ns

SnmpSensor = snmp_ns.class_("SnmpSensor", cg.PollingComponent)

CONF_COMMUNITY = "community"

# ---- numeric keys ----
CONF_BATTERY_VOLTAGE = "battery_voltage"
CONF_INPUT_VOLTAGE = "input_voltage"
CONF_OUTPUT_VOLTAGE = "output_voltage"
CONF_LOAD = "load"
CONF_RUNTIME = "runtime"
CONF_REMAINING_RUNTIME = "remaining_runtime"
CONF_BATTERY_CAPACITY = "battery_capacity"
CONF_BATTERY_TEMP = "battery_temp"
CONF_INPUT_FREQUENCY = "input_frequency"
CONF_OUTPUT_FREQUENCY = "output_frequency"
CONF_OUTPUT_STATUS = "output_status"
CONF_OUTPUT_SOURCE = "output_source"
CONF_BATTERY_REPLACE_STATUS = "battery_replace_status"
CONF_SELF_TEST_RESULT = "self_test_result"

# ---- text keys ----
CONF_MODEL = "model"
CONF_UPS_NAME = "ups_name"
CONF_MANUFACTURE_DATE = "manufacture_date"
CONF_LAST_BATTERY_REPLACEMENT = "last_battery_replacement"
CONF_LAST_SELF_TEST_DATE = "last_self_test_date"
CONF_SERIAL_NUMBER = "serial_number"
CONF_OUTPUT_STATUS_TEXT = "output_status_text"
CONF_RUNTIME_FORMATTED = "runtime_formatted"
CONF_REMAINING_RUNTIME_FORMATTED = "remaining_runtime_formatted"
CONF_OUTPUT_SOURCE_TEXT = "output_source_text"
CONF_BATTERY_REPLACE_TEXT = "battery_replace_text"
CONF_SELF_TEST_TEXT = "self_test_text"

NUMERIC_SCHEMA = sensor.sensor_schema(
    accuracy_decimals=0
)

TEXT_SCHEMA = text_sensor.text_sensor_schema()

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SnmpSensor),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_COMMUNITY): cv.string,

        # ---- numeric optional ----
        cv.Optional(CONF_BATTERY_VOLTAGE): NUMERIC_SCHEMA,
        cv.Optional(CONF_INPUT_VOLTAGE): NUMERIC_SCHEMA,
        cv.Optional(CONF_OUTPUT_VOLTAGE): NUMERIC_SCHEMA,
        cv.Optional(CONF_LOAD): NUMERIC_SCHEMA,
        cv.Optional(CONF_RUNTIME): NUMERIC_SCHEMA,
        cv.Optional(CONF_REMAINING_RUNTIME): NUMERIC_SCHEMA,
        cv.Optional(CONF_BATTERY_CAPACITY): NUMERIC_SCHEMA,
        cv.Optional(CONF_BATTERY_TEMP): NUMERIC_SCHEMA,
        cv.Optional(CONF_INPUT_FREQUENCY): NUMERIC_SCHEMA,
        cv.Optional(CONF_OUTPUT_FREQUENCY): NUMERIC_SCHEMA,
        cv.Optional(CONF_OUTPUT_STATUS): NUMERIC_SCHEMA,
        cv.Optional(CONF_OUTPUT_SOURCE): NUMERIC_SCHEMA,
        cv.Optional(CONF_BATTERY_REPLACE_STATUS): NUMERIC_SCHEMA,
        cv.Optional(CONF_SELF_TEST_RESULT): NUMERIC_SCHEMA,

        # ---- text optional ----
        cv.Optional(CONF_MODEL): TEXT_SCHEMA,
        cv.Optional(CONF_UPS_NAME): TEXT_SCHEMA,
        cv.Optional(CONF_MANUFACTURE_DATE): TEXT_SCHEMA,
        cv.Optional(CONF_LAST_BATTERY_REPLACEMENT): TEXT_SCHEMA,
        cv.Optional(CONF_LAST_SELF_TEST_DATE): TEXT_SCHEMA,
        cv.Optional(CONF_SERIAL_NUMBER): TEXT_SCHEMA,
        cv.Optional(CONF_OUTPUT_STATUS_TEXT): TEXT_SCHEMA,
        cv.Optional(CONF_RUNTIME_FORMATTED): TEXT_SCHEMA,
        cv.Optional(CONF_REMAINING_RUNTIME_FORMATTED): TEXT_SCHEMA,
        cv.Optional(CONF_OUTPUT_SOURCE_TEXT): TEXT_SCHEMA,
        cv.Optional(CONF_BATTERY_REPLACE_TEXT): TEXT_SCHEMA,
        cv.Optional(CONF_SELF_TEST_TEXT): TEXT_SCHEMA,
    }
).extend(cv.polling_component_schema("10s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))

    # numeric
    if CONF_BATTERY_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_VOLTAGE])
        cg.add(var.set_battery_voltage_sensor(s))

    if CONF_INPUT_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_INPUT_VOLTAGE])
        cg.add(var.set_input_voltage_sensor(s))

    if CONF_OUTPUT_VOLTAGE in config:
        s = await sensor.new_sensor(config[CONF_OUTPUT_VOLTAGE])
        cg.add(var.set_output_voltage_sensor(s))

    if CONF_LOAD in config:
        s = await sensor.new_sensor(config[CONF_LOAD])
        cg.add(var.set_load_sensor(s))

    if CONF_RUNTIME in config:
        s = await sensor.new_sensor(config[CONF_RUNTIME])
        cg.add(var.set_runtime_sensor(s))

    if CONF_REMAINING_RUNTIME in config:
        s = await sensor.new_sensor(config[CONF_REMAINING_RUNTIME])
        cg.add(var.set_remaining_runtime_sensor(s))

    if CONF_BATTERY_CAPACITY in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_CAPACITY])
        cg.add(var.set_battery_capacity_sensor(s))

    if CONF_BATTERY_TEMP in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_TEMP])
        cg.add(var.set_battery_temp_sensor(s))

    if CONF_INPUT_FREQUENCY in config:
        s = await sensor.new_sensor(config[CONF_INPUT_FREQUENCY])
        cg.add(var.set_input_frequency_sensor(s))

    if CONF_OUTPUT_FREQUENCY in config:
        s = await sensor.new_sensor(config[CONF_OUTPUT_FREQUENCY])
        cg.add(var.set_output_frequency_sensor(s))

    if CONF_OUTPUT_STATUS in config:
        s = await sensor.new_sensor(config[CONF_OUTPUT_STATUS])
        cg.add(var.set_output_status_sensor(s))

    if CONF_OUTPUT_SOURCE in config:
        s = await sensor.new_sensor(config[CONF_OUTPUT_SOURCE])
        cg.add(var.set_output_source_sensor(s))

    if CONF_BATTERY_REPLACE_STATUS in config:
        s = await sensor.new_sensor(config[CONF_BATTERY_REPLACE_STATUS])
        cg.add(var.set_battery_replace_status_sensor(s))

    if CONF_SELF_TEST_RESULT in config:
        s = await sensor.new_sensor(config[CONF_SELF_TEST_RESULT])
        cg.add(var.set_self_test_result_sensor(s))

    # text
    if CONF_MODEL in config:
        ts = await text_sensor.new_text_sensor(config[CONF_MODEL])
        cg.add(var.set_model_text_sensor(ts))

    if CONF_UPS_NAME in config:
        ts = await text_sensor.new_text_sensor(config[CONF_UPS_NAME])
        cg.add(var.set_name_text_sensor(ts))

    if CONF_MANUFACTURE_DATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_MANUFACTURE_DATE])
        cg.add(var.set_manufacture_date_text_sensor(ts))

    if CONF_LAST_BATTERY_REPLACEMENT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_LAST_BATTERY_REPLACEMENT])
        cg.add(var.set_last_battery_replacement_text_sensor(ts))

    if CONF_LAST_SELF_TEST_DATE in config:
        ts = await text_sensor.new_text_sensor(config[CONF_LAST_SELF_TEST_DATE])
        cg.add(var.set_last_self_test_text_sensor(ts))

    if CONF_SERIAL_NUMBER in config:
        ts = await text_sensor.new_text_sensor(config[CONF_SERIAL_NUMBER])
        cg.add(var.set_serial_number_text_sensor(ts))

    if CONF_OUTPUT_STATUS_TEXT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_OUTPUT_STATUS_TEXT])
        cg.add(var.set_output_status_text_sensor(ts))

    if CONF_RUNTIME_FORMATTED in config:
        ts = await text_sensor.new_text_sensor(config[CONF_RUNTIME_FORMATTED])
        cg.add(var.set_runtime_formatted_text_sensor(ts))

    if CONF_REMAINING_RUNTIME_FORMATTED in config:
        ts = await text_sensor.new_text_sensor(config[CONF_REMAINING_RUNTIME_FORMATTED])
        cg.add(var.set_remaining_runtime_formatted_text_sensor(ts))

    if CONF_OUTPUT_SOURCE_TEXT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_OUTPUT_SOURCE_TEXT])
        cg.add(var.set_output_source_text_sensor(ts))

    if CONF_BATTERY_REPLACE_TEXT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_BATTERY_REPLACE_TEXT])
        cg.add(var.set_battery_replace_text_sensor(ts))

    if CONF_SELF_TEST_TEXT in config:
        ts = await text_sensor.new_text_sensor(config[CONF_SELF_TEST_TEXT])
        cg.add(var.set_self_test_text_sensor(ts))
