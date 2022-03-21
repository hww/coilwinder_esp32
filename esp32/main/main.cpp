/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string>
#include "menu_system.h"
#include "orthocyclic_round.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "display.h"
#include "menu.h"
#include "config.h"
#include "kinematic.h"
#include "step_motor.h"
#include "menu_export.h"
#include "mathlib.h"
#include "orthocyclic_round.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

static const char* TAG = "main";

// main time based speed
float g_speed = 1;

// ==============================================================================
// LEDS
// ==============================================================================

#define GPIO_R GPIO_NUM_2

OrthocyclicRound ortho_round;

static void init_menu() {
    // Create root menu
    MenuSystem::instance.init();
    // Create the global settings
    auto global = MenuSystem::instance.get_or_create("/time");
    // Add options to the global menu
    global->add(new FloatItem(global, "Speed",
                              []() -> float { return g_speed; },
                              [](float s) { g_speed = CLAMP(s,0.2,1); }));
    // Open root menu
    MenuSystem::instance.open_menu(MenuSystem::instance.root,0);
    // Initialize children
    Kinematic::instance.init_menu(std::string("kinematic"));
    ortho_round.init_menu("ortho-round");
}

// ==============================================================================
// Main application
// ==============================================================================

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Start appplication");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "This is %s chip with %d CPU cores, WiFi%s%s, ",
            CHIP_NAME,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");


    display_init();
    Kinematic::instance.init();

    // Init the menu now when all other systems are initialized
    // Config the menu
    init_menu();

    // After menu initialized
    ortho_round.update_config();

    vTaskDelay(200 / portTICK_PERIOD_MS);

    const TickType_t time_increment = MOTOR_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();
    float time = 0;
    while (true) {
        vTaskDelayUntil( &last_wake_time, time_increment );
        time += g_speed * ((float)MOTOR_UPDATE_PERIOD_MS/1000.0f);
        Kinematic::instance.update(time);
        MenuSystem::instance.update(time);
        ortho_round.update();
    }
    printf("Restarting now.\n");
    fflush(stdout);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    esp_restart();
}
