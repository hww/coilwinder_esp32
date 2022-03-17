/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
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
#include "menu_system.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

static const char* TAG = "main";

// main time based speed
float g_speed;

// ==============================================================================
// LEDS
// ==============================================================================

#define GPIO_R GPIO_NUM_2


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
    kinematic.init();

    // Init the menu now when all other systems are initialized
    // Config the menu
    MenuSystem::instance.init();
    /*
    main_menu = new Menu(nullptr, "Menu");
    auto motor_speed_menu = new Menu("Speed...", nullptr);
    main_menu->Add(motor_speed_menu);
    motor_speed_menu->Add(main_menu);
    motor_speed_menu->Add(new FloatItem("Speed", &g_speed, 0, 1, 0.01));
    MenuSystem.instance.open_menu(main_menu);

    menu_init();
    kinematic.InitMenu(main_menu, "Kinemat...");
    */

    vTaskDelay(200 / portTICK_PERIOD_MS);

    const TickType_t time_increment = MOTOR_UPDATE_PERIOD_MS / portTICK_PERIOD_MS;
    TickType_t last_wake_time = xTaskGetTickCount();
    float time = 0;
    while (true) {
        vTaskDelayUntil( &last_wake_time, time_increment );
        time += g_speed * (1000.0f * MOTOR_UPDATE_PERIOD_MS);
        kinematic.update(time);
        MenuSystem::instance.update(time);
    }
    /*
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);


    esp_restart();
    */
}
