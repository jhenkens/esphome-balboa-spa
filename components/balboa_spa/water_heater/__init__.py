import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import water_heater
from esphome.const import CONF_ID, CONF_UNIT_OF_MEASUREMENT, UNIT_CELSIUS, UNIT_FAHRENHEIT
from esphome.cpp_generator import AssignmentExpression

from .. import (
    CONF_SPA_ID,
    balboa_spa_ns,
    BalboaSpa,
)

DEPENDENCIES = ["balboa_spa", "water_heater"]
AUTO_LOAD = ["water_heater"]

BalboaSpaWaterHeater = balboa_spa_ns.class_("BalboaSpaWaterHeater", water_heater.WaterHeater)

CONFIG_SCHEMA = cv.ENTITY_BASE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BalboaSpaWaterHeater),
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_UNIT_OF_MEASUREMENT): cv.one_of(
            UNIT_CELSIUS, UNIT_FAHRENHEIT
        )
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await water_heater.register_water_heater(var, config)

    parent = await cg.get_variable(config[CONF_SPA_ID])
    cg.add(var.set_parent(parent))
    if unit := config.get(CONF_UNIT_OF_MEASUREMENT):
        if unit == UNIT_FAHRENHEIT:
            TEMP_SCALE = balboa_spa_ns.enum("TEMP_SCALE", is_class=True)
            cg.add(var.set_temp_scale(TEMP_SCALE.F))
