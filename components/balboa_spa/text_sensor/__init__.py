import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from .. import balboa_spa_ns, BalboaSpa, CONF_SPA_ID

DEPENDENCIES = ["balboa_spa"]
SpaTimeTextSensor = balboa_spa_ns.class_("SpaTimeTextSensor", text_sensor.TextSensor)
SpaFilter1ConfigTextSensor = balboa_spa_ns.class_("SpaFilter1ConfigTextSensor", text_sensor.TextSensor)
SpaFilter2ConfigTextSensor = balboa_spa_ns.class_("SpaFilter2ConfigTextSensor", text_sensor.TextSensor)
FaultMessageTextSensor = balboa_spa_ns.class_("FaultMessageTextSensor", text_sensor.TextSensor)
FaultLogTimeTextSensor = balboa_spa_ns.class_("FaultLogTimeTextSensor", text_sensor.TextSensor)
ReminderTextSensor = balboa_spa_ns.class_("ReminderTextSensor", text_sensor.TextSensor)
ComponentVersionTextSensor = balboa_spa_ns.class_("ComponentVersionTextSensor", text_sensor.TextSensor)
ClientIdTextSensor = balboa_spa_ns.class_("ClientIdTextSensor", text_sensor.TextSensor)

CONF_SPA_TIME = "spa_time"
CONF_FILTER1_CONFIG = "filter1_config"
CONF_FILTER2_CONFIG = "filter2_config"
CONF_FAULT_MESSAGE = "fault_message"
CONF_FAULT_LOG_TIME = "fault_log_time"
CONF_REMINDER = "reminder"
CONF_COMPONENT_VERSION = "component_version"
CONF_CLIENT_ID = "client_id"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_SPA_ID): cv.use_id(BalboaSpa),
    cv.Optional(CONF_SPA_TIME): text_sensor.text_sensor_schema(SpaTimeTextSensor, icon="mdi:clock"),
    cv.Optional(CONF_FILTER1_CONFIG): text_sensor.text_sensor_schema(SpaFilter1ConfigTextSensor, icon="mdi:air-filter"),
    cv.Optional(CONF_FILTER2_CONFIG): text_sensor.text_sensor_schema(SpaFilter2ConfigTextSensor, icon="mdi:air-filter"),
    cv.Optional(CONF_FAULT_MESSAGE): text_sensor.text_sensor_schema(FaultMessageTextSensor, icon="mdi:alert-circle"),
    cv.Optional(CONF_FAULT_LOG_TIME): text_sensor.text_sensor_schema(FaultLogTimeTextSensor, icon="mdi:calendar-clock"),
    cv.Optional(CONF_REMINDER): text_sensor.text_sensor_schema(ReminderTextSensor, icon="mdi:bell"),
    cv.Optional(CONF_COMPONENT_VERSION): text_sensor.text_sensor_schema(ComponentVersionTextSensor, icon="mdi:information"),
    cv.Optional(CONF_CLIENT_ID): text_sensor.text_sensor_schema(ClientIdTextSensor, icon="mdi:account"),
})

async def to_code(config):
    parent = await cg.get_variable(config[CONF_SPA_ID])
    if conf := config.get(CONF_SPA_TIME):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_FILTER1_CONFIG):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_FILTER2_CONFIG):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_FAULT_MESSAGE):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_FAULT_LOG_TIME):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_REMINDER):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_COMPONENT_VERSION):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
    if conf := config.get(CONF_CLIENT_ID):
        var = await text_sensor.new_text_sensor(conf)
        cg.add(var.set_parent(parent))
