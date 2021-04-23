#include "menu.h"

#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include <driver/gpio.h>
#include "rotary_encoder.h"

#define TAG "menu"

// ==============================================================================
// ROTARY ENCODER
// ==============================================================================

#define ROT_ENC_A_GPIO GPIO_NUM_16
#define ROT_ENC_B_GPIO GPIO_NUM_17
#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION    false  // Set to true to reverse the clockwise/counterclockwise sense

// Additional button
#define BUTTON_GPIO GPIO_NUM32
#define AUX_BUTTON_GPIO GPIO_NUM33

static QueueHandle_t event_queue;
static rotary_encoder_info_t info;

/**
 * Menu Initialization
 **/
void menu_init()
{
    // esp32-rotary-encoder requires that the GPIO ISR service is installed before calling rotary_encoder_register()
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Initialise the rotary encoder device with the GPIOs for A and B signals
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
}

/**
 * Update menu every frame
 **/
void menu_update()
{
    // Wait for incoming events on the event queue.
    rotary_encoder_event_t event = {0, ROTARY_ENCODER_DIRECTION_NOT_SET };
    if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
    {
        ESP_LOGI(TAG, "Event: position %d, direction %s", event.state.position,
                 event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
    }
    else
    {
        // Poll current position and direction
        rotary_encoder_state_t state = { 0 };
        ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
        ESP_LOGI(TAG, "Poll: position %d, direction %s", state.position,
                 state.direction ? (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");

        // Reset the device
        if (RESET_AT && (state.position >= RESET_AT || state.position <= -RESET_AT))
        {
            ESP_LOGI(TAG, "Reset");
            ESP_ERROR_CHECK(rotary_encoder_reset(&info));
        }
    }
}
