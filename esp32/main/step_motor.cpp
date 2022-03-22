#include <cstdint>
#include <math.h>
#include <stdlib.h>
#include <string>

#include "esp_timer.h"

#include "config.h"
#include "menu_item.h"
#include "menu_system.h"
#include "step_motor.h"
#include "step_motor_config.h"
#include "step_motor_hal.h"
#include "time.h"
#include "menu.h"
#include "mathlib.h"

static const char* TAG = "motor";


/** ******************************************/
/** Moving to target agent                   */
/** ******************************************/

// {{{ Text

StepMotorAgent::StepMotorAgent()
    : id(0)
    , motor()
    , moving(false)
    , target(0)
    , velocity(0)
    , test_endpoint(false)
    , log(0)
{}

void StepMotorAgent::init(StepMotor* _motor) {
    motor = _motor;
    id = motor->id;
    config = motor->config;
}

void StepMotorAgent::move_to(unit_t pos, unit_t _velocity) {
    target = config->units_to_steps(pos);
    velocity = abs(_velocity);
    test_endpoint = false;
    moving = true;
    on_step();
    if (log > 0)
        ESP_LOGI(TAG, "[%d] Moving to pos %f (steps  %d)", motor->id, pos, (int)target);
}

void StepMotorAgent::move_to(steps_t pos, unit_t _velocity) {
    target = pos;
    velocity = abs(_velocity);
    test_endpoint = false;
    moving = true;
    on_step();
    if (log > 0)
        ESP_LOGI(TAG, "[%d] Moving to %d", motor->id, (int)target);
}

void StepMotorAgent::set_test_endpoint(bool v) {
    test_endpoint = v;
}

void StepMotorAgent::on_step() {
    if (motor == nullptr)
        return;

    if (moving) {
        if (target < motor->position) {
            motor->set_target_velocity(-velocity);
        } else if (target > motor->position) {
            motor->set_target_velocity(velocity);
        } else {
            motor->set_target_velocity(0);
            moving = false;
            if (log > 0)
                ESP_LOGI(TAG, "[%d] Moving complete", motor->id);
        }

        if (test_endpoint && motor->hal.get_endpoint())
            stop();
    }
}

void StepMotorAgent::stop() {
    if (motor == nullptr)
        return;

    if (moving) {
        moving = false;
        motor->set_target_velocity(0);
    }
}

// }}}

/** ******************************************/
/** The motor                                */
/** ******************************************/

StepMotor::StepMotor() {
}

/** The @arg points to motor_t structure */
static void  c_timer_isr(void* arg) {
    ((StepMotor*)arg)->isr();
}
/** C-homing task */
static void c_homing_task(void* arg) {
    ((StepMotor*)arg)->homing_task();
}


/** Initialize the motor */
void StepMotor::init(StepMotorConfig* _config) {
    config = _config;
    id = _config->id;
    ESP_LOGI(TAG, "[%d] Initialize motor", id);
    config->init();
    hal.init(this);
    agent.init(this);
    // Reset the motor's status
    target_velocity = 0;
    velocity = 0;
    position = 0;
    // Configure timer
    timer_arg.callback = &c_timer_isr;
    timer_arg.arg = this;
    ESP_ERROR_CHECK(esp_timer_create(&timer_arg, &timer_handle));
    /* Start the timers */
    timer_interval_us = TIMER_IDLE_DELAY_US;
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, timer_interval_us));
}

