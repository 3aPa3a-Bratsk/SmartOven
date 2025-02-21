#include "menu.h"
#include "storage.h"
#include <string.h>
#include <stdio.h>

static LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
static LCDMenu menu(&lcd);

static int encoder_pos = 0;
static bool encoder_sw = false;
static OvenState *oven_state;

// Структура меню
static const char* main_menu[] = {
    "Режим работы",
    "Настройки шкафа",
    "Сетевые настр.",
    "Калибровка темп."
};

static const char* mode_menu[] = {
    "Колбаса",
    "Запекание",
    "Подогрев",
    "Йогурт"
};

static const char* settings_menu[] = {
    "Нагреватели",
    "Подсветка",
    "Назад"
};

static const char* network_menu[] = {
    "WiFi",
    "MQTT",
    "Время",
    "Назад"
};

static const char* calibration_menu[] = {
    "Верхний датчик",
    "Нижний датчик",
    "Назад"
};

// Текущее состояние меню
static enum {
    MENU_MAIN,
    MENU_MODE,
    MENU_SETTINGS,
    MENU_NETWORK,
    MENU_CALIBRATION,
    MENU_VALUE_EDIT
} menu_state = MENU_MAIN;

static int current_value = 0;
static int min_value = 0;
static int max_value = 100;
static void (*value_callback)(int) = NULL;

