#pragma once

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/timer.h>

typedef struct {
    gpio_num_t led_pin; // A LED motor step pin
    ledc_channel_t channel; // LEDC_CHANNEL_0, etc
    ledc_channel_config_t config;
} led_t;

void led_init(led_t& led);
void led_set(uint32_t duty);
