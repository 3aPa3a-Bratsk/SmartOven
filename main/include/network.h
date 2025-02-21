#ifndef NETWORK_H
#define NETWORK_H

#include "config.h"
#include <esp_wifi.h>
#include <mqtt_client.h>

void network_init(void);
void network_task(void *pvParameters);
void wifi_connect(void);
void mqtt_connect(void);
void send_state_update(OvenState *state);

#endif