void menu_init(void) {
    lcd.begin(16, 2);
    menu.setup_menu(main_menu, 4);

    // Настройка пинов энкодера
    gpio_set_direction(PIN_ENCODER_CLK, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_ENCODER_DT, GPIO_MODE_INPUT);
    gpio_set_direction(PIN_ENCODER_SW, GPIO_MODE_INPUT);

    gpio_set_pull_mode(PIN_ENCODER_CLK, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_ENCODER_DT, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(PIN_ENCODER_SW, GPIO_PULLUP_ONLY);
}

static void start_value_edit(int initial, int min, int max, void (*callback)(int)) {
    menu_state = MENU_VALUE_EDIT;
    current_value = initial;
    min_value = min;
    max_value = max;
    value_callback = callback;
}

static void handle_mode_select(int mode_idx) {
    oven_state->mode = (OperationMode)mode_idx;

    switch(oven_state->mode) {
        case MODE_SAUSAGE:
            start_value_edit(70, 50, 100, [](int val) {
                oven_state->target_temp = val;
            });
            break;

        case MODE_BAKING:
            start_value_edit(180, TEMP_MIN_BAKE, TEMP_MAX_BAKE, [](int val) {
                oven_state->target_temp = val;
            });
            start_value_edit(60, TIME_MIN_BAKE, TIME_MAX_BAKE, [](int val) {
                oven_state->remaining_time = val;
            });
            break;

        case MODE_WARMING:
            start_value_edit(70, TEMP_MIN_WARM, TEMP_MAX_WARM, [](int val) {
                oven_state->target_temp = val;
            });
            start_value_edit(60, TIME_MIN_WARM, TIME_MAX_WARM, [](int val) {
                oven_state->remaining_time = val;
            });
            break;

        case MODE_YOGURT:
            start_value_edit(40, TEMP_MIN_YOGURT, TEMP_MAX_YOGURT, [](int val) {
                oven_state->target_temp = val;
            });
            start_value_edit(360, TIME_MIN_YOGURT, TIME_MAX_YOGURT, [](int val) {
                oven_state->remaining_time = val;
            });
            break;
    }
}

void handle_encoder(void) {
    static int last_clk = 1;
    int clk = gpio_get_level(PIN_ENCODER_CLK);
    int dt = gpio_get_level(PIN_ENCODER_DT);

    if (clk != last_clk) {
        if (dt != clk) {
            encoder_pos++;
        } else {
            encoder_pos--;
        }

        if (menu_state == MENU_VALUE_EDIT) {
            current_value = constrain(current_value + (encoder_pos > 0 ? 1 : -1),
                                    min_value, max_value);
            encoder_pos = 0;
        } else {
            menu.navigate(encoder_pos > 0 ? 1 : -1);
            encoder_pos = 0;
        }
    }
    last_clk = clk;

    // Обработка нажатия кнопки
    if (!gpio_get_level(PIN_ENCODER_SW) && !encoder_sw) {
        encoder_sw = true;

        if (menu_state == MENU_VALUE_EDIT) {
            if (value_callback) {
                value_callback(current_value);
            }
            menu_state = MENU_MAIN;
            menu.setup_menu(main_menu, 4);
        } else {
            switch(menu_state) {
                case MENU_MAIN:
                    switch(menu.get_current_index()) {
                        case 0: // Режим работы
                            menu_state = MENU_MODE;
                            menu.setup_menu(mode_menu, 4);
                            break;
                        case 1: // Настройки шкафа
                            menu_state = MENU_SETTINGS;
                            menu.setup_menu(settings_menu, 3);
                            break;
                        case 2: // Сетевые настройки
                            menu_state = MENU_NETWORK;
                            menu.setup_menu(network_menu, 4);
                            break;
                        case 3: // Калибровка температуры
                            menu_state = MENU_CALIBRATION;
                            menu.setup_menu(calibration_menu, 3);
                            break;
                    }
                    break;

                case MENU_MODE:
                    handle_mode_select(menu.get_current_index());
                    menu_state = MENU_MAIN;
                    menu.setup_menu(main_menu, 4);
                    break;

                case MENU_SETTINGS:
                    switch(menu.get_current_index()) {
                        case 0: // Нагреватели
                            oven_state->heater_top = !oven_state->heater_top;
                            oven_state->heater_bottom = !oven_state->heater_bottom;
                            break;
                        case 1: // Подсветка
                            oven_state->light = !oven_state->light;
                            gpio_set_level(PIN_RELAY_LIGHT, oven_state->light);
                            break;
                        case 2: // Назад
                            menu_state = MENU_MAIN;
                            menu.setup_menu(main_menu, 4);
                            break;
                    }
                    break;

                case MENU_NETWORK:
                    switch(menu.get_current_index()) {
                        case 0: // WiFi
                            // Здесь должен быть код для настройки WiFi
                            break;
                        case 1: // MQTT
                            // Здесь должен быть код для настройки MQTT
                            break;
                        case 2: // Время
                            // Здесь должен быть код для настройки времени
                            break;
                        case 3: // Назад
                            menu_state = MENU_MAIN;
                            menu.setup_menu(main_menu, 4);
                            break;
                    }
                    break;

                case MENU_CALIBRATION:
                    switch(menu.get_current_index()) {
                        case 0: // Верхний датчик
                            start_value_edit(0, -10, 10, [](int val) {
                                float temp = (float)val;
                                apply_temp_correction(&temp, true);
                            });
                            break;
                        case 1: // Нижний датчик
                            start_value_edit(0, -10, 10, [](int val) {
                                float temp = (float)val;
                                apply_temp_correction(&temp, false);
                            });
                            break;
                        case 2: // Назад
                            menu_state = MENU_MAIN;
                            menu.setup_menu(main_menu, 4);
                            break;
                    }
                    break;
            }
        }
    } else if (gpio_get_level(PIN_ENCODER_SW)) {
        encoder_sw = false;
    }
}

void update_display(OvenState *state) {
    char line1[17];
    char line2[17];

    if (menu_state == MENU_VALUE_EDIT) {
        snprintf(line1, 17, "Установить:");
        snprintf(line2, 17, "%d", current_value);
    } else {
        switch (state->mode) {
            case MODE_SAUSAGE:
                snprintf(line1, 17, "Колб T:%3.1f/%3.1f", 
                        state->current_temp_bottom, state->target_temp);
                snprintf(line2, 17, "Разн: %3.1f°C", 
                        state->current_temp_top - state->current_temp_bottom);
                break;

            case MODE_BAKING:
                snprintf(line1, 17, "Вып T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Время: %02d:%02d", 
                        state->remaining_time / 60, state->remaining_time % 60);
                break;

            case MODE_WARMING:
                snprintf(line1, 17, "Подг T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Время: %02d:%02d", 
                        state->remaining_time / 60, state->remaining_time % 60);
                break;

            case MODE_YOGURT:
                snprintf(line1, 17, "Йог T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Время: %02d:%02d", 
                        state->remaining_time / 60, state->remaining_time % 60);
                break;
        }
    }

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void menu_task(void *pvParameters) {
    oven_state = (OvenState*)pvParameters;

    while(1) {
        handle_encoder();
        vTaskDelay(pdMS_TO_TICKS(MENU_UPDATE_INTERVAL));
    }
}