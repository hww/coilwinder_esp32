#pragma once

#include <stdint.h>
#include <driver/gpio.h>
#include "esp_timer.h"
#include "time.hpp"

typedef struct {
    int id;
    /** The driver settings */
//    timer_group_t timer_group;
//    timer_idx_t timer;   // TIMER0 or TIMER1
    uint64_t timer_interval;
    /** GPIO */
    gpio_num_t step_pin;
    gpio_num_t dir_pin;
    gpio_num_t enable_pin;
    gpio_num_t endstop_pin;
    int dir_pin_reverse;
    int step_pin_reverse;
    /** Kinematic */
    float max_velocity;
    float max_accel;
    float rotation_distance;
    int microsteps;
    int steps_per_turn;
    float position_min;
    float position_endstop;
    float position_max;
    float homing_speed;
    float homing_retract_dist;
    float second_homing_speed;

    /** Control containes requested state */
    bool enabled;   // enable disable motor current
    float velocity; // desired velocity
    float position; // desired position in units

    /** Status display the actual state of motor */
    float current_velocity;
    float current_position;
    int current_dir;
    int current_position_steps;
    float distance_per_step;
    bool complete;
    Time time;

    /** System */
    uint32_t isr_count;
    esp_timer_handle_t timer_handle;
    esp_timer_create_args_t timer_arg;
} motor_t;

void motor_init(motor_t& motor);

void kinematic_init();
void kinematic_update();

extern motor_t motor_x;
extern motor_t motor_y;
extern float g_speed;
