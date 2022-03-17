#include <math.h>
#include <stdlib.h>
#include <string>

#include "esp_timer.h"

#include "config.h"
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

StepMotorAgent::StepMotorAgent()
  : id(0)
  , motor()
  , moving(false)
  , target(0)
  , velocity(0)
{}

void StepMotorAgent::init(StepMotor* _motor) {
  motor = _motor;
  id = motor->id;
  config = motor->config;
}

void StepMotorAgent::move_to(steps_t pos, unit_t _velocity) {
  target = pos;
  velocity = _velocity;
  on_step();
}

void StepMotorAgent::on_step() {
  if (motor == nullptr)
    return;

  if (moving) {
    if (target < motor->position) {
      motor->set_target_velocity(velocity);
    } else if (target > motor->position) {
      motor->set_target_velocity(-velocity);
    } else {
      moving = false;
      ESP_LOGI(TAG, "[%d] Moving complete", motor->id);
    }
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
void StepMotor::init_menu(Menu* parent, std::string name) {
  /*
  auto menu = new Menu(parent, name);
  parent_menu->Add(menu);
  menu->add(parent_menu);
  menu->add(new FloatItem("vel.", []() {
    return GetTargetVelocity();
  }, [](float v) {
    SetTargetVelocity(v);
  }));
  auto pos_01 = new FloatItem("Pos+-1", []() {
    return GetTargetPosition();
  }, [](float v) {
    SetTargetPosition(v);
  });
  auto pos_10 = new FloatItem("Pos+-10", []() {
    return GetTargetPosition();
  }, [](float v) {
    SetTargetPosition(v);
  });
  pos_01.Step(1);
  pos_10.Step(10);
  menu->Add(pos_01);
  menu->Add(pos_10);
  */
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

void StepMotor::set_target_velocity(unit_t vel) {
  target_velocity = vel;
  if (vel == 0)
    velocity = 0;
}
bool StepMotor::get_enable() {
  return hal.get_enable();
}
void StepMotor::set_enable(bool v) {
  hal.set_enable(v);
}
bool StepMotor::get_endpoint() {
  return hal.get_endpoint();
}

bool StepMotor::is_moving_home() {
  return get_direction(target_velocity) == get_direction(config->homing_dir);
}

// ==================================================
// Convert units
// ==================================================

/** Return true if sucesfully convert velocity to timer inteval */
bool StepMotor::verify_timer_interval(uint64_t &interval) {
  if (interval == 0) {
    interval = TIMER_IDLE_DELAY_US;
    ESP_LOGE(TAG, "[%d] The timer interval is zero", id);
    return false;
  } else if (interval > MAXIMUM_TIMER_INTERVAL_US) {
    interval = MAXIMUM_TIMER_INTERVAL_US;
    ESP_LOGE(TAG, "[%d] The timer interval is too big", id);
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

  if (fabs(veldif) < accel) {
    velocity = target_velocity;
  } else {
    // apply acceleration to velocity
    velocity += accel * accdir;

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
    auto steps_per_sec = config->units_to_steps(velocity);
    timer_interval_us = (uint64_t)((double)1.0 / steps_per_sec * 1000000);
    verify_timer_interval(timer_interval_us);

    if (log > 2)
      printf("[%d] tgt-vel: %f vel: %f interval:  %lf\n", id, target_velocity, velocity,(double)timer_interval_us);

    // Restart the timer
    esp_timer_stop(timer_handle);
    esp_timer_start_once(timer_handle, timer_interval_us);
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

  if (velocity != 0) {
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

    if (hal.get_endpoint())
      agent.stop();
  }
}

// ==================================================
// Moving to the target point
// ==================================================

/** Move motor to position with this velocity */
void StepMotor::move_to(steps_t position, unit_t velocity) {
  agent.move_to(position, velocity);
}

/** Move motor to position */
void StepMotor::move_to(unit_t position, unit_t velocity) {
  agent.move_to(config->units_to_steps(position), velocity);
}

// ==================================================
// The homing process
// ==================================================


#define HOMING_TASK_PRIO 2

void StepMotor::homing_task() {
  ESP_LOGI(TAG, "Start Homing...");
  auto retract_steps = config->units_to_steps(config->homing_retract_dist);
  // prehoming
  ESP_LOGI(TAG, "Homing [Prehoming]");
  auto target = position - retract_steps * config->homing_dir;
  move_to(target, config->homing_speed);
  ets_delay_us(1000);

  while (agent.moving)
    ets_delay_us(1000);

  // homing 1
  ESP_LOGI(TAG, "Homing [Homing 1]");
  target = position + retract_steps * config->homing_dir;
  move_to(target, config->homing_speed);
  ets_delay_us(1000);

  while (!get_endpoint())
    ets_delay_us(1000);

  // retract
  ESP_LOGI(TAG, "Homing [Retract]");
  target = position - retract_steps * config->homing_dir;
  move_to(target, config->second_homing_speed);
  ets_delay_us(1000);

  while (agent.moving)
    ets_delay_us(1000);

  // homing 2
  ESP_LOGI(TAG, "Homing [Homing 2]");
  target = position + retract_steps * config->homing_dir;
  move_to(target, config->second_homing_speed);
  ets_delay_us(1000);

  while (!get_endpoint())
    ets_delay_us(1000);

  // done
  ESP_LOGI(TAG, "Homing [Done]");
  position = config->position_endstop;
  vTaskDelete(NULL);
}

void StepMotor::move_to_home() {
  ESP_LOGI(TAG, "Start homing task");
  if (homing_task_handle != NULL)
    vTaskDelete(homing_task_handle);
  xTaskCreate(c_homing_task, "homing_task", 4096, this, HOMING_TASK_PRIO, &homing_task_handle);
}

