#include "network.h"
#include "storage.h"
#include <string.h>
#include <esp_log.h>

static const char *TAG = "network";
static esp_mqtt_client_handle_t mqtt_client;

void wifi_connect(void) {
    wifi_config_t wifi_config = {0};
    // Load WiFi credentials from NVS
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Disconnected");
            break;
        case MQTT_EVENT_DATA:
            // Handle incoming MQTT messages
            break;
    }
}

void mqtt_connect(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = DEFAULT_MQTT_SERVER,
        .port = DEFAULT_MQTT_PORT,
        .username = DEFAULT_MQTT_USER,
        .password = DEFAULT_MQTT_PASS,
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void send_state_update(OvenState *state) {
    char json[200];
    snprintf(json, sizeof(json),
             "{\"mode\":%d,\"temp_top\":%.1f,\"temp_bottom\":%.1f,\"target\":%.1f,\"time\":%d}",
             state->mode, state->current_temp_top, state->current_temp_bottom,
             state->target_temp, state->remaining_time);
             
    esp_mqtt_client_publish(mqtt_client, DEFAULT_MQTT_TOPIC, json, 0, 1, 0);
}

void network_task(void *pvParameters) {
    OvenState *state = (OvenState*)pvParameters;
    
    while(1) {
        send_state_update(state);
        vTaskDelay(pdMS_TO_TICKS(NETWORK_UPDATE_INTERVAL));
    }
}
