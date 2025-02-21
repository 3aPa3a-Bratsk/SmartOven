#ifndef CONFIG_H
#define CONFIG_H

// GPIO pins configuration
#define PIN_MAX6675_TOP_SCK     18
#define PIN_MAX6675_TOP_CS      5
#define PIN_MAX6675_TOP_SO      19
#define PIN_MAX6675_BOTTOM_SCK  18
#define PIN_MAX6675_BOTTOM_CS   17
#define PIN_MAX6675_BOTTOM_SO   19

#define PIN_LCD_RS              25
#define PIN_LCD_EN              26
#define PIN_LCD_D4              27
#define PIN_LCD_D5              14
#define PIN_LCD_D6              12
#define PIN_LCD_D7              13

#define PIN_ENCODER_CLK         32
#define PIN_ENCODER_DT          33
#define PIN_ENCODER_SW          34

#define PIN_RELAY_TOP           15
#define PIN_RELAY_BOTTOM        2
#define PIN_RELAY_LIGHT         4
#define PIN_RELAY_FAN           16

// Temperature limits
#define TEMP_MIN_YOGURT         30
#define TEMP_MAX_YOGURT         50
#define TEMP_MIN_WARM           50
#define TEMP_MAX_WARM           120
#define TEMP_MIN_BAKE           100
#define TEMP_MAX_BAKE           250

// Time limits (in minutes)
#define TIME_MIN_YOGURT         180  // 3 hours
#define TIME_MAX_YOGURT         480  // 8 hours
#define TIME_MIN_WARM            30   // 30 минут
#define TIME_MAX_WARM            180  // 3 часа
#define TIME_MIN_BAKE            15   // 15 минут
#define TIME_MAX_BAKE            180  // 3 часа

// Default network settings
#define DEFAULT_MQTT_SERVER     "bratsk-service.ru"
#define DEFAULT_MQTT_PORT       8883
#define DEFAULT_MQTT_USER       "smarthome"
#define DEFAULT_MQTT_PASS       "Matrix666!"
#define DEFAULT_MQTT_TOPIC      "ZOwen"

// Task priorities
#define TASK_PRIORITY_MENU      5
#define TASK_PRIORITY_TEMP      4
#define TASK_PRIORITY_NETWORK   3

// Other constants
#define TEMP_READ_INTERVAL      1000  // ms
#define MENU_UPDATE_INTERVAL    100   // ms
#define NETWORK_UPDATE_INTERVAL 5000  // ms

// Определения для WiFi
#define MAX_SSID_LENGTH         32
#define MAX_PASSWORD_LENGTH     64
#define MAX_MQTT_SERVER_LENGTH  64
#define MAX_MQTT_USER_LENGTH    32
#define MAX_MQTT_PASS_LENGTH    32
#define MAX_MQTT_TOPIC_LENGTH   32


typedef enum {
    MODE_SAUSAGE,
    MODE_BAKING,
    MODE_WARMING,
    MODE_YOGURT
} OperationMode;

typedef struct {
    float target_temp;
    float current_temp_top;
    float current_temp_bottom;
    int remaining_time;
    OperationMode mode;
    bool heater_top;
    bool heater_bottom;
    bool light;
    bool fan;
    // Новые поля для настроек
    char wifi_ssid[MAX_SSID_LENGTH];
    char wifi_password[MAX_PASSWORD_LENGTH];
    char mqtt_server[MAX_MQTT_SERVER_LENGTH];
    char mqtt_user[MAX_MQTT_USER_LENGTH];
    char mqtt_password[MAX_MQTT_PASS_LENGTH];
    char mqtt_topic[MAX_MQTT_TOPIC_LENGTH];
    int timezone;
} OvenState;

#endif