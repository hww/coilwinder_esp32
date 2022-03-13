#include "motor.h"
#include <freertos/FreeRTOS.h>
#include <math.h>
#include "esp_log.h"

#include "config.h"
#include "esp_timer.h"
#include "time.hpp"

static const char* TAG = "motor";

/* Global control for RPM */
float g_speed = 1;

// Hardware timer clock divider
#define TIMER_DIVIDER (2)
// Convert counter value to seconds
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)
// Default iterrupt time
#define TIMER_IDLE_DELAY_US 20000
// Calculate the timer value with delay seconds
#define TIMER_VALUE(sec) ((uint64_t)(sec * TIMER_SCALE))

// ==================================================
// GPIO Tools
// ==================================================

static void enable_gpio(gpio_num_t pin) {
  gpio_pad_select_gpio(pin);
  gpio_set_direction(pin, static_cast<gpio_mode_t>(GPIO_MODE_INPUT_OUTPUT));
  gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
  gpio_set_level(pin, 0);
}

static void toggle_pin(gpio_num_t pin)
{
  int level = gpio_get_level(pin);
  level = !level;
  gpio_set_level(pin, level);
}

// ==================================================
// Velocity control
// ==================================================

const float EPSILON = 0.000001;

static inline bool inexact_compare(float x, float y) {
  return ((fabs(x) - fabs(y)) < EPSILON);
}

/** make current velocity ecual to desired velocity */
static void motor_interpolate_velocity(motor_t& motor, float dt)
{
  // apply global speed
  auto velocity = motor.velocity * g_speed;
  // compute direction
  auto veldif = velocity - motor.current_velocity;
  auto accdir = inexact_compare(veldif, 0) ? 0 : (veldif > 0 ? 1 : -1);
  // compute acceleration
  auto accel = motor.max_accel * dt;
  if (fabs(veldif) < accel) {
    motor.current_velocity = velocity;
  } else {
    // apply acceleration to velocity
    motor.current_velocity += accel * accdir;
    // limit velocity
    if (accdir > 0) {
      if (motor.current_velocity > motor.max_velocity)
        motor.current_velocity = motor.max_velocity;
    } else if (accdir < 0) {
      if (motor.current_velocity < -motor.max_velocity)
        motor.current_velocity = -motor.max_velocity;
    }
  }
  // update direction of moving
  motor.current_dir = motor.current_velocity == 0 ? 0 : (motor.current_velocity > 0 ? 1 : -1);
  if (motor.current_dir == 0) {
    motor.timer_interval = TIMER_IDLE_DELAY_US;
  } else {
    int total_steps_per_turn = motor.steps_per_turn * motor.microsteps;
    float rot_per_sec = fabs(motor.current_velocity) / motor.rotation_distance;
    float steps_per_sec = rot_per_sec * total_steps_per_turn;
    float step_delay_s = 1.0f / steps_per_sec;
    motor.timer_interval = ((double)step_delay_s * 1000000);
  }
  //printf("%lld\n", motor.timer_interval);
}

// ==================================================
// Timer ISR
// ==================================================

/* The @arg points to motor_t structure */
static void  timer_isr(void *arg)
{
  motor_t* motor = (motor_t*)arg;
  motor->isr_count++;
  // Update motor's velocity
  if (motor->enabled)
  {
    motor->time.update();
    motor_interpolate_velocity(*motor, motor->time.m_delta_time);
    if(motor->current_velocity != 0)
    {
      auto old_dir = gpio_get_level(motor->dir_pin) ^ motor->dir_pin_reverse;
      auto new_dir = motor->current_dir > 0 ? 1 : 0;
      gpio_set_level(motor->dir_pin, new_dir ^ motor->dir_pin_reverse);
      if (old_dir != new_dir)
        ets_delay_us(1);
      // make a step to the current direction
      gpio_set_level(motor->step_pin, 1 ^ motor->step_pin_reverse);
      ets_delay_us(1);
      gpio_set_level(motor->step_pin, 0 ^ motor->step_pin_reverse);
      // compure the position in units
      motor->current_position_steps += motor->current_dir;
      motor->current_position = motor->current_position_steps * motor->distance_per_step;
    }
  }
  esp_timer_stop(motor->timer_handle);
  esp_timer_start_once(motor->timer_handle, motor->timer_interval);
}

void motor_init(motor_t& motor)
{
  ESP_LOGI(TAG, "[%d] Initialize motor", motor.id);
  enable_gpio(motor.step_pin);
  enable_gpio(motor.dir_pin);
  gpio_set_level(motor.dir_pin, 1);

  // Configure motor
  motor.distance_per_step = motor.rotation_distance / (motor.microsteps * motor.steps_per_turn);

  // Configure timer
  motor.timer_arg.callback = &timer_isr;
  motor.timer_arg.arg = &motor;

  ESP_ERROR_CHECK(esp_timer_create(&motor.timer_arg, &motor.timer_handle));
  /* The timer has been created but is not running yet */

  /* Start the timers */
  ESP_ERROR_CHECK(esp_timer_start_periodic(motor.timer_handle, TIMER_IDLE_DELAY_US));
}

void motor_reset_status(motor_t& motor)
{
  // Control containes requested state
  motor.enabled = false;
  motor.velocity = 0;
  motor.position = 0;
  // Status display the actual state of motor
  motor.current_velocity = 0;
  motor.current_position = 0;
  motor.current_position_steps = 0;
  motor.current_dir = 0;
  motor.complete = true;
}

void motor_update(motor_t &motor) {
  if (motor.id == 0)
  {
 //    printf("id: %d, isr: %d velocity: %f, cur-velocity: %f\n", motor.id, motor.isr_count, motor.velocity, motor.current_velocity);
  }
}

// ==============================================================================
// Motors
// ==============================================================================

motor_t motor_x;
motor_t motor_y;

void kinematic_init()
{
  motor_reset_status(motor_x);
  motor_reset_status(motor_y);
  /** Y */
  motor_x.id = 0;
  // The driver settings
//  motor_x.timer = TIMER_0;   // TIMER0 or TIMER1
  //motor_x.timer_group = TIMER_GROUP_0;
  // GPIO
  motor_x.step_pin = MOTOR_X_STEP_PIN;
  motor_x.dir_pin = MOTOR_X_DIR_PIN;
  motor_x.enable_pin = MOTOR_X_ENABLE_PIN;
  motor_x.endstop_pin = MOTOR_X_ENDSTOP_PIN;
  motor_x.dir_pin_reverse = MOTOR_X_DIR_PIN_REVERSE;
  motor_x.step_pin_reverse = MOTOR_X_STEP_PIN_REVERSE;
  // Kinematic
  motor_x.max_velocity = MOTOR_X_MAX_VELOCITY;
  motor_x.max_accel = MOTOR_X_MAX_ACCELERATION;
  motor_x.rotation_distance = MOTOR_X_ROTATION_DISTANCE;
  motor_x.microsteps = MOTOR_X_MICROSTEPS;
  motor_x.steps_per_turn = MOTOR_STEPS_PER_TURN;
  motor_x.position_min = MOTOR_X_POSITION_MIN;
  motor_x.position_endstop = MOTOR_X_POSITION_ENDSTOP;
  motor_x.position_max = MOTOR_X_POSITION_MAX;
  motor_x.homing_speed = MOTOR_X_HOMING_SPEED;
  motor_x.homing_retract_dist = MOTOR_X_HOMING_RETRACT_DIST;
  motor_x.second_homing_speed = MOTOR_X_SECOND_HOMING_SPEED;
  /** Y */
  motor_y.id = 1;
  // The driver settings
  //motor_y.timer = TIMER_1;   // TIMER0 or TIMER1
  //motor_y.timer_group = TIMER_GROUP_0;
  // GPIO
  motor_y.step_pin = MOTOR_Y_STEP_PIN;
  motor_y.dir_pin = MOTOR_Y_DIR_PIN;
  motor_y.enable_pin = MOTOR_Y_ENABLE_PIN;
  motor_y.endstop_pin = MOTOR_Y_ENDSTOP_PIN;
  motor_y.dir_pin_reverse = MOTOR_Y_DIR_PIN_REVERSE;
  motor_y.step_pin_reverse = MOTOR_Y_STEP_PIN_REVERSE;
  // Kinematic
  motor_y.max_velocity = MOTOR_Y_MAX_VELOCITY;
  motor_y.max_accel = MOTOR_Y_MAX_ACCELERATION;
  motor_y.rotation_distance = MOTOR_Y_ROTATION_DISTANCE;
  motor_y.microsteps = MOTOR_Y_MICROSTEPS;
  motor_y.steps_per_turn = MOTOR_STEPS_PER_TURN;
  motor_y.position_min = MOTOR_Y_POSITION_MIN;
  motor_y.position_endstop = MOTOR_Y_POSITION_ENDSTOP;
  motor_y.position_max = MOTOR_Y_POSITION_MAX;
  motor_y.homing_speed = MOTOR_Y_HOMING_SPEED;
  motor_y.homing_retract_dist = MOTOR_Y_HOMING_RETRACT_DIST;
  motor_y.second_homing_speed = MOTOR_Y_SECOND_HOMING_SPEED;

  motor_init(motor_x);
  motor_init(motor_y);

  motor_x.enabled = motor_y.enabled = true;
}


void kinematic_update() {
  motor_update(motor_x);
  motor_update(motor_y);
}
