#ifndef MENU_H
#define CONFIG_H

#include "config.h"
#include <LiquidCrystal.h>
#include <LCDMenu.h>

void menu_init(void);
void menu_task(void *pvParameters);
void update_display(OvenState *state);
void handle_encoder(void);

#endif
