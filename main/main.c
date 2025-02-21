#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "menu.h"
#include "temperature.h"
#include "network.h"
#include "storage.h"
#include <time.h>
#include <string.h>

static OvenState oven_state = {
    .target_temp = 0,
    .current_temp_top = 0,
    .current_temp_bottom = 0,
    .remaining_time = 0,
    .mode = MODE_BAKING,
    .heater_top = true,
    .heater_bottom = true,
    .light = false,
    .fan = false,
    .timezone = 8, // UTC+8 по умолчанию
    .wifi_ssid = "",
    .wifi_password = "",
    .mqtt_server = DEFAULT_MQTT_SERVER,
    .mqtt_user = DEFAULT_MQTT_USER,
    .mqtt_password = DEFAULT_MQTT_PASS,
    .mqtt_topic = DEFAULT_MQTT_TOPIC
};

void app_main(void) {
    // Инициализация компонентов
    storage_init();
    load_network_settings(&oven_state);
    load_temp_correction(&temp_correction_top, &temp_correction_bottom);

    menu_init();
    temperature_init();
    network_init();

    // Создание задач
    xTaskCreate(menu_task, "menu_task", 4096, &oven_state, TASK_PRIORITY_MENU, NULL);
    xTaskCreate(temperature_task, "temp_task", 4096, &oven_state, TASK_PRIORITY_TEMP, NULL);
    xTaskCreate(network_task, "network_task", 4096, &oven_state, TASK_PRIORITY_NETWORK, NULL);

    // Основной цикл обновления дисплея
    while(1) {
        update_display(&oven_state);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}