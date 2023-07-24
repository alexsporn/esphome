from esphome.components import switch
import esphome.config_validation as cv
import esphome.codegen as cg
from esphome.const import ICON_SECURITY, DEVICE_CLASS_LOCK
from .. import levoit_ns, CONF_LEVOIT_ID, Levoit

DEPENDENCIES = ["levoit"]
CODEOWNERS = ["@acvigue"]

CONF_DISPLAY_LOCK = "display_lock"

LevoitSwitch = levoit_ns.class_("LevoitSwitch", switch.Switch, cg.Component)
LevoitSwitchPurpose = levoit_ns.enum("LevoitSwitchPurpose")

CONFIG_SCHEMA = (
    cv.Schema({
        cv.GenerateID(CONF_LEVOIT_ID): cv.use_id(Levoit),
        cv.Optional(CONF_DISPLAY_LOCK): switch.switch_schema(LevoitSwitch, icon=ICON_SECURITY).extend(cv.COMPONENT_SCHEMA),
    })
)


async def to_code(config):
    parent = await cg.get_variable(config[CONF_LEVOIT_ID])
    if config[CONF_DISPLAY_LOCK]:
        var = await switch.new_switch(config[CONF_DISPLAY_LOCK], parent, LevoitSwitchPurpose.DISPLAY_LOCK)
        await cg.register_component(var, config[CONF_DISPLAY_LOCK])
