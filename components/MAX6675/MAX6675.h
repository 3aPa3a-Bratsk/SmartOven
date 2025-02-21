#ifndef MAX6675_H
#define MAX6675_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef unsigned char byte;

class MAX6675 {
 public:
  MAX6675(int8_t sck, int8_t cs, int8_t so);
  double readCelsius();

 private:
  int8_t _sck, _cs, _so;
  byte spiread(void);
};

#endif
