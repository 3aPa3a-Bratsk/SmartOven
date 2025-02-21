#ifndef LiquidCrystal_h
#define LiquidCrystal_h

#include "driver/gpio.h"
#include <inttypes.h>

class LiquidCrystal {
public:
  LiquidCrystal(uint8_t rs, uint8_t enable,
                uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);

  void begin(uint8_t cols, uint8_t rows);
  void clear();
  void home();
  void setCursor(uint8_t col, uint8_t row);
  void print(const char* str);
  void print(int num);
  void print(float num, int digits = 2);

private:
  void send(uint8_t value, bool mode);
  void write4bits(uint8_t value);
  void pulseEnable();
  
  uint8_t _rs_pin;
  uint8_t _enable_pin;
  uint8_t _data_pins[4];
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _numlines;
};

#endif
