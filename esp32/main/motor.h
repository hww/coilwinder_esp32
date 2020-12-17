#pragma once

#include <stdint.h>
#include <driver/gpio.h>

typedef struct {
    timer_idx_t tmr;
    gpio_num_t step_pin;
    gpio_num_t dir_pin;
} Motor_t;
