import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID, CONF_UNIT_OF_MEASUREMENT, UNIT_CELSIUS, UNIT_FAHRENHEIT, CONF_VISUAL
from esphome.cpp_generator import AssignmentExpression

from .. import (
    CONF_SPA_ID,
    balboa_spa_ns,
    BalboaSpa,
)

DEPENDENCIES = ["balboa_spa", "climate"]
AUTO_LOAD = ["climate"]

BalboaSpaThermostat = balboa_spa_ns.class_('BalboaSpaThermostat', cg.Component, climate.Climate)

CONFIG_SCHEMA = cv.ENTITY_BASE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(BalboaSpaThermostat),
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_UNIT_OF_MEASUREMENT): cv.one_of(
            UNIT_CELSIUS, UNIT_FAHRENHEIT
        )
    }
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    await climate.register_climate(var, config)

    parent = await cg.get_variable(config[CONF_SPA_ID])
    cg.add(var.set_parent(parent))
    if unit := config.get(CONF_UNIT_OF_MEASUREMENT):
        if unit == UNIT_FAHRENHEIT:
            TEMP_SCALE = balboa_spa_ns.enum("TEMP_SCALE", is_class=True)
            cg.add(var.set_temp_scale(TEMP_SCALE.F))
