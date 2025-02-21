#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "config.h"
#include <max6675.h>

void temperature_init(void);
void temperature_task(void *pvParameters);
float read_temperature(int sensor);
void control_temperature(OvenState *state);
void apply_temp_correction(float *temp, bool is_top);

#endif
