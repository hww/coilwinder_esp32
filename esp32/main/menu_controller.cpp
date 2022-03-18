#include "menu.h"

#include <stdbool.h>
#include <string>
#include <array>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include <driver/gpio.h>
#include "hal/gpio_types.h"

#include "config.h"
#include "rotary_encoder.h"
#include "macro.h"
#include "display.h"
#include "menu.h"
#include "menu_system.h"
#include "gpiolib.h"

#define TAG "menu_controller"

static QueueHandle_t encoder_queue;
static rotary_encoder_info_t encoder_info;
static bool button_rot;
static bool button_a;
static bool button_b;

// ==============================================================================
// Forward declration
// ==============================================================================

void menu_controller_update();

static int get_encoder_evt_dir (rotary_encoder_state_t &state);
static bool get_press(gpio_num_t gpio, bool &old_state);
#ifdef DIRECT_ACESS_TO_QUAD_ENCODER
static int get_encoder_dir();
static int get_encoder_pos();
#endif

// ==============================================================================
// Initializer
// ==============================================================================

void menu_controller_init() {
  // esp32-rotary-encoder requires that the GPIO ISR service is installed before calling rotary_encoder_register()
  ESP_ERROR_CHECK(gpio_install_isr_service(0));
  // Initialise the rotary encoder device with the GPIOs for A and B signals
  ESP_ERROR_CHECK(rotary_encoder_init(&encoder_info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
  ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&encoder_info, ROT_ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
  ESP_ERROR_CHECK(rotary_encoder_flip_direction(&encoder_info));
#endif
  // Create a queue for events from the rotary encoder driver.
  // Tasks can read from this queue to receive up to date position encoder_information.
  encoder_queue = rotary_encoder_create_queue();
  ESP_ERROR_CHECK(rotary_encoder_set_queue(&encoder_info, encoder_queue));
  // Config buttons
  set_gpio_mode(GP_BUTTON_ROT, GPIO_MODE_INPUT, 1);
  set_gpio_mode(GP_BUTTON_A, GPIO_MODE_INPUT, 1);
  set_gpio_mode(GP_BUTTON_B, GPIO_MODE_INPUT, 1);
}

/**
 * Update menu every frame
 **/
void menu_controller_update() {
  auto press_rot = get_press(GP_BUTTON_ROT, button_rot);
  auto press_a = get_press(GP_BUTTON_A, button_a);
  auto press_b = get_press(GP_BUTTON_B, button_b);

  if (press_rot)
    MenuSystem::instance.on_event(MenuEvent::PressQuad);

  // Wait for incoming events on the event queue.
  rotary_encoder_event_t event = {0, ROTARY_ENCODER_DIRECTION_NOT_SET };

  if (xQueueReceive(encoder_queue, &event, 1 / portTICK_PERIOD_MS) == pdTRUE) {
    int dir = get_encoder_evt_dir(event.state);

    if (dir != 0) {
      if (MenuSystem::instance.is_edit) {
        auto evt = dir>0 ? MenuEvent::Right : MenuEvent::Left;
        MenuSystem::instance.on_event(evt);
      } else {
        auto evt = dir>0 ? MenuEvent::Down : MenuEvent::Up;
        MenuSystem::instance.on_event(evt);
      }
    }
  }
}

// ==============================================================================
// Encoder API
// ==============================================================================
// The rotary_encoder_state_t& state can be one of the next values
//    ROTARY_ENCODER_DIRECTION_NOT_SET = 0,
//    ROTARY_ENCODER_DIRECTION_CLOCKWISE,
//    ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE,
// ==============================================================================


static int get_encoder_evt_dir (rotary_encoder_state_t &state) {
  if (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE)
    return 1;

  if (state.direction == ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE)
    return -1;

  return 0;
}

#ifdef DIRECT_ACESS_TO_QUAD_ENCODER

static int get_encoder_dir() {
  rotary_encoder_state_t state = { 0, ROTARY_ENCODER_DIRECTION_NOT_SET  };
  ESP_ERROR_CHECK(rotary_encoder_get_state(&encoder_info, &state));

  if (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE)
    return 1;

  if (state.direction == ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE)
    return -1;

  return 0;
}

 static int get_encoder_pos() {
  rotary_encoder_state_t state = { 0, ROTARY_ENCODER_DIRECTION_NOT_SET };
  ESP_ERROR_CHECK(rotary_encoder_get_state(&encoder_info, &state));
  return state.position;
}

#endif

// ==============================================================================
// The menu system
// ==============================================================================

/** Return the button is pressed and update previous state */
static bool get_press(gpio_num_t gpio, bool &old_state) {
  bool state = !gpio_get_level(gpio);
  bool press = state && !old_state;
  old_state = state;
  return press;
}
