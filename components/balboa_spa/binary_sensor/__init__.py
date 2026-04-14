import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor

from .. import (
    balboa_spa_ns,
    BalboaSpa,
    CONF_SPA_ID
)

from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_PROBLEM,
    ENTITY_CATEGORY_DIAGNOSTIC,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_RUNNING
)

DEPENDENCIES = ["balboa_spa"]

SpaSensor = balboa_spa_ns.class_("BalboaSpaBinarySensors", binary_sensor.BinarySensor)
SpaSensorTypeEnum = SpaSensor.enum("BalboaSpaBinarySensorType", True)

CONF_BLOWER = "blower"
CONF_HIGHRANGE = "highrange"
CONF_CIRCULATION = "circulation"
CONF_RESTMODE = "restmode"
CONF_HEATSTATE = "heatstate"
CONF_CONNECTED = "connected"
CONF_FILTER1_WINDOW_ACTIVE = "filter1_window_active"
CONF_FILTER2_WINDOW_ACTIVE = "filter2_window_active"
CONF_CLEANUP_CYCLE = "cleanup_cycle"
CONF_TIME_SYNCED = "time_synced"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_BLOWER): binary_sensor.binary_sensor_schema(
            SpaSensor,
            device_class=DEVICE_CLASS_POWER,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_HIGHRANGE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CIRCULATION): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:pump",
            device_class=DEVICE_CLASS_RUNNING,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_RESTMODE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_HEATSTATE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            device_class=DEVICE_CLASS_POWER,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:lan-connect",
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_FILTER1_WINDOW_ACTIVE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:air-filter",
            device_class=DEVICE_CLASS_RUNNING,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_FILTER2_WINDOW_ACTIVE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:air-filter",
            device_class=DEVICE_CLASS_RUNNING,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_CLEANUP_CYCLE): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:vacuum",
            device_class=DEVICE_CLASS_RUNNING,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
        cv.Optional(CONF_TIME_SYNCED): binary_sensor.binary_sensor_schema(
            SpaSensor,
            icon="mdi:clock-check",
            device_class=DEVICE_CLASS_PROBLEM,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC
        ),
    })

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])

    sensor_types = [
        CONF_BLOWER, CONF_HIGHRANGE, CONF_CIRCULATION, CONF_RESTMODE,
        CONF_HEATSTATE, CONF_CONNECTED, CONF_FILTER1_WINDOW_ACTIVE,
        CONF_FILTER2_WINDOW_ACTIVE, CONF_CLEANUP_CYCLE, CONF_TIME_SYNCED
    ]
    for sensor_type in sensor_types:
        if conf := config.get(sensor_type):
            var = await binary_sensor.new_binary_sensor(conf)
            cg.add(var.set_parent(parent))
            sensor_type_value = getattr(SpaSensorTypeEnum, sensor_type.upper())
            cg.add(var.set_sensor_type(sensor_type_value))
