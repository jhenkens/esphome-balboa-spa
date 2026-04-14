import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ['uart']

CONF_SPA_ID = "balboa_spa_id"
CONF_CLIENT_ID = "client_id"
CONF_LISTENER_KEEPALIVE = "listener_keepalive"
CONF_STARTUP_DELAY = "startup_delay"
CONF_LIVE_RANGE_REFRESH = "live_range_refresh"

balboa_spa_ns = cg.esphome_ns.namespace('balboa_spa')
BalboaSpa = balboa_spa_ns.class_('BalboaSpa', cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BalboaSpa),
    cv.Optional(CONF_CLIENT_ID): cv.int_range(min=1, max=47),
    cv.Optional(CONF_LISTENER_KEEPALIVE, default=300000): cv.positive_int,
    cv.Optional(CONF_STARTUP_DELAY, default=10000): cv.positive_int,
    cv.Optional(CONF_LIVE_RANGE_REFRESH, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    if client_id_conf := config.get(CONF_CLIENT_ID):
        cg.add(var.set_client_id(client_id_conf))

    cg.add(var.set_listener_keepalive(config[CONF_LISTENER_KEEPALIVE]))
    cg.add(var.set_startup_delay(config[CONF_STARTUP_DELAY]))
    cg.add(var.set_live_range_refresh(config[CONF_LIVE_RANGE_REFRESH]))

    yield uart.register_uart_device(var, config)
