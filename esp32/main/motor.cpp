#include "motor.h"
#include <freertos/FreeRTOS.h>

#define STEPS_PER_MM (40.0f/6400.0f)

#define DELAY_BETWEEN_STEPS 10000

/* Global control for RPM */
int g_main_rpm = 0;

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

/* The @arg points to motor_t structure */
static void timer_handler(void *arg)
{
    motor_t* motor = (motor_t*)arg;
    if (motor->timer == 0)
      TIMERG0.int_clr_timers.t0 = 1;
    else if (motor->timer == 1)
      TIMERG0.int_clr_timers.t1 = 1;
    else
      assert(false);
    TIMERG0.hw_timer[motor->timer].update = 1;
    TIMERG0.hw_timer[motor->timer].config.alarm_en = 1;

    if(g_main_rpm > 0)
    {
      if (motor->cnt_ena && motor->cnt <= 0) {
        motor->cnt = motor->coil_length_steps;
        toggle_pin(motor->dir_pin);
        motor->status = !motor->status;
      }
      gpio_set_level(motor->step_pin, 1);
      ets_delay_us(1);
      gpio_set_level(motor->step_pin, 0);
      if (motor->cnt_ena)
        motor->cnt--;
    }
}

static esp_err_t config_timer(timer_group_t tgrp, timer_idx_t tmr, void (*fn)(void*), void * arg)
{
  esp_err_t res;

  timer_config_t timerCfg = {
    TIMER_ALARM_EN,
    TIMER_PAUSE,
    TIMER_INTR_LEVEL,
    TIMER_COUNT_UP,
    TIMER_AUTORELOAD_EN,
    2 }; // divider

  if((res = timer_init(tgrp, tmr, &timerCfg)) == ESP_OK) {
    printf("TIMER initialized!\n");
  } else {
    printf("TIMER not initialized!\n");
    return res;
  }

  timer_set_alarm_value(tgrp, tmr, DELAY_BETWEEN_STEPS);
  timer_set_counter_value(tgrp, tmr, 0);

  timer_enable_intr(tgrp, tmr);

  timer_isr_handle_t *handle = 0;
  if((res = timer_isr_register(tgrp, tmr, fn, arg, 0, handle)) == ESP_OK) {
    printf("TIMER isr registered!!! :)\n");
  } else {
    printf("TIMER isr not registered!\n");
    return res;
  }

  if((res = timer_start(tgrp, tmr)) == ESP_OK) {
    printf("TIMER started!\n");
  } else {
    printf("TIMER not started!\n");
    return res;
  }
  return ESP_OK;
}

void motor_set_step_duration(motor_t& motor, int stepDelay)
{
  timer_set_alarm_value(TIMER_GROUP_0, motor.timer, stepDelay);
}

void motor_init(motor_t& motor)
{
    enable_gpio(motor.step_pin);
    enable_gpio(motor.dir_pin);

    if(config_timer(TIMER_GROUP_0, motor.timer, timer_handler, &motor) == ESP_OK) {
      printf("timer initialized!\n");
    }
    gpio_set_level(motor.dir_pin, 1);
}
