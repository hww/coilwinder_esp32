#pragma once

#include <string>
#include <stdint.h>
#include "esp_timer.h"
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "gpiolib.h"
#include "step_motor_hal.h"
#include "typeslib.h"

// Default iterrupt time when no othe
#define TIMER_IDLE_DELAY_US 20000
// Creating and starting a timer, and dispatching the callback
// takes some timehere is a lower limit to the timeout value
// of one-shot esp_timer. If esp_timer_start_once() is called
// with a timeout value less than 20us, the callback will be
// dispatched only after approximately 20us.
#define MINIMUM_TIMER_INTERVAL_US 20
// Do not allow the steps with time more than this
#define MAXIMUM_TIMER_INTERVAL_US 10000000

class Menu;
class StepMotor;
class StepMotorConfig;
class ShepMotorHAL;

/** Move the motor to target position with given velocity */
class StepMotorAgent {
 public:
  int id;
  StepMotor* motor;
  StepMotorConfig* config;
  bool moving;
  steps_t target;
  unit_t velocity;

  StepMotorAgent();
  void init(StepMotor* motor);
  void move_to(unit_t pos, unit_t velocity);
  void move_to(steps_t pos, unit_t velocity);
  void stop();
  void on_step();
};

class StepMotor  {
  public:
  StepMotor();

  void init(StepMotorConfig* conf);
  void init_menu(Menu* menu, std::string name);
  void update(float time);

  unit_t get_target_position();
  void set_target_position(unit_t vel);
  unit_t get_target_velocity();
  void set_target_velocity(unit_t vel);
  bool get_enable();
  void set_enable(bool v);
  bool get_endpoint();
  bool is_moving_home();

  bool verify_timer_interval(uint64_t &interval);

  void update_velocity(float time);

  void move_to(steps_t positin, unit_t velocity);
  void move_to(unit_t position, unit_t velocity);
  void move_to_home();
  void homing_task();

  void isr();


  /** The motor's ID */
  int id;
  StepMotorConfig* config;

  /** The hgardware abstraction layer */
  StepMotorHAL hal;
  StepMotorAgent agent;

  /** Control containes requested state */
  unit_t target_velocity;
  bool enabled;

  /** Status display the actual state of motor */
  unit_t velocity;
  steps_t position;

  /** System */
  int log;
  uint32_t isr_count;
  esp_timer_handle_t timer_handle;
  esp_timer_create_args_t timer_arg;
  float previous_update_at;
  uint64_t timer_interval_us;
  float delta_time;

  /** Threads */
  TaskHandle_t homing_task_handle;

};
