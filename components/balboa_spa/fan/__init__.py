import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan

from esphome.const import (
    CONF_ID
)

from .. import (
    balboa_spa_ns,
    BalboaSpa,
    CONF_SPA_ID
)

DEPENDENCIES = ["balboa_spa"]

JetFan = balboa_spa_ns.class_("JetFan", fan.Fan)

CONF_JET_1 = "jet_1"
CONF_JET_2 = "jet_2"
CONF_JET_3 = "jet_3"
CONF_JET_4 = "jet_4"
def jet_fan_schema():
    return fan.fan_schema(JetFan)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_JET_1): jet_fan_schema(),
        cv.Optional(CONF_JET_2): jet_fan_schema(),
        cv.Optional(CONF_JET_3): jet_fan_schema(),
        cv.Optional(CONF_JET_4): jet_fan_schema(),
    })

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])

    for jet_index, fan_type in enumerate([CONF_JET_1, CONF_JET_2, CONF_JET_3, CONF_JET_4], start=1):
        if conf := config.get(fan_type):
            fan_var = cg.new_Pvariable(conf[CONF_ID], jet_index)
            await fan.register_fan(fan_var, conf)
            cg.add(fan_var.set_parent(parent))
