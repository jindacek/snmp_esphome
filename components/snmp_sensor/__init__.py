import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, text_sensor
from esphome.automation import maybe_simple_id
from esphome.const import *
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL, ENTITY_CATEGORY_CONFIG, CONF_NAME
from esphome.core import coroutine_with_priority

from .component import snmp_ns, SnmpSensor

CONF_HOST = "host"
CONF_COMMUNITY = "community"

# Numeric sub-sensors
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

# Text sub-sensors
CONF_MODEL = "model"
CONF_NAME = "name"
CONF_MANUFACTURE_DATE = "manufacture_date"
CONF_LAST_BATTERY_REPLACEMENT = "last_battery_replacement"
CONF_LAST_SELF_TEST_DATE = "last_self_test_date"
CONF_SERIAL_NUMBER = "serial_number"
CONF_RUNTIME_FORMATTED = "runtime_formatted"
CONF_REMAINING_RUNTIME_FORMATTED = "remaining_runtime_formatted"
CONF_OUTPUT_STATUS_TEXT = "output_status_text"
CONF_OUTPUT_SOURCE_TEXT = "output_source_text"
CONF_BATTERY_REPLACE_TEXT = "battery_replace_text"
CONF_SELF_TEST_TEXT = "self_test_text"


def _num_schema(unit=None, icon=None):
    sch = sensor.sensor_schema()
    if unit is not None:
        sch = sch.extend({cv.Optional("unit_of_measurement", default=unit): cv.string})
    if icon is not None:
        sch = sch.extend({cv.Optional("icon", default=icon): cv.icon})
    return sch


def _txt_schema(icon=None):
    sch = text_sensor.text_sensor_schema()
    if icon is not None:
        sch = sch.extend({cv.Optional("icon", default=icon): cv.icon})
    return sch


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(SnmpSensor),
        cv.Required(CONF_HOST): cv.string,
        cv.Required(CONF_COMMUNITY): cv.string,
        cv.Optional(CONF_UPDATE_INTERVAL, default="10s"): cv.update_interval,

        # ---- numeric optional ----
        cv.Optional(CONF_BATTERY_VOLTAGE): _num_schema("V"),
        cv.Optional(CONF_INPUT_VOLTAGE): _num_schema("V"),
        cv.Optional(CONF_OUTPUT_VOLTAGE): _num_schema("V"),
        cv.Optional(CONF_LOAD): _num_schema("%"),
        cv.Optional(CONF_RUNTIME): _num_schema("s"),
        cv.Optional(CONF_REMAINING_RUNTIME): _num_schema("s"),
        cv.Optional(CONF_BATTERY_CAPACITY): _num_schema("%"),
        cv.Optional(CONF_BATTERY_TEMP): _num_schema("°C"),
        cv.Optional(CONF_INPUT_FREQUENCY): _num_schema("Hz"),
        cv.Optional(CONF_OUTPUT_FREQUENCY): _num_schema("Hz"),
        cv.Optional(CONF_OUTPUT_STATUS): _num_schema(),
        cv.Optional(CONF_OUTPUT_SOURCE): _num_schema(),
        cv.Optional(CONF_BATTERY_REPLACE_STATUS): _num_schema(),
        cv.Optional(CONF_SELF_TEST_RESULT): _num_schema(),

        # ---- text optional ----
        cv.Optional(CONF_MODEL): _txt_schema(),
        cv.Optional(CONF_NAME): _txt_schema(),
        cv.Optional(CONF_MANUFACTURE_DATE): _txt_schema(),
        cv.Optional(CONF_LAST_BATTERY_REPLACEMENT): _txt_schema(),
        cv.Optional(CONF_LAST_SELF_TEST_DATE): _txt_schema(),
        cv.Optional(CONF_SERIAL_NUMBER): _txt_schema(),
        cv.Optional(CONF_RUNTIME_FORMATTED): _txt_schema(),
        cv.Optional(CONF_REMAINING_RUNTIME_FORMATTED): _txt_schema(),
        cv.Optional(CONF_OUTPUT_STATUS_TEXT): _txt_schema(),
        cv.Optional(CONF_OUTPUT_SOURCE_TEXT): _txt_schema(),
        cv.Optional(CONF_BATTERY_REPLACE_TEXT): _txt_schema(),
        cv.Optional(CONF_SELF_TEST_TEXT): _txt_schema(),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_host(config[CONF_HOST]))
    cg.add(var.set_community(config[CONF_COMMUNITY]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    # numeric sensors
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

    # text sensors
    if CONF_MODEL in config:
        t = await text_sensor.new_text_sensor(config[CONF_MODEL])
        cg.add(var.set_model_text_sensor(t))

    if CONF_NAME in config:
        t = await text_sensor.new_text_sensor(config[CONF_NAME])
        cg.add(var.set_name_text_sensor(t))

    if CONF_MANUFACTURE_DATE in config:
        t = await text_sensor.new_text_sensor(config[CONF_MANUFACTURE_DATE])
        cg.add(var.set_manufacture_date_text_sensor(t))

    if CONF_LAST_BATTERY_REPLACEMENT in config:
        t = await text_sensor.new_text_sensor(config[CONF_LAST_BATTERY_REPLACEMENT])
        cg.add(var.set_last_battery_replacement_text_sensor(t))

    if CONF_LAST_SELF_TEST_DATE in config:
        t = await text_sensor.new_text_sensor(config[CONF_LAST_SELF_TEST_DATE])
        cg.add(var.set_last_self_test_text_sensor(t))

    if CONF_SERIAL_NUMBER in config:
        t = await text_sensor.new_text_sensor(config[CONF_SERIAL_NUMBER])
        cg.add(var.set_serial_number_text_sensor(t))

    if CONF_RUNTIME_FORMATTED in config:
        t = await text_sensor.new_text_sensor(config[CONF_RUNTIME_FORMATTED])
        cg.add(var.set_runtime_formatted_text_sensor(t))

    if CONF_REMAINING_RUNTIME_FORMATTED in config:
        t = await text_sensor.new_text_sensor(config[CONF_REMAINING_RUNTIME_FORMATTED])
        cg.add(var.set_remaining_runtime_formatted_text_sensor(t))

    if CONF_OUTPUT_STATUS_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_OUTPUT_STATUS_TEXT])
        cg.add(var.set_output_status_text_sensor(t))

    if CONF_OUTPUT_SOURCE_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_OUTPUT_SOURCE_TEXT])
        cg.add(var.set_output_source_text_sensor(t))

    if CONF_BATTERY_REPLACE_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_BATTERY_REPLACE_TEXT])
        cg.add(var.set_battery_replace_text_sensor(t))

    if CONF_SELF_TEST_TEXT in config:
        t = await text_sensor.new_text_sensor(config[CONF_SELF_TEST_TEXT])
        cg.add(var.set_self_test_text_sensor(t))
