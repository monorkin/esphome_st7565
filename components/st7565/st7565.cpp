#include "st7565.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/display/display_buffer.h"

namespace esphome {
namespace st7565 {

static const char *const TAG = "st7565";

virtual DisplayType get_display_type() const override { return DisplayType::DISPLAY_TYPE_MONO; }

void ST7565::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ST7565...");
  this->dump_config();
  this->spi_setup();
  this->init_reset_();
  this->dc_pin_->setup();
  this->init_internal_(this->get_buffer_length_());
  display_init_();
}

void ST7565::init_reset_() {
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(true);
    delay(1);
    // Trigger Reset
    this->reset_pin_->digital_write(false);
    delay(1);
    // Wake up
    this->reset_pin_->digital_write(true);
  }
}

void ST7565::command_(uint8_t value) {
  this->dc_pin_->digital_write(false);
  this->enable();
  this->write_byte(value);
  this->disable();
}

void ST7565::start_data_() {
  this->dc_pin_->digital_write(true);
  this->enable();
}

void ST7565::end_data_() {
  this->disable();
}

void ST7565::data_(uint8_t value) {
  this->start_data_();
  this->write_byte(value);
  this->end_data_();
}

void HOT ST7565::display() {
  uint8_t col, maxcol, page;

  for (page = 0; page < 8; page++) {
    this->command_(this->ST7565_SET_PAGE_ADDRESS | page);

    // start at the beginning of the row
    col = 0;
    maxcol = this->get_width_internal() - 1;

    this->command_(this->ST7565_SET_COLUMN_ADDRESS_UPPER_NIBBLE | (col & 0xF0));
    this->command_(this->ST7565_SET_COLUMN_ADDRESS_LOWER_NIBBLE | (col & 0x0F));

    this->start_data_();
    for (; col <= maxcol; col++) {
      this->write_byte(this->buffer_[(this->get_width_internal() * page) + col]);
    }
    this->end_data_();
  }

  // Reset the page position to 0
  this->command_(this->ST7565_SET_PAGE_ADDRESS);
}

void ST7565::fill(Color color) {
  memset(this->buffer_, color.is_on() ? 0xFF : 0x00, this->get_buffer_length_());
}

void ST7565::dump_config() {
  LOG_DISPLAY("", "ST7565", this);
  LOG_PIN("  CS Pin: ", this->cs_);
  LOG_PIN("  DC Pin: ", this->dc_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  Height: %d", this->height_);
  ESP_LOGCONFIG(TAG, "  Width: %d", this->width_);
}

void ST7565::update() {
  this->do_update_();
  this->display();
}

int ST7565::get_width_internal() { return this->width_; }

int ST7565::get_height_internal() { return this->height_; }

size_t ST7565::get_buffer_length_() {
  return size_t(this->get_width_internal()) * size_t(this->get_height_internal()) / 8u;
}

void HOT ST7565::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x >= this->get_width_internal() || x < 0 || y >= this->get_height_internal() || y < 0) {
    ESP_LOGW(TAG, "Position out of area: %dx%d", x, y);
    return;
  }

  uint16_t pos = x + (y / 8) * this->get_width_internal();
  uint8_t subpos = y % 8;

  if (color.is_on()) {
    this->buffer_[pos] |= (1 << subpos);
  } else {
    this->buffer_[pos] &= ~(1 << subpos);
  }
}

void ST7565::display_init_() {
  ESP_LOGD(TAG, "Initializing display...");

  this->command_(this->ST7565_SOFT_RESET);
  this->command_(this->ST7565_DISPLAY_OFF);
  this->command_(this->ST7565_SET_SCAN_DIR_REVERSE);
  this->command_(this->ST7565_SET_DISPLAY_NORMAL);
  this->command_(this->ST7565_SET_LCD_BIAS_1_7);

  // Disable the cursor (we will only ever use graphics mode)
  this->command_(this->ST7565_DISABLE_CURSOR);
  this->command_(0x00);

  // Enable boost circuit, voltage regulator & voltage follower one by one
  this->command_(this->ST7565_SET_POWER_CONTROL | 0x04);
  delay(50);
  this->command_(this->ST7565_SET_POWER_CONTROL | 0x06);
  delay(50);
  this->command_(this->ST7565_SET_POWER_CONTROL | 0x07);
  delay(10);

  // Set the 5V regulator resistor ratio to the maximum value
  this->command_(this->ST7565_SET_5V_REGULATOR_RESISTOR_RATIO | 0x06);

  // Set the brightness/contrast of the display
  this->command_(this->ST7565_SET_BRIGHTNESS);
  this->command_(this->contrast_);

  this->command_(this->ST7565_DISPLAY_ON);
  // this->command_(this->ST7565_SET_ALL_PIXELS_ON);
}

}  // namespace st7565
}  // namespace esphome