/** Initialize menu system */
void StepMotor::init_menu(std::string path) {

    auto menu = MenuSystem::instance.get_or_create(path);
    // Move to target position
    menu->add(new FloatItem(menu, "pos+-0.1",
                            [&]() { return (float)get_position(); },
                            [&](float v) { set_target_position((unit_t)v); }));
    menu->get_last<FloatItem>().set_step(1).set_precision(1);
    // Move to target position
    menu->add(new FloatItem(menu, "pos+-1.0",
                            [&]() { return (float)get_position(); },
                            [&](float v) { set_target_position((unit_t)v); }));
    menu->get_last<FloatItem>().set_step(10).set_precision(1);
    // Move to target position
    menu->add(new FloatItem(menu, "pos+-10.",
                            [&]() { return (float)get_position(); },
                            [&](float v) { set_target_position((unit_t)v); }));
    menu->get_last<FloatItem>().set_step(100).set_precision(1);
    // Velocity
    menu->add(new FloatItem(menu, "velocity",
                            [&]() { return (float)get_target_velocity(); },
                            [&](float v) { set_target_velocity((unit_t)v); }));
    menu->get_last<FloatItem>().set_step(1).set_precision(0);
    // Log
    menu->add(new IntItem(menu, "log",
                          [&] () -> int { return log; },
                          [&](int v) { log = v; }));
}

/** Update the motor even 20ms */
void StepMotor::update(float time) {
    delta_time = time - previous_update_at;
    previous_update_at = time;
    update_velocity(time);
}

// ==================================================
// Utilities
// ==================================================

unit_t StepMotor::get_position() {
    return config->steps_to_units(position);
}
// {{{
unit_t StepMotor::get_target_position() {
    return config->steps_to_units(agent.target);
}
void StepMotor::set_target_position(unit_t pos) {
    agent.move_to(pos, config->max_velocity);
}
// }}}
// {{{
unit_t StepMotor::get_target_velocity() {
    return target_velocity;
}
void StepMotor::set_target_velocity(unit_t vel) {
    target_velocity = vel;
}
// }}}
// {{{
unit_t StepMotor::get_velocity()
{
    return velocity;
}
unit_t StepMotor::get_default_velocity()
{
    return config->max_velocity;
}
// }}}
// {{{
bool StepMotor::get_enable() {
    return hal.get_enable();
}
void StepMotor::set_enable(bool v) {
    hal.set_enable(v);
}
// }}}
bool StepMotor::get_endpoint() {
    return hal.get_endpoint();
}
bool StepMotor::is_moving() {
    return agent.moving;
}
bool StepMotor::is_moving_home() {
    return get_direction(target_velocity) == get_direction(config->homing_dir);
}

unit_t StepMotor::get_acceleration() {
    return config->max_accel;
}
// ==================================================
// Velocity controller
// ==================================================

/** Return true if sucesfully convert velocity to timer inteval */
bool StepMotor::verify_timer_interval(uint64_t &interval) {
    if (interval == 0) {
        interval = TIMER_IDLE_DELAY_US;
        ESP_LOGE(TAG, "[%d] The timer interval is zero", id);
        return false;
    } else if (interval > MAXIMUM_TIMER_INTERVAL_US) {
        interval = MAXIMUM_TIMER_INTERVAL_US;
        ESP_LOGW(TAG, "[%d] The timer interval is too big", id);
        return false;
    } else if (interval < MINIMUM_TIMER_INTERVAL_US) {
        interval = MINIMUM_TIMER_INTERVAL_US;
        ESP_LOGE(TAG, "[%d] The timer interval is too small", id);
        return false;
    }
    return true;
}

/** make current velocity ecual to desired velocity */
void StepMotor::update_velocity(float time) {
    float old_velocity = velocity;
    // compute direction
    auto veldif = target_velocity - velocity;
    auto accdir = get_direction(veldif);
    // compute acceleration
    auto accel = config->max_accel * delta_time;

    // apply acceleration to velocity
    velocity += accel * accdir;

    if (fabs(target_velocity - velocity) < accel) {
        velocity = target_velocity;
    } else {
        // limit velocity
        if (accdir > 0) {
            if (velocity > config->max_velocity)
                velocity = config->max_velocity;
        } else if (accdir < 0) {
            if (velocity < -config->max_velocity)
                velocity = -config->max_velocity;
        }
    }

    // Restart the timer
    if (velocity != old_velocity) {

        if (velocity == 0) {
            timer_interval_us = TIMER_IDLE_DELAY_US;
        } else {
            float steps_per_sec = config->units_to_fsteps(abs(velocity));
            if (log > 3)
                printf("[%d] steps-per-sec: %f\n", id, steps_per_sec);
            timer_interval_us = (uint64_t)(((double)1.0 / steps_per_sec) * 1000000);
            verify_timer_interval(timer_interval_us);
        }
        if (log > 2)
            printf("[%d] tgt-vel: %f vel: %f interval:  %lf\n", id, target_velocity, velocity,(double)timer_interval_us);

        // Restart the timer
        //esp_timer_stop(timer_handle);
        //esp_timer_start_once(timer_handle, timer_interval_us);
    }
}

