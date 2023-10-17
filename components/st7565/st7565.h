#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"

namespace esphome {
namespace st7565 {

class ST7565;

class ST7565 : public PollingComponent,
               public display::DisplayBuffer,
               public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_HIGH, spi::CLOCK_PHASE_TRAILING,
                                     spi::DATA_RATE_8MHZ> {
 public:
  static const uint8_t ST7565_SET_COLUMN_ADDRESS_UPPER_NIBBLE = 0x00;
  static const uint8_t ST7565_SET_COLUMN_ADDRESS_LOWER_NIBBLE = 0x10;
  static const uint8_t ST7565_SET_5V_REGULATOR_RESISTOR_RATIO = 0x20;
  static const uint8_t ST7565_SET_POWER_CONTROL = 0x28;
  static const uint8_t ST7565_SET_LINE_ONE_AS_START = 0x40;
  static const uint8_t ST7565_SET_BRIGHTNESS = 0x81;
  static const uint8_t ST7565_SET_ADC_TO_NORMAL = 0xA0;
  static const uint8_t ST7565_SET_ADC_TO_REVERSE = 0xA1;
  static const uint8_t ST7565_SET_LCD_BIAS_1_9 = 0xA2;
  static const uint8_t ST7565_SET_LCD_BIAS_1_7 = 0xA3;
  static const uint8_t ST7565_SET_ALL_PIXELS_OFF = 0xA4;
  static const uint8_t ST7565_SET_ALL_PIXELS_ON = 0xA5;
  static const uint8_t ST7565_SET_DISPLAY_NORMAL = 0xA6;
  static const uint8_t ST7565_SET_DISPLAY_REVERSE = 0xA7;
  static const uint8_t ST7565_DISABLE_CURSOR = 0xAC;
  static const uint8_t ST7565_DISPLAY_OFF = 0xAE;
  static const uint8_t ST7565_DISPLAY_ON = 0xAF;
  static const uint8_t ST7565_SET_PAGE_ADDRESS = 0xB0;
  static const uint8_t ST7565_SET_SCAN_DIR_NORMAL = 0xC0;
  static const uint8_t ST7565_SET_SCAN_DIR_REVERSE = 0xC8;
  static const uint8_t ST7565_SOFT_RESET = 0xE2;

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_GRAYSCALE; }

  void HOT display();
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  void update() override;
  void fill(Color color) override;

  void set_height(uint16_t height) { this->height_ = height; }
  void set_width(uint16_t width) { this->width_ = width; }
  void set_contrast(uint8_t contrast) { this->contrast_ = contrast; }
  void set_dc_pin(GPIOPin *dc_pin) { this->dc_pin_ = dc_pin; }
  void set_reset_pin(GPIOPin *reset) { this->reset_pin_ = reset; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_height_internal() override;
  int get_width_internal() override;
  size_t get_buffer_length_();
  void init_reset_();
  void display_init_();
  void command_(uint8_t value);
  void start_data_();
  void end_data_();
  void data_(uint8_t value);

  uint8_t contrast_ = 32>>2;
  int16_t width_ = 128, height_ = 64;
  GPIOPin *reset_pin_;
  GPIOPin *dc_pin_;
};

}  // namespace st7565
}  // namespace esphome
