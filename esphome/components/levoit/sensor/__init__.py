import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_OUTPUT_ID
)

from .. import levoit_ns, CONF_LEVOIT_ID, Levoit

DEPENDENCIES = ["levoit"]
CODEOWNERS = ["@acvigue"]

CONF_PURPOSE = "purpose"

LevoitSensor = levoit_ns.class_("LevoitSensor", cg.Component, sensor.Sensor)

CONFIG_SCHEMA = sensor.SENSOR_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(LevoitSensor),
        cv.GenerateID(CONF_LEVOIT_ID): cv.use_id(Levoit),
        cv.Required(CONF_PURPOSE): cv.uint8_t,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LEVOIT_ID])

    var = cg.new_Pvariable(config[CONF_OUTPUT_ID], parent)
    cg.add(var.set_purpose(config[CONF_PURPOSE]))
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
