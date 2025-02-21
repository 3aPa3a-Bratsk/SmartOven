#include "menu.h"
#include "storage.h"
#include <string.h>
#include <stdio.h>

static LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
static LCDMenu menu(&lcd);

static int encoder_pos = 0;
static bool encoder_sw = false;

// Структура меню
static const char* main_menu[] = {
    "Mode",
    "Oven Settings",
    "Network Config",
    "Temp Calibration"
};

static const char* mode_menu[] = {
    "Sausage",
    "Baking",
    "Warming",
    "Yogurt"
};

static const char* settings_menu[] = {
    "Heaters",
    "Light",
    "Back"
};

static const char* network_menu[] = {
    "WiFi Setup",
    "MQTT Setup",
    "Time Setup",
    "Back"
};

static const char* calibration_menu[] = {
    "Top sensor",
    "Bottom sensor",
    "Back"
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

    // Setup encoder pins
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

static void handle_mode_select(OvenState *state, int mode_idx) {
    state->mode = (OperationMode)mode_idx;
    switch(state->mode) {
        case MODE_SAUSAGE:
            start_value_edit(70, 50, 100, [](int val) { 
                state->target_temp = val;
            });
            break;
        case MODE_BAKING:
            start_value_edit(180, TEMP_MIN_BAKE, TEMP_MAX_BAKE, [](int val) {
                state->target_temp = val;
            });
            // После установки температуры запросим время
            start_value_edit(60, TIME_MIN_BAKE, TIME_MAX_BAKE, [](int val) {
                state->remaining_time = val;
            });
            break;
        case MODE_WARMING:
            start_value_edit(70, TEMP_MIN_WARM, TEMP_MAX_WARM, [](int val) {
                state->target_temp = val;
            });
            start_value_edit(60, TIME_MIN_WARM, TIME_MAX_WARM, [](int val) {
                state->remaining_time = val;
            });
            break;
        case MODE_YOGURT:
            start_value_edit(40, TEMP_MIN_YOGURT, TEMP_MAX_YOGURT, [](int val) {
                state->target_temp = val;
            });
            start_value_edit(360, TIME_MIN_YOGURT, TIME_MAX_YOGURT, [](int val) {
                state->remaining_time = val;
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

    // Handle button press
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
                        case 0: // Mode
                            menu_state = MENU_MODE;
                            menu.setup_menu(mode_menu, 4);
                            break;
                        case 1: // Oven Settings
                            menu_state = MENU_SETTINGS;
                            menu.setup_menu(settings_menu, 3);
                            break;
                        case 2: // Network Config
                            menu_state = MENU_NETWORK;
                            menu.setup_menu(network_menu, 4);
                            break;
                        case 3: // Temp Calibration
                            menu_state = MENU_CALIBRATION;
                            menu.setup_menu(calibration_menu, 3);
                            break;
                    }
                    break;

                case MENU_MODE:
                    handle_mode_select(get_oven_state(), menu.get_current_index());
                    menu.setup_menu(main_menu,4);
                    menu_state = MENU_MAIN;
                    break;

                case MENU_SETTINGS:
                    switch(menu.get_current_index()) {
                        case 0: // Heaters
                            // Toggle heaters
                            break;
                        case 1: // Light
                            get_oven_state()->light = !get_oven_state()->light;
                            gpio_set_level(PIN_RELAY_LIGHT, get_oven_state()->light);
                            break;
                        case 2: // Back
                            menu_state = MENU_MAIN;
                            menu.setup_menu(main_menu, 4);
                            break;
                    }
                    break;

                // ... остальные case для других состояний меню
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
        snprintf(line1, 17, "Set value:");
        snprintf(line2, 17, "%d", current_value);
    } else {
        switch (state->mode) {
            case MODE_SAUSAGE:
                snprintf(line1, 17, "Saus T:%3.1f/%3.1f", state->current_temp_top, state->target_temp);
                snprintf(line2, 17, "Time: %02d:%02d", state->remaining_time / 60, state->remaining_time % 60);
                break;
            case MODE_BAKING:
                snprintf(line1, 17, "Bake T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Time: %02d:%02d", state->remaining_time / 60, state->remaining_time % 60);
                break;
            case MODE_WARMING:
                snprintf(line1, 17, "Warm T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Time: %02d:%02d", state->remaining_time / 60, state->remaining_time % 60);
                break;
            case MODE_YOGURT:
                snprintf(line1, 17, "Yogt T:%3.1f/%3.1f", 
                        (state->current_temp_top + state->current_temp_bottom) / 2, 
                        state->target_temp);
                snprintf(line2, 17, "Time: %02d:%02d", state->remaining_time / 60, state->remaining_time % 60);
                break;
        }
    }

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

void menu_task(void *pvParameters) {
    while(1) {
        handle_encoder();
        vTaskDelay(pdMS_TO_TICKS(MENU_UPDATE_INTERVAL));
    }
}