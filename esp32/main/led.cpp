#include "led.h"


void led_init(led_t& led)
{
    led.config = {};
    led.config.gpio_num = led.led_pin;
    led.config.speed_mode = LEDC_HIGH_SPEED_MODE;
    led.config.channel = led.channel;
    led.config.intr_type = LEDC_INTR_DISABLE;
    led.config.timer_sel = LEDC_TIMER_2;
    led.config.duty = 0;

    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    // Deprecated!
    //ledc_timer.bit_num = LEDC_TIMER_10_BIT;
    ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
    ledc_timer.timer_num = LEDC_TIMER_2;
    ledc_timer.freq_hz = 500;

    ledc_channel_config(&led.config);
    ledc_timer_config(&ledc_timer);
}


void led_set(uint32_t duty)
{
  //printf("max duty: %d, duty: %d\n", MAX_DUTY, duty);

  #ifdef INV_OUT
    duty = MAX_DUTY - duty;
  #endif

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}
