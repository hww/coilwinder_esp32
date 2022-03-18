#ifndef GPIO_TOOLS_H_
#define GPIO_TOOLS_H_

#include <driver/gpio.h>

void set_gpio_mode(gpio_num_t gpionum, gpio_mode_t gpiomode, int gpioval);
void set_gpio_oc(gpio_num_t pin);
void set_gpio_out(gpio_num_t pin);
void set_gpio_in(gpio_num_t pin);
bool set_gpio(gpio_num_t pin, int value);
void toggle_gpio(gpio_num_t pin);



#endif // GPIO_TOOLS_H_
