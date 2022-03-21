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
#include "menu_event.h"
#include "rotary_encoder.h"
#include "macro.h"
#include "display.h"
#include "menu.h"
#include "menu_system.h"
#include "gpiolib.h"

#define TAG "menu_controller"

static QueueHandle_t encoder_queue;
static rotary_encoder_info_t encoder_info;

static int delta_position = 0;
static int log = 0;

// ==============================================================================
// Forward declration
// ==============================================================================


void input_controller_update();

bool get_key(Button e);
bool get_key_up(Button e);
bool get_key_down(Button e);

static int get_encoder_evt_dir (rotary_encoder_state_t &state);
static void update_button(Button e, gpio_num_t gpio);
static void update_button(Button e, bool state);
#ifdef DIRECT_ACESS_TO_QUAD_ENCODER
static int get_encoder_dir();
static int get_encoder_pos();
#endif

// ==============================================================================
// Initializer
// ==============================================================================

void input_controller_init() {
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
void input_controller_update() {

  update_button(Button::Quad, GP_BUTTON_ROT);
  update_button(Button::A, GP_BUTTON_A);
  update_button(Button::B, GP_BUTTON_B);

  if (input_get_key_down(Button::Quad))
    MenuSystem::instance.on_event(MenuEvent::PressQuad);
  if (input_get_key_down(Button::A))
    MenuSystem::instance.on_event(MenuEvent::PressA);
  if (input_get_key_down(Button::B))
    MenuSystem::instance.on_event(MenuEvent::PressB);

  // Wait for incoming events on the event queue.
  rotary_encoder_event_t event = {0, ROTARY_ENCODER_DIRECTION_NOT_SET };
  delta_position = 0;

  if (xQueueReceive(encoder_queue, &event, 1 / portTICK_PERIOD_MS) == pdTRUE) {
    int dir = get_encoder_evt_dir(event.state);

    if (dir != 0) {
      delta_position += dir;
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

#define MAX_KEY 4

static bool key[MAX_KEY];
static bool key_up[MAX_KEY];
static bool key_down[MAX_KEY];

/** Return the button is pressed and update previous state */
static void update_button(Button e, bool state) {
  bool oldst = key[(int)e];
  key[(int)e] = state;
  key_up[(int)e] = !state && oldst;
  key_down[(int)e] = state && !oldst;
  if (log>0) {
    if (key_down[(int)e])
      ESP_LOGI(TAG, "On Key Down %d", (int)e);
  }
}

/** Return the button is pressed and update previous state */
static void update_button(Button e, gpio_num_t gpio) {
  update_button(e, !gpio_get_level(gpio));
}

bool input_get_key(Button e) { return key[(int)e]; }
bool input_get_key_up(Button e) { return key_down[(int)e]; }
bool input_get_key_down(Button e) { return key_down[(int)e]; }
int input_get_delta_position() { return delta_position; }
