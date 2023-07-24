from esphome.components import switch
import esphome.config_validation as cv
import esphome.codegen as cg
from .. import levoit_ns, CONF_LEVOIT_ID, Levoit

DEPENDENCIES = ["levoit"]
CODEOWNERS = ["@acvigue"]

CONF_PURPOSE = "purpose"

LevoitSwitch = levoit_ns.class_("LevoitSwitch", switch.Switch, cg.Component)

CONFIG_SCHEMA = (
    switch.switch_schema(LevoitSwitch)
    .extend(
        {
            cv.GenerateID(CONF_LEVOIT_ID): cv.use_id(Levoit),
            cv.Required(CONF_PURPOSE): cv.uint8_t,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
)


async def to_code(config):
    var = await switch.new_switch(config)
    await cg.register_component(var, config)

    paren = await cg.get_variable(config[CONF_LEVOIT_ID])
    cg.add(var.set_levoit_parent(paren))

    cg.add(var.set_purpose(config[CONF_PURPOSE]))
    await switch.register_switch(var, config)
