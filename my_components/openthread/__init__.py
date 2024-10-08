# from esphome.components.zephyr import ZEPHYR_CORE_KEY
from esphome.const import (KEY_CORE, KEY_TARGET_PLATFORM, CONF_ID, CONF_MAC_ADDRESS)

from esphome.core import CORE, EsphomeError, coroutine_with_priority
import esphome.config_validation as cv
import esphome.codegen as cg

from esphome.components.esp32 import add_idf_sdkconfig_option, const, get_esp32_variant, add_idf_component

AUTO_LOAD = ["network"] #"openthread_srp"

# TODO: Doesn't conflict with wifi if you're using another ESP as an RCP (radio coprocessor)
CONFLICTS_WITH = ["wifi"]
DEPENDENCIES = ["esp32"]

CONF_NETWORK_NAME = "network_name"
CONF_CHANNEL = "channel"
CONF_NETWORK_KEY = "network_key"
CONF_PSKC = "pskc"
CONF_PANID = "panid"
CONF_EXTPANID = "extpanid"


def set_core_data(config):

    if not (CORE.is_esp32 and CORE.using_esp_idf):
        raise cv.Invalid("OpenThread is only supported on ESP32 with ESP-IDF")
    
    # TODO: Check that the board supports 802.15.4
    # and expose options for using SPI/UART RCPs
    add_idf_sdkconfig_option("CONFIG_IEEE802154_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_RADIO_NATIVE", True)

    # TODO: Report openthread status    
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_NAME", f'{config[CONF_NETWORK_NAME]}')
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_CHANNEL", config[CONF_CHANNEL])
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_PANID", config[CONF_PANID])
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_EXTPANID", f'{config[CONF_EXTPANID]}')
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_MASTERKEY", f'{config[CONF_NETWORK_KEY]}')
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_NETWORK_PSKC", f'{config[CONF_PSKC]}')
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_DNS64_CLIENT", True)
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_SRP_CLIENT", True);
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_SRP_CLIENT_MAX_SERVICES", 5);
    add_idf_sdkconfig_option("CONFIG_OPENTHREAD_FTD", True) # Full Thread Device
    

openthread_ns = cg.esphome_ns.namespace("openthread")
OpenThreadComponent = openthread_ns.class_("OpenThreadComponent", cg.Component)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(OpenThreadComponent),
            cv.Required(CONF_NETWORK_NAME): cv.string_strict,
            cv.Required(CONF_CHANNEL): cv.int_,
            cv.Required(CONF_NETWORK_KEY): cv.string_strict,
            cv.Required(CONF_PSKC): cv.string_strict,
            cv.Required(CONF_PANID): cv.int_,
            cv.Required(CONF_EXTPANID): cv.string_strict,
        }
    ),
)

@coroutine_with_priority(60.0)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_host_name(cg.RawExpression(f'"{CORE.name}"')))
    cg.add(var.set_mac(cg.RawExpression('get_mac_address()')))
    await cg.register_component(var, config)

    cg.add_global(cg.RawStatement('#include "esp_openthread.h"'))
    cg.add_global(cg.RawStatement('#include "esp_openthread_lock.h"'))
    cg.add_global(cg.RawStatement('#include "esp_task_wdt.h"'))
    cg.add_global(cg.RawStatement('#include "esp_task_wdt.h"'))
    cg.add_global(cg.RawStatement('#include <openthread/thread.h>'))
    

    set_core_data(config)

