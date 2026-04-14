import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor

from .. import (
    balboa_spa_ns,
    BalboaSpa,
    CONF_SPA_ID
)

DEPENDENCIES = ["balboa_spa"]

SpaSensor = balboa_spa_ns.class_("BalboaSpaSensors", sensor.Sensor)
SpaSensorTypeEnum = SpaSensor.enum("BalboaSpaSensorType", True)

SpaFaultLogSensor = balboa_spa_ns.class_("BalboaSpaFaultLogSensors", sensor.Sensor)
SpaFaultLogSensorTypeEnum = SpaFaultLogSensor.enum("BalboaSpaFaultLogSensorType", True)

CONF_BLOWER = "blower"
CONF_HIGHRANGE = "highrange"
CONF_CIRCULATION = "circulation"
CONF_RESTMODE = "restmode"
CONF_HEATSTATE = "heatstate"
CONF_CURRENT_TEMP = "current_temp"
CONF_TARGET_TEMP = "target_temp"
CONF_SPA_TEMP_SCALE = "spa_temp_scale"
CONF_TIME_SINCE_LAST_STATUS = "time_since_last_status"
CONF_FAULT_CODE = "fault_code"
CONF_FAULT_TOTAL_ENTRIES = "fault_total_entries"
CONF_FAULT_CURRENT_ENTRY = "fault_current_entry"
CONF_FAULT_DAYS_AGO = "fault_days_ago"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_BLOWER): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_HIGHRANGE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_CIRCULATION): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_RESTMODE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_HEATSTATE): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_CURRENT_TEMP): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_TARGET_TEMP): sensor.sensor_schema(
            SpaSensor,
        ),
        cv.Optional(CONF_SPA_TEMP_SCALE): sensor.sensor_schema(
            SpaSensor,
            icon="mdi:thermometer-lines",
            entity_category="diagnostic",
        ),
        cv.Optional(CONF_TIME_SINCE_LAST_STATUS): sensor.sensor_schema(
            SpaSensor,
            icon="mdi:timer-outline",
            unit_of_measurement="s",
            entity_category="diagnostic",
        ),
        cv.Optional(CONF_FAULT_CODE): sensor.sensor_schema(
            SpaFaultLogSensor,
        ),
        cv.Optional(CONF_FAULT_TOTAL_ENTRIES): sensor.sensor_schema(
            SpaFaultLogSensor,
        ),
        cv.Optional(CONF_FAULT_CURRENT_ENTRY): sensor.sensor_schema(
            SpaFaultLogSensor,
        ),
        cv.Optional(CONF_FAULT_DAYS_AGO): sensor.sensor_schema(
            SpaFaultLogSensor,
        ),
    })

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])

    for sensor_type in [CONF_BLOWER, CONF_HIGHRANGE, CONF_CIRCULATION, CONF_RESTMODE, CONF_HEATSTATE, CONF_CURRENT_TEMP, CONF_TARGET_TEMP, CONF_SPA_TEMP_SCALE, CONF_TIME_SINCE_LAST_STATUS]:
        if conf := config.get(sensor_type):
            var = await sensor.new_sensor(conf)
            cg.add(var.set_parent(parent))
            sensor_type_value = getattr(SpaSensorTypeEnum, sensor_type.upper())
            cg.add(var.set_sensor_type(sensor_type_value))
    
    for sensor_type in [CONF_FAULT_CODE, CONF_FAULT_TOTAL_ENTRIES, CONF_FAULT_CURRENT_ENTRY, CONF_FAULT_DAYS_AGO]:
        if conf := config.get(sensor_type):
            var = await sensor.new_sensor(conf)
            cg.add(var.set_parent(parent))
            sensor_type_value = getattr(SpaFaultLogSensorTypeEnum, sensor_type.upper())
            cg.add(var.set_sensor_type(sensor_type_value))
