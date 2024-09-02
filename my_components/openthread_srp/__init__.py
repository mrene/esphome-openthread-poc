from esphome.const import CONF_ID
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.core import CORE

DEPENDENCIES = ['openthread']

openthread_srp_ns = cg.esphome_ns.namespace("openthread")
OpenThreadSRPComponent = openthread_srp_ns.class_("OpenThreadSRP",
                                                  cg.Component)


def _remove_id_if_disabled(value):
    value = value.copy()
    if value[CONF_DISABLED]:
        value.pop(CONF_ID)
    return value


CONF_DISABLED = "disabled"
CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(OpenThreadSRPComponent),
            cv.Optional(CONF_DISABLED, default=False): cv.boolean,
        }
    ),
    _remove_id_if_disabled,
)


async def to_code(config):
    if config[CONF_DISABLED]:
        return
    cg.add_define("USE_OT_SRP")

    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_host_name(cg.RawExpression(f'"{CORE.name}"')))
    await cg.register_component(var, config)
