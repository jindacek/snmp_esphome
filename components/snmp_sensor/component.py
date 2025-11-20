import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_HOST, CONF_PORT, CONF_UPDATE_INTERVAL

DEPENDENCIES = ['network']
AUTO_LOAD = ['sensor']

# Namespace pro SNMP
snmp_ns = cg.esphome_ns.namespace('snmp')

# 1. SNMP Sensor (původní - pro jednotlivé senzory)
SNMPSensor = snmp_ns.class_('SNMPSensor', sensor.Sensor, cg.PollingComponent)

# 2. SNMP Multi Component (nový - pro více senzorů najednou)
SNMPMultiComponent = snmp_ns.class_('SNMPMultiComponent', cg.PollingComponent)

# Konfigurace pro jednotlivé SNMP senzory (původní)
CONF_COMMUNITY = 'community'
CONF_OID = 'oid'

SNMP_SENSOR_SCHEMA = sensor.SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(SNMPSensor),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_COMMUNITY): cv.string,
    cv.Required(CONF_OID): cv.string,
    cv.Optional(CONF_PORT, default=161): cv.port,
}).extend(cv.polling_component_schema('60s'))

# Konfigurace pro SNMP Multi Component (nový)
CONF_SNMP_SENSOR = 'snmp_sensor'
CONF_SENSORS = 'sensors'
CONF_VOLTAGE_OID = 'voltage_oid'
CONF_CAPACITY_OID = 'capacity_oid'
CONF_RUNTIME_OID = 'runtime_oid'
CONF_LOAD_OID = 'load_oid'
# Přidejte další OID podle potřeby...

SNMP_MULTI_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(SNMPMultiComponent),
    cv.Required(CONF_HOST): cv.string,
    cv.Required(CONF_COMMUNITY): cv.string,
    cv.Optional(CONF_PORT, default=161): cv.port,
    cv.Optional(CONF_UPDATE_INTERVAL, default='60s'): cv.update_interval,
    # Definice OID pro různé senzory
    cv.Optional(CONF_VOLTAGE_OID, default="1.3.6.1.4.1.318.1.1.1.3.2.1.0"): cv.string,
    cv.Optional(CONF_CAPACITY_OID, default="1.3.6.1.4.1.318.1.1.1.2.2.1.0"): cv.string,
    cv.Optional(CONF_RUNTIME_OID, default="1.3.6.1.4.1.318.1.1.1.2.2.3.0"): cv.string,
    cv.Optional(CONF_LOAD_OID, default="1.3.6.1.4.1.318.1.1.1.4.2.3.0"): cv.string,
    # Přidejte další OID podle potřeby...
}).extend(cv.polling_component_schema('60s'))

# Registrace obou konfigurací
CONFIG_SCHEMA = cv.typed_schema({
    'sensor': SNMP_SENSOR_SCHEMA,
    'snmp_sensor': SNMP_MULTI_SCHEMA,
}, lower=True)

def to_code(config):
    if config[CONF_TYPE] == 'sensor':
        # Původní kód pro jednotlivé senzory
        var = cg.new_Pvariable(config[CONF_ID])
        yield cg.register_component(var, config)
        yield sensor.register_sensor(var, config)
        
        cg.add(var.set_host(config[CONF_HOST]))
        cg.add(var.set_community(config[CONF_COMMUNITY]))
        cg.add(var.set_oid(config[CONF_OID]))
        cg.add(var.set_port(config[CONF_PORT]))
    
    elif config[CONF_TYPE] == 'snmp_sensor':
        # Nový kód pro multi komponentu
        var = cg.new_Pvariable(config[CONF_ID])
        yield cg.register_component(var, config)
        
        cg.add(var.set_host(config[CONF_HOST]))
        cg.add(var.set_community(config[CONF_COMMUNITY]))
        cg.add(var.set_port(config[CONF_PORT]))
        
        # Nastavení OID
        cg.add(var.set_voltage_oid(config[CONF_VOLTAGE_OID]))
        cg.add(var.set_capacity_oid(config[CONF_CAPACITY_OID]))
        cg.add(var.set_runtime_oid(config[CONF_RUNTIME_OID]))
        cg.add(var.set_load_oid(config[CONF_LOAD_OID]))
        # Přidejte další OID podle potřeby...
