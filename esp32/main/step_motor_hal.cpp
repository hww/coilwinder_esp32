#include "config.h"
#include "step_motor_hal.h"
#include "step_motor.h"
#include "step_motor_config.h"
#include "gpiolib.h"

static const char TAG[] = "motor-hal";

/** ******************************************/
/** The GPIO driver of the StepMotor         */
/** ******************************************/

StepMotorHAL::StepMotorHAL() {}

void StepMotorHAL::init(StepMotor* motor) {
  id = motor->id;
  config = motor->config;
  ESP_LOGI(TAG, "[%d] Initialize step motor", id);
  set_gpio_oc(config->step_pin);
  set_gpio_oc(config->dir_pin);
  set_gpio_oc(config->enable_pin);
  set_gpio_in(config->endstop_pin);
}
bool StepMotorHAL::get_enable() {
  if (config->enable_pin != GPIO_NULL)
    return (bool)gpio_get_level(config->enable_pin) != config->enable_pin_reverse;

  return false;
}
void StepMotorHAL::set_enable(bool v) {
  if (config->enable_pin != GPIO_NULL)
    set_gpio(config->enable_pin, v != config->enable_pin_reverse);
}
bool StepMotorHAL::get_direction() {
  if (config->dir_pin != GPIO_NULL)
    return (bool)gpio_get_level(config->dir_pin) != config->dir_pin_reverse;

  return false;
}
void StepMotorHAL::set_direction(bool v) {
  if (config->dir_pin != GPIO_NULL)
    set_gpio(config->dir_pin, v != config->dir_pin_reverse);
}
void StepMotorHAL::set_step(bool v) {
  if (config->step_pin != GPIO_NULL)
    set_gpio(config->step_pin, v != config->step_pin_reverse);
}
bool StepMotorHAL::get_endpoint() {
  if (config->endstop_pin != GPIO_NULL)
    return (bool)gpio_get_level(config->endstop_pin) != config->endstop_pin_reverse;

  return false;
}
