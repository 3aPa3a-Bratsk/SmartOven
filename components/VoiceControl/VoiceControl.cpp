#include "VoiceControl.h"
#include "esp_log.h"
#include <string.h>

static const char* TAG = "voice_control";

VoiceControl::VoiceControl() {
    _callback = nullptr;
    _task_handle = nullptr;
    _is_listening = false;
}

void VoiceControl::begin() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = I2S_BUFFER_SIZE,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_SCK_PIN,
        .ws_io_num = I2S_WS_PIN,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_PIN
    };
    
    ESP_ERROR_CHECK(i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL));
    ESP_ERROR_CHECK(i2s_set_pin(I2S_PORT, &pin_config));
    ESP_ERROR_CHECK(i2s_start(I2S_PORT));
    
    ESP_LOGI(TAG, "I2S микрофон инициализирован");
}

void VoiceControl::set_callback(voice_command_callback_t callback) {
    _callback = callback;
}

void VoiceControl::start_listening() {
    if (!_is_listening) {
        _is_listening = true;
        xTaskCreate(process_task, "voice_task", 4096, this, 5, &_task_handle);
        ESP_LOGI(TAG, "Начало прослушивания голосовых команд");
    }
}

void VoiceControl::stop_listening() {
    if (_is_listening) {
        _is_listening = false;
        if (_task_handle != nullptr) {
            vTaskDelete(_task_handle);
            _task_handle = nullptr;
        }
        ESP_LOGI(TAG, "Прослушивание остановлено");
    }
}

void VoiceControl::process_task(void* arg) {
    VoiceControl* voice = static_cast<VoiceControl*>(arg);
    while (voice->_is_listening) {
        voice->process_audio();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(NULL);
}

void VoiceControl::process_audio() {
    size_t bytes_read = 0;
    i2s_read(I2S_PORT, _audio_buffer, sizeof(_audio_buffer), &bytes_read, portMAX_DELAY);
    
    if (detect_command()) {
        const char* command = recognize_command();
        if (command && _callback) {
            _callback(command);
        }
    }
}

bool VoiceControl::detect_command() {
    // Здесь должна быть реализация определения наличия команды
    // Например, по уровню громкости или ключевому слову
    return false;
}

const char* VoiceControl::recognize_command() {
    // Здесь должна быть реализация распознавания команды
    // Возвращает распознанную команду или nullptr
    return nullptr;
}
