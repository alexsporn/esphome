from esphome.components import time
from esphome import automation
from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]
CODEOWNERS = ["@acvigue"]

CONF_LEVOIT_ID = "levoit_id"
levoit_ns = cg.esphome_ns.namespace("levoit")
Levoit = levoit_ns.class_("Levoit", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = (
    cv.Schema()
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