// ==================================================
// Timer ISR
// ==================================================

void StepMotor::isr() {
    // Just for debugging update the value
    isr_count++;
    esp_timer_stop(timer_handle);
    esp_timer_start_once(timer_handle, timer_interval_us);

    if (agent.moving && velocity!=0) {
        // Set direction and make idle if the pin was changed
        bool ndir = velocity > 0;
        bool odir = hal.get_direction();
        hal.set_direction(ndir);

        if (ndir != odir)
            ets_delay_us(MOTOR_DIR_PULSE_DELAY_MS);

        hal.set_step(true);
        ets_delay_us(MOTOR_STEP_PULSE_WIDTH_MS);
        hal.set_step(false);
        // compure the position in units
        position += get_direction(velocity);
        agent.on_step();
    }
}

// ==================================================
// Moving to the target point
// ==================================================

void StepMotor::set_origin()
{
    position = 0;
}

/** Move motor to position with this velocity */
void StepMotor::move_to(steps_t position, unit_t velocity) {
    agent.move_to(position, velocity);
}

/** Move motor to position */
void StepMotor::move_to(unit_t position, unit_t velocity) {
    agent.move_to(config->units_to_steps(position), velocity);
}

/** Move motor to position with this velocity */
void StepMotor::move_to_rel(steps_t _position, unit_t velocity) {
    agent.move_to(position + _position, velocity);
}

/** Move motor to position */
void StepMotor::move_to_rel(unit_t _position, unit_t velocity) {
    agent.move_to(position + config->units_to_steps(_position), velocity);
}
// ==================================================
// The homing process
// ==================================================

#define HOMING_TASK_PRIO 2

void StepMotor::homing_task() {
    ESP_LOGI(TAG, "Start Homing...  (homing_dir=%d)",config->homing_dir);
    auto retract = abs(config->homing_retract_dist);
    auto maxpos = abs(config->position_max);
    uint64_t delay = 1 / portTICK_PERIOD_MS;
    // prehoming
    ESP_LOGI(TAG, "Homing [Prehoming]");
    move_to_rel((unit_t)(retract * -config->homing_dir), config->homing_speed);
    while (agent.moving)
        vTaskDelay(delay);

    // homing 1
    ESP_LOGI(TAG, "Homing [Homing 1]");
    move_to_rel((unit_t)(5 * maxpos*config->homing_dir), config->homing_speed);
    agent.set_test_endpoint(true);
    while (!get_endpoint())
        vTaskDelay(delay);

    // retract
    ESP_LOGI(TAG, "Homing [Retract]");
    move_to_rel((unit_t)(retract * -config->homing_dir), config->second_homing_speed);
    while (agent.moving)
        vTaskDelay(delay);

    // homing 2
    ESP_LOGI(TAG, "Homing [Homing 2]");
    move_to_rel((unit_t)(2 * retract * config->homing_dir), config->homing_speed);
    agent.set_test_endpoint(true);
    while (agent.moving)
        vTaskDelay(delay);
    // done
    ESP_LOGI(TAG, "Homing [Done]");
    position = config->position_endstop;
    homing_task_handle = NULL;
    vTaskDelete(NULL);
}

void StepMotor::move_to_home() {
    ESP_LOGI(TAG, "Start homing task");
    if (homing_task_handle != NULL) {
        vTaskDelete(homing_task_handle);
        homing_task_handle = NULL;
    }
    xTaskCreate(c_homing_task, "homing_task", 4096, this, HOMING_TASK_PRIO, &homing_task_handle);
}

