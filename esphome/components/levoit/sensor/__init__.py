import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_OUTPUT_ID,
    CONF_PM_2_5,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    ICON_BLUR,
    DEVICE_CLASS_PM25,
    STATE_CLASS_MEASUREMENT
)

from .. import levoit_ns, CONF_LEVOIT_ID, Levoit

DEPENDENCIES = ["levoit"]
CODEOWNERS = ["@acvigue"]

CONF_PURPOSE = "purpose"

LevoitSensor = levoit_ns.class_("LevoitSensor", cg.Component, sensor.Sensor)
LevoitSensorPurpose = levoit_ns.enum("LevoitSensorPurpose")

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(CONF_LEVOIT_ID): cv.use_id(Levoit),
        cv.Optional(CONF_PM_2_5): sensor.sensor_schema(LevoitSensor, unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER, icon=ICON_BLUR, accuracy_decimals=2, device_class=DEVICE_CLASS_PM25, state_class=STATE_CLASS_MEASUREMENT)
    })
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LEVOIT_ID])

    if config[CONF_PM_2_5]:
        var = await sensor.new_sensor(config[CONF_PM_2_5], parent, LevoitSensorPurpose.PM25)
        await cg.register_component(var, config[CONF_PM_2_5])
