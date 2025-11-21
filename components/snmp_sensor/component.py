import esphome.codegen as cg

snmp_ns = cg.esphome_ns.namespace("snmp_sensor")
SnmpSensor = snmp_ns.class_("SnmpSensor", cg.PollingComponent)
