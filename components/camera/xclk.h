#pragma once

#include "camera_common.h"

esp_err_t camera_enable_out_clock();

void camera_disable_out_clock();
//esp_err_t camera_enable_out_clock_freq(camera_config_t* config,int new_freq);
