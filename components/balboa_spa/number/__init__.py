import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_MIN_VALUE, CONF_MAX_VALUE, CONF_STEP
from .. import balboa_spa_ns, BalboaSpa, CONF_SPA_ID

DEPENDENCIES = ["balboa_spa"]

SpaTargetTempNumber = balboa_spa_ns.class_("SpaTargetTempNumber", number.Number, cg.Component)

CONF_TARGET_TEMP = "target_temp"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
    cv.Optional(CONF_TARGET_TEMP): number.number_schema(SpaTargetTempNumber).extend({
        cv.Optional(CONF_MIN_VALUE, default=7.0): cv.float_,
        cv.Optional(CONF_MAX_VALUE, default=40.0): cv.float_,
        cv.Optional(CONF_STEP, default=0.5): cv.positive_float,
    }),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])
    if conf := config.get(CONF_TARGET_TEMP):
        var = await number.new_number(
            conf,
            min_value=conf[CONF_MIN_VALUE],
            max_value=conf[CONF_MAX_VALUE],
            step=conf[CONF_STEP],
        )
        await cg.register_component(var, conf)
        cg.add(var.set_parent(parent))
