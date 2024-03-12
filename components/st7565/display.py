from esphome import pins
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, spi
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_WIDTH,
    CONF_HEIGHT,
    CONF_CONTRAST,
    CONF_DC_PIN,
    CONF_RESET_PIN
)
from esphome.const import __version__ as ESPHOME_VERSION

AUTO_LOAD = ["display"]
DEPENDENCIES = ["spi"]

st7565_ns = cg.esphome_ns.namespace("st7565")
ST7565 = st7565_ns.class_(
    "ST7565", cg.PollingComponent, display.DisplayBuffer, spi.SPIDevice
)
ST7565Ref = ST7565.operator("ref")

CONFIG_SCHEMA = (
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ST7565),
            cv.Required(CONF_WIDTH): cv.int_,
            cv.Required(CONF_HEIGHT): cv.int_,
            cv.Required(CONF_DC_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_CONTRAST, default=0x10): cv.int_
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(spi.spi_device_schema())
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    if cv.Version.parse(ESPHOME_VERSION) < cv.Version.parse("2023.12.0"):
        await cg.register_component(var, config)
    await spi.register_spi_device(var, config)

    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayBufferRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    cg.add(var.set_contrast(config[CONF_CONTRAST]))

    dc = await cg.gpio_pin_expression(config[CONF_DC_PIN])
    cg.add(var.set_dc_pin(dc))

    reset = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(var.set_reset_pin(reset))

    await display.register_display(var, config)
