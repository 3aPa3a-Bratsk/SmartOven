#ifndef VOICE_CONTROL_H
#define VOICE_CONTROL_H

#include "driver/i2s.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

// Настройки I2S для микрофона INMP441 или аналогичного
#define I2S_WS_PIN          23
#define I2S_SCK_PIN         22
#define I2S_SD_PIN          21
#define I2S_PORT            I2S_NUM_0
#define I2S_SAMPLE_RATE     16000
#define I2S_BUFFER_SIZE     512

typedef void (*voice_command_callback_t)(const char* command);

class VoiceControl {
public:
    VoiceControl();
    void begin();
    void set_callback(voice_command_callback_t callback);
    void start_listening();
    void stop_listening();

private:
    static void process_task(void* arg);
    void process_audio();
    bool detect_command();
    const char* recognize_command();
    
    voice_command_callback_t _callback;
    TaskHandle_t _task_handle;
    bool _is_listening;
    int16_t _audio_buffer[I2S_BUFFER_SIZE];
};

#endif
