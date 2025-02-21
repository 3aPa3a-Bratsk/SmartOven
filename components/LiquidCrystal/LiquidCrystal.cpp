#include "LiquidCrystal.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

LiquidCrystal::LiquidCrystal(uint8_t rs, uint8_t enable,
                             uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
  _rs_pin = rs;
  _enable_pin = enable;
  _data_pins[0] = d4;
  _data_pins[1] = d5;
  _data_pins[2] = d6;
  _data_pins[3] = d7;

  gpio_set_direction((gpio_num_t)_rs_pin, GPIO_MODE_OUTPUT);
  gpio_set_direction((gpio_num_t)_enable_pin, GPIO_MODE_OUTPUT);
  for (int i = 0; i < 4; i++) {
    gpio_set_direction((gpio_num_t)_data_pins[i], GPIO_MODE_OUTPUT);
  }
}

void LiquidCrystal::begin(uint8_t cols, uint8_t lines) {
  _displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
  _numlines = lines;

  // Initialize in 4-bit mode
  write4bits(0x03);
  vTaskDelay(pdMS_TO_TICKS(5));
  write4bits(0x03);
  vTaskDelay(pdMS_TO_TICKS(5));
  write4bits(0x03);
  vTaskDelay(pdMS_TO_TICKS(1));
  write4bits(0x02);

  // Set # lines, font size, etc.
  send(LCD_FUNCTIONSET | _displayfunction, 0);

  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
  send(LCD_DISPLAYCONTROL | _displaycontrol, 0);

  clear();

  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  send(LCD_ENTRYMODESET | _displaymode, 0);
}

void LiquidCrystal::clear() {
  send(LCD_CLEARDISPLAY, 0);
  vTaskDelay(pdMS_TO_TICKS(2));
}

void LiquidCrystal::setCursor(uint8_t col, uint8_t row) {
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if (row >= _numlines) {
    row = _numlines - 1;
  }
  send(LCD_SETDDRAMADDR | (col + row_offsets[row]), 0);
}

void LiquidCrystal::print(const char* str) {
  while (*str) {
    send(*str++, 1);
  }
}

void LiquidCrystal::print(int num) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%d", num);
  print(buf);
}

void LiquidCrystal::print(float num, int digits) {
  char buf[16];
  snprintf(buf, sizeof(buf), "%.*f", digits, num);
  print(buf);
}

void LiquidCrystal::send(uint8_t value, bool mode) {
  gpio_set_level((gpio_num_t)_rs_pin, mode);
  write4bits(value >> 4);
  write4bits(value);
}

void LiquidCrystal::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    gpio_set_level((gpio_num_t)_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}

void LiquidCrystal::pulseEnable() {
  gpio_set_level((gpio_num_t)_enable_pin, 0);
  vTaskDelay(pdMS_TO_TICKS(1));
  gpio_set_level((gpio_num_t)_enable_pin, 1);
  vTaskDelay(pdMS_TO_TICKS(1));
  gpio_set_level((gpio_num_t)_enable_pin, 0);
  vTaskDelay(pdMS_TO_TICKS(1));
}
