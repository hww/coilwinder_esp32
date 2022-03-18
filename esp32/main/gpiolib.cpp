#include "gpiolib.h"
#include "config.h"

void set_gpio_mode(gpio_num_t gpionum, gpio_mode_t gpiomode, int gpioval) {
  gpio_pad_select_gpio(gpionum);
  gpio_set_direction(gpionum, gpiomode);
  gpio_set_level(gpionum, gpioval);
  gpio_set_pull_mode(gpionum, GPIO_PULLUP_ONLY);
}

void set_gpio_out(gpio_num_t pin) {
  if (pin == GPIO_NULL)
    return;
  gpio_pad_select_gpio(pin);
  gpio_set_direction(pin, static_cast<gpio_mode_t>(GPIO_MODE_OUTPUT));
  gpio_set_level(pin, 1);
}

void set_gpio_oc(gpio_num_t pin) {
  if (pin == GPIO_NULL)
    return;
  gpio_pad_select_gpio(pin);
  gpio_set_direction(pin, static_cast<gpio_mode_t>(GPIO_MODE_INPUT_OUTPUT));
  gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
  gpio_set_level(pin, 1);
}

void set_gpio_in(gpio_num_t pin) {
  if (pin == GPIO_NULL)
    return;
  gpio_pad_select_gpio(pin);
  gpio_set_direction(pin, static_cast<gpio_mode_t>(GPIO_MODE_INPUT));
}

bool set_gpio(gpio_num_t pin, int value)
{
  if (pin == GPIO_NULL)
    return false;
  int oldval = gpio_get_level(pin);
  gpio_set_level(pin, value);
  return oldval != value;
}

void toggle_gpio(gpio_num_t pin)
{
  if (pin == GPIO_NULL)
    return;
  int level = gpio_get_level(pin);
  level = !level;
  gpio_set_level(pin, level);
}


