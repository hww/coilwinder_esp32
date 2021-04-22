#pragma once

#include <stdint.h>
#include <driver/gpio.h>
#include <driver/timer.h>


typedef struct {
    timer_idx_t timer;   // TIMER0 or TIMER1
    gpio_num_t step_pin; // A step motor step pin
    gpio_num_t dir_pin;  // A step motor direcion
    bool status;         // Toggle every turn dirrection
    bool cnt_ena;        // Auto turn direction
    int cnt;             // Auto turn direction after this steps
    int coil_length_steps; // Auto turn direction after this steps
} motor_t;

void motor_init(motor_t& motor);
void motor_set_step_duration(motor_t& motor, int stepDelay);
