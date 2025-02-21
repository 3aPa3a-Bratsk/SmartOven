#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "menu.h"
#include "temperature.h"
#include "network.h"
#include "storage.h"
#include "VoiceControl.h" // Added include for voice control
#include <time.h>
#include <string.h>

// Global variables for temperature correction
float temp_correction_top = 0.0;
float temp_correction_bottom = 0.0;

static VoiceControl voice_control; // Added voice control object

static OvenState oven_state = {
    .target_temp = 0,
    .current_temp_top = 0,
    .current_temp_bottom = 0,
    .remaining_time = 0,
    .mode = MODE_BAKING,
    .heater_top = false,
    .heater_bottom = false,
    .light = false,
    .fan = false,
    .timezone = 8, // UTC+8 by default
    .wifi_ssid = "",
    .wifi_password = "",
    .mqtt_server = DEFAULT_MQTT_SERVER,
    .mqtt_user = DEFAULT_MQTT_USER,
    .mqtt_password = DEFAULT_MQTT_PASS,
    .mqtt_topic = DEFAULT_MQTT_TOPIC
};

static void handle_voice_command(const char* command) {
    if (strcmp(command, VOICE_CMD_STOP) == 0) {
        // Stop the current process
        oven_state.heater_top = false;
        oven_state.heater_bottom = false;
        oven_state.fan = false;
    } else if (strcmp(command, VOICE_CMD_START) == 0) {
        // Start in the current mode
        control_temperature(&oven_state);
    } else if (strcmp(command, VOICE_CMD_LIGHT) == 0) {
        // Toggle the backlight
        oven_state.light = !oven_state.light;
        gpio_set_level(PIN_RELAY_LIGHT, oven_state.light);
    } else if (strcmp(command, VOICE_CMD_MODE) == 0) {
        // Switch modes
        oven_state.mode = (oven_state.mode + 1) % 4;
    } else if (strcmp(command, VOICE_CMD_TEMP_UP) == 0) {
        // Increase temperature by 5 degrees
        oven_state.target_temp = MIN(oven_state.target_temp + 5, 
            oven_state.mode == MODE_YOGURT ? TEMP_MAX_YOGURT :
            oven_state.mode == MODE_WARMING ? TEMP_MAX_WARM :
            oven_state.mode == MODE_BAKING ? TEMP_MAX_BAKE : 100);
    } else if (strcmp(command, VOICE_CMD_TEMP_DOWN) == 0) {
        // Decrease temperature by 5 degrees
        oven_state.target_temp = MAX(oven_state.target_temp - 5,
            oven_state.mode == MODE_YOGURT ? TEMP_MIN_YOGURT :
            oven_state.mode == MODE_WARMING ? TEMP_MIN_WARM :
            oven_state.mode == MODE_BAKING ? TEMP_MIN_BAKE : 50);
    }
}

void app_main(void) {
    // Initialize components
    storage_init();
    load_network_settings(&oven_state);
    load_temp_correction(&temp_correction_top, &temp_correction_bottom);

    menu_init();
    temperature_init();
    network_init();

    // Initialize voice control
    if (oven_state.voice_control_enabled) {
        voice_control.begin();
        voice_control.set_callback(handle_voice_command);
        voice_control.start_listening();
    }

    // Create tasks
    xTaskCreate(menu_task, "menu_task", 4096, &oven_state, TASK_PRIORITY_MENU, NULL);
    xTaskCreate(temperature_task, "temp_task", 4096, &oven_state, TASK_PRIORITY_TEMP, NULL);
    xTaskCreate(network_task, "network_task", 4096, &oven_state, TASK_PRIORITY_NETWORK, NULL);

    // Main loop for display update
    while(1) {
        update_display(&oven_state);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}