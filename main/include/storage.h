#ifndef STORAGE_H
#define STORAGE_H

#include "config.h"
#include <nvs_flash.h>

void storage_init(void);
void save_settings(void);
void load_settings(void);
void save_temp_correction(float top, float bottom);
void load_temp_correction(float *top, float *bottom);

#endif
