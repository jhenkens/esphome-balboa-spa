import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from esphome.const import (
    CONF_ID,
    ICON_LIGHTBULB,
    ICON_GRAIN,
    ICON_THERMOMETER,
)

from .. import (
    balboa_spa_ns,
    BalboaSpa,
    CONF_SPA_ID
)

DEPENDENCIES = ["balboa_spa"]

JetSwitch = balboa_spa_ns.class_("JetSwitch", switch.Switch)
LightSwitch = balboa_spa_ns.class_("LightSwitch", switch.Switch)
BlowerSwitch = balboa_spa_ns.class_("BlowerSwitch", switch.Switch)
HighrangeSwitch = balboa_spa_ns.class_("HighrangeSwitch", switch.Switch)
Filter2Switch = balboa_spa_ns.class_("Filter2Switch", switch.Switch)
RestModeSwitch = balboa_spa_ns.class_("RestModeSwitch", switch.Switch)

CONF_JET1 = "jet1"
CONF_JET2 = "jet2"
CONF_JET3 = "jet3"
CONF_JET4 = "jet4"
CONF_LIGHTS = "light"
CONF_LIGHT2 = "light2"
CONF_BLOWER = "blower"
CONF_HIGHRANGE = "highrange"
CONF_FILTER2 = "filter2"
CONF_REST_MODE = "rest_mode"
CONF_ON_LEVEL = "on_level"

def jet_switch_schema(cls):
    return switch.switch_schema(
        cls,
        icon="mdi:pump",
        default_restore_mode="DISABLED",
    ).extend({
        cv.Optional(CONF_ON_LEVEL, default=1): cv.one_of(1, 2, int=True),
    })

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
        cv.Optional(CONF_JET1): jet_switch_schema(JetSwitch),
        cv.Optional(CONF_JET2): jet_switch_schema(JetSwitch),
        cv.Optional(CONF_JET3): jet_switch_schema(JetSwitch),
        cv.Optional(CONF_JET4): jet_switch_schema(JetSwitch),
        cv.Optional(CONF_LIGHTS): switch.switch_schema(
            LightSwitch,
            icon=ICON_LIGHTBULB,
            default_restore_mode="DISABLED",
        ),
        cv.Optional(CONF_LIGHT2): switch.switch_schema(
            LightSwitch,
            icon=ICON_LIGHTBULB,
            default_restore_mode="DISABLED",
        ),
        cv.Optional(CONF_BLOWER): switch.switch_schema(
            BlowerSwitch,
            icon=ICON_GRAIN,
            default_restore_mode="DISABLED",
        ),
        cv.Optional(CONF_HIGHRANGE): switch.switch_schema(
            HighrangeSwitch,
            icon=ICON_THERMOMETER,
            default_restore_mode="DISABLED",
        ),
        cv.Optional(CONF_FILTER2): switch.switch_schema(
            Filter2Switch,
            icon=ICON_GRAIN,
            default_restore_mode="DISABLED",
        ),
        cv.Optional(CONF_REST_MODE): switch.switch_schema(
            RestModeSwitch,
            icon=ICON_THERMOMETER,
            default_restore_mode="DISABLED",
        ),
    })

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])

    for jet_index, switch_type in enumerate([CONF_JET1, CONF_JET2, CONF_JET3, CONF_JET4], start=1):
        if conf := config.get(switch_type):
            sw_var = cg.new_Pvariable(conf[CONF_ID], jet_index)
            await switch.register_switch(sw_var, conf)
            cg.add(sw_var.set_parent(parent))
            if CONF_ON_LEVEL in conf:
                cg.add(sw_var.set_on_level(conf[CONF_ON_LEVEL]))

    for light_index, switch_type in enumerate([CONF_LIGHTS, CONF_LIGHT2], start=1):
        if conf := config.get(switch_type):
            sw_var = cg.new_Pvariable(conf[CONF_ID], light_index)
            await switch.register_switch(sw_var, conf)
            cg.add(sw_var.set_parent(parent))

    for switch_type, cls in [
        (CONF_BLOWER, BlowerSwitch),
        (CONF_HIGHRANGE, HighrangeSwitch),
        (CONF_FILTER2, Filter2Switch),
        (CONF_REST_MODE, RestModeSwitch),
    ]:
        if conf := config.get(switch_type):
            sw_var = await switch.new_switch(conf)
            cg.add(sw_var.set_parent(parent))
