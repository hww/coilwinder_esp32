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
#include "menu_system.h"
#include "motor.h"

#define TAG "menu"


static QueueHandle_t event_queue;
static rotary_encoder_info_t info;

static void gpio_setup(int gpioNum, int gpioMode, int gpioVal);

// ==============================================================================
// GPIO
// ==============================================================================

static void gpio_setup(int gpioNum, int gpioMode, int gpioVal) {
    auto gpioNumNative = (gpio_num_t)(gpioNum);
    auto gpioModeNative = (gpio_mode_t)(gpioMode);
    gpio_pad_select_gpio(gpioNumNative);
    gpio_set_direction(gpioNumNative, gpioModeNative);
    gpio_set_level(gpioNumNative, gpioVal);
    gpio_set_pull_mode(gpioNumNative, GPIO_PULLUP_ONLY);
}

// ==============================================================================
// Encoder API
// ==============================================================================

//    ROTARY_ENCODER_DIRECTION_NOT_SET = 0,
//    ROTARY_ENCODER_DIRECTION_CLOCKWISE,
//    ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE,
int get_encoder_evt_dir (rotary_encoder_state_t& state) {
    if (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE)
        return 1;
    if (state.direction == ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE)
        return -1;
    return 0;
}

int get_encoder_dir()
{
    rotary_encoder_state_t state = { 0, ROTARY_ENCODER_DIRECTION_NOT_SET  };
    ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
    if (state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE)
        return 1;
    if (state.direction == ROTARY_ENCODER_DIRECTION_COUNTER_CLOCKWISE)
        return -1;
    return 0;
}

int get_encoder_pos()
{
    rotary_encoder_state_t state = { 0, ROTARY_ENCODER_DIRECTION_NOT_SET };
    ESP_ERROR_CHECK(rotary_encoder_get_state(&info, &state));
    return state.position;
}


// The top level menu
Menu* main_menu = nullptr;
// The current menu
Menu* current_menu = nullptr;
// The true value when editing value mode
bool menu_edit_mode = false;

void open_menu(Menu* menu) {
    current_menu = menu;
    current_menu->m_line_num = 0;
}

/** Rendering the screen content */
void render_screen()
{
    display_clear();
    if (current_menu == nullptr)
        open_menu(main_menu);
    current_menu->display_menu(menu_edit_mode);
}

// ==============================================================================
// The menu system
// ==============================================================================

// Переменные изменяемые на экране
int shaft_pos, layer_pos;
int turns = 1, step = 1, speed=1, layers=1;


// ==============================================================================
// Initializer
// ==============================================================================
/**
 * Menu Initialization
 **/
void menu_init()
{
    // esp32-rotary-encoder requires that the GPIO ISR service is installed before calling rotary_encoder_register()
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    // Initialise the rotary encoder device with the GPIOs for A and B signals
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ROT_ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif

    // Create a queue for events from the rotary encoder driver.
    // Tasks can read from this queue to receive up to date position information.
    event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));

    // Config buttons
    gpio_setup(GP_BUTTON_ROT, GPIO_MODE_INPUT, 1);
    gpio_setup(GP_BUTTON_A, GPIO_MODE_INPUT, 1);
    gpio_setup(GP_BUTTON_B, GPIO_MODE_INPUT, 1);

    // Config the menu
    main_menu = new Menu("Menu...", nullptr);
    auto coil_menu = new Menu("Coil...", nullptr);
    main_menu->Add(coil_menu);
    coil_menu->Add(main_menu);
    coil_menu->Add(new IntItem("Turns", &turns, 0,10000,1));
    coil_menu->Add(new IntItem("Layers", &layers, 0,10000,1));

    auto motor_v_menu = new Menu("Motor Velocity...", nullptr);
    main_menu->Add(motor_v_menu);
    motor_v_menu->Add(main_menu);
    motor_v_menu->Add(new FloatItem("x vel.", &motor_x.velocity, -motor_x.max_velocity,motor_x.max_velocity, 5));
    motor_v_menu->Add(new FloatItem("y vel.", &motor_y.velocity, -motor_y.max_velocity,motor_y.max_velocity, 5));

    auto motor_speed_menu = new Menu("Speed...", nullptr);
    main_menu->Add(motor_speed_menu);
    motor_speed_menu->Add(main_menu);
    motor_speed_menu->Add(new FloatItem("Speed", &g_speed, 0, 1, 0.01));

    render_screen();
    display_update();

}

/** Return the button is pressed and update previous state */
bool get_press(gpio_num_t gpio, bool& old_state) {
    bool state = !gpio_get_level(gpio);
    bool press = state && !old_state;
    old_state = state;
    return press;
}

static bool button_rot;
static bool button_a;
static bool button_b;

/**
 * Update menu every frame
 **/
void menu_update()
{
    if (get_press(GP_BUTTON_ROT, button_rot))
    {
        MenuEvent evt;
        evt.etype = MenuEventType::OnPress;
        evt.eres = MenuEventRes::Nop;
        evt.edit = menu_edit_mode;
        current_menu->menu_event(evt);
        switch (evt.eres) {
            case MenuEventRes::EditStart:
                menu_edit_mode = true;
                break;
            case MenuEventRes::EditStop:
                menu_edit_mode = false;
                break;
            case MenuEventRes::MenuOpen:
                open_menu(evt.menu);
                break;
            default:
                break;
        }
        render_screen();
        display_update();
        return;
    }

    // Wait for incoming events on the event queue.
    rotary_encoder_event_t event = {0, ROTARY_ENCODER_DIRECTION_NOT_SET };
    if (xQueueReceive(event_queue, &event, 1 / portTICK_PERIOD_MS) == pdTRUE)
    {
        int dir = get_encoder_evt_dir(event.state);

        if (dir != 0) {
            MenuEvent evt;
            evt.etype = MenuEventType::OnRotate;
            evt.eres = MenuEventRes::Nop;
            evt.dir = dir;
            evt.edit = menu_edit_mode;
            current_menu->menu_event(evt);
            switch (evt.eres) {
                case MenuEventRes::EditStart:
                    menu_edit_mode = true;
                    break;
                case MenuEventRes::EditStop:
                    menu_edit_mode = false;
                    break;
                case MenuEventRes::MenuOpen:
                    open_menu(evt.menu);
                    break;
                default:
                    break;
            }
            render_screen();
            display_update();

        }
    }
}
