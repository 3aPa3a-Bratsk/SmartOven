#include "temperature.h"
#include "storage.h"

static MAX6675 thermocouple_top(PIN_MAX6675_TOP_SCK, PIN_MAX6675_TOP_CS, PIN_MAX6675_TOP_SO);
static MAX6675 thermocouple_bottom(PIN_MAX6675_BOTTOM_SCK, PIN_MAX6675_BOTTOM_CS, PIN_MAX6675_BOTTOM_SO);

extern float temp_correction_top;
extern float temp_correction_bottom;

void temperature_init(void) {
    gpio_set_direction(PIN_RELAY_TOP, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RELAY_BOTTOM, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RELAY_LIGHT, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RELAY_FAN, GPIO_MODE_OUTPUT);
}

float read_temperature(int sensor) {
    float temp;
    if (sensor == 0) {
        temp = thermocouple_top.readCelsius() + temp_correction_top;
    } else {
        temp = thermocouple_bottom.readCelsius() + temp_correction_bottom;
    }
    return temp;
}

void control_temperature(OvenState *state) {
    float avg_temp = (state->current_temp_top + state->current_temp_bottom) / 2;
    float temp_diff = fabsf(state->current_temp_top - state->current_temp_bottom);

    switch (state->mode) {
        case MODE_SAUSAGE:
            // В режиме колбасы важно поддерживать равномерную температуру
            // и не превышать целевую температуру в продукте
            if (state->current_temp_bottom < state->target_temp) {
                if (temp_diff > 5.0) {
                    // Если разница температур большая, включаем оба нагревателя
                    state->heater_top = true;
                    state->heater_bottom = true;
                } else {
                    // Иначе используем только нижний нагреватель
                    state->heater_top = false;
                    state->heater_bottom = true;
                }
                state->fan = true; // Включаем вентилятор для равномерного нагрева
            } else {
                state->heater_top = false;
                state->heater_bottom = false;
                state->fan = false;
            }
            break;

        case MODE_BAKING:
            // В режиме выпечки поддерживаем среднюю температуру
            if (avg_temp < state->target_temp) {
                if (avg_temp < state->target_temp - 20) {
                    // Быстрый нагрев
                    state->heater_top = true;
                    state->heater_bottom = true;
                    state->fan = true;
                } else {
                    // Точная регулировка
                    state->heater_top = (state->current_temp_top < state->target_temp);
                    state->heater_bottom = (state->current_temp_bottom < state->target_temp);
                    state->fan = true;
                }
            } else {
                state->heater_top = false;
                state->heater_bottom = false;
                state->fan = (avg_temp < state->target_temp + 5);
            }
            break;

        case MODE_WARMING:
            // В режиме подогрева используем в основном нижний нагреватель
            if (avg_temp < state->target_temp) {
                state->heater_bottom = true;
                state->heater_top = (avg_temp < state->target_temp - 10);
                state->fan = true;
            } else {
                state->heater_top = false;
                state->heater_bottom = false;
                state->fan = (avg_temp < state->target_temp + 2);
            }
            break;

        case MODE_YOGURT:
            // В режиме йогурта важно поддерживать стабильную низкую температуру
            if (avg_temp < state->target_temp) {
                if (avg_temp < state->target_temp - 2) {
                    state->heater_bottom = true;
                    state->fan = true;
                } else {
                    state->heater_bottom = true;
                    state->fan = false;
                }
                state->heater_top = false;
            } else {
                state->heater_top = false;
                state->heater_bottom = false;
                state->fan = false;
            }
            break;
    }

    // Применяем состояния к реле
    gpio_set_level(PIN_RELAY_TOP, state->heater_top);
    gpio_set_level(PIN_RELAY_BOTTOM, state->heater_bottom);
    gpio_set_level(PIN_RELAY_FAN, state->fan);
}

void temperature_task(void *pvParameters) {
    OvenState *state = (OvenState*)pvParameters;

    while(1) {
        state->current_temp_top = read_temperature(0);
        state->current_temp_bottom = read_temperature(1);
        control_temperature(state);
        vTaskDelay(pdMS_TO_TICKS(TEMP_READ_INTERVAL));
    }
}

void apply_temp_correction(float *temp, bool is_top) {
    if (is_top) {
        temp_correction_top = *temp;
    } else {
        temp_correction_bottom = *temp;
    }
    save_temp_correction(temp_correction_top, temp_correction_bottom);
}