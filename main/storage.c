#include "storage.h"
#include <string.h>
#include <time.h>

#define STORAGE_NAMESPACE "oven"

void storage_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void save_network_settings(OvenState *state) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_str(handle, "wifi_ssid", state->wifi_ssid);
        nvs_set_str(handle, "wifi_pass", state->wifi_password);
        nvs_set_str(handle, "mqtt_srv", state->mqtt_server);
        nvs_set_str(handle, "mqtt_user", state->mqtt_user);
        nvs_set_str(handle, "mqtt_pass", state->mqtt_password);
        nvs_set_str(handle, "mqtt_topic", state->mqtt_topic);
        nvs_set_i32(handle, "timezone", state->timezone);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

void load_network_settings(OvenState *state) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
        size_t required_size;

        if (nvs_get_str(handle, "wifi_ssid", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "wifi_ssid", state->wifi_ssid, &required_size);
        }
        if (nvs_get_str(handle, "wifi_pass", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "wifi_pass", state->wifi_password, &required_size);
        }
        if (nvs_get_str(handle, "mqtt_srv", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "mqtt_srv", state->mqtt_server, &required_size);
        } else {
            strncpy(state->mqtt_server, DEFAULT_MQTT_SERVER, MAX_MQTT_SERVER_LENGTH);
        }
        if (nvs_get_str(handle, "mqtt_user", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "mqtt_user", state->mqtt_user, &required_size);
        } else {
            strncpy(state->mqtt_user, DEFAULT_MQTT_USER, MAX_MQTT_USER_LENGTH);
        }
        if (nvs_get_str(handle, "mqtt_pass", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "mqtt_pass", state->mqtt_password, &required_size);
        } else {
            strncpy(state->mqtt_password, DEFAULT_MQTT_PASS, MAX_MQTT_PASS_LENGTH);
        }
        if (nvs_get_str(handle, "mqtt_topic", NULL, &required_size) == ESP_OK) {
            nvs_get_str(handle, "mqtt_topic", state->mqtt_topic, &required_size);
        } else {
            strncpy(state->mqtt_topic, DEFAULT_MQTT_TOPIC, MAX_MQTT_TOPIC_LENGTH);
        }

        nvs_get_i32(handle, "timezone", &state->timezone);
        nvs_close(handle);
    }
}

void save_temp_correction(float top, float bottom) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_OK) {
        nvs_set_float(handle, "temp_corr_top", top);
        nvs_set_float(handle, "temp_corr_bot", bottom);
        nvs_commit(handle);
        nvs_close(handle);
    }
}

void load_temp_correction(float *top, float *bottom) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
        nvs_get_float(handle, "temp_corr_top", top);
        nvs_get_float(handle, "temp_corr_bot", bottom);
        nvs_close(handle);
    }
}