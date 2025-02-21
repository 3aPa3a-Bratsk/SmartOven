#include "MAX6675.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"

MAX6675::MAX6675(int8_t sck, int8_t cs, int8_t so) {
    _sck = sck;
    _cs = cs;
    _so = so;
    
    gpio_set_direction((gpio_num_t)_cs, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)_sck, GPIO_MODE_OUTPUT);
    gpio_set_direction((gpio_num_t)_so, GPIO_MODE_INPUT);
    
    gpio_set_level((gpio_num_t)_cs, 1);
}

double MAX6675::readCelsius(void) {
    uint16_t v;

    gpio_set_level((gpio_num_t)_cs, 0);
    vTaskDelay(pdMS_TO_TICKS(1));

    v = spiread();
    v <<= 8;
    v |= spiread();

    gpio_set_level((gpio_num_t)_cs, 1);

    if (v & 0x4) {
        return NAN; // Thermocouple is disconnected
    }

    v >>= 3;
    return v * 0.25;
}

byte MAX6675::spiread(void) {
    int i;
    byte d = 0;

    for (i = 7; i >= 0; i--) {
        gpio_set_level((gpio_num_t)_sck, 0);
        vTaskDelay(pdMS_TO_TICKS(1));
        if (gpio_get_level((gpio_num_t)_so)) {
            d |= (1 << i);
        }
        gpio_set_level((gpio_num_t)_sck, 1);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return d;
}
