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
#include "display.h"
#include "motor.h"
#include "menu.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#define CHIP_NAME "ESP32"
#endif

// ==============================================================================
// Motors
// ==============================================================================

#define MOTOR_X_STEP_PIN GPIO_NUM_25
#define MOTOR_X_DIR_PIN GPIO_NUM_26
#define MOTOR_Y_STEP_PIN GPIO_NUM_27
#define MOTOR_Y_DIR_PIN GPIO_NUM_14

motor_t motor_x;
motor_t motor_y;

// ==============================================================================
// LEDS
// ==============================================================================

#define GPIO_R GPIO_NUM_2
#define GPIO_G GPIO_NUM_15
#define GPIO_B GPIO_NUM_12


// ==============================================================================
// Main application
// ==============================================================================

extern "C" void app_main(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU cores, WiFi%s%s, ",
            CHIP_NAME,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    display_init();
    menu_init();

    motor_x.timer = TIMER_0;
    motor_x.step_pin = MOTOR_X_STEP_PIN;
    motor_x.dir_pin = MOTOR_X_DIR_PIN;
    motor_x.status = false;
    motor_x.cnt_ena = false;
    motor_x.cnt = 0;
    motor_x.coil_length_steps = 0;

    motor_y.timer = TIMER_0;
    motor_y.step_pin = MOTOR_Y_STEP_PIN;
    motor_y.dir_pin = MOTOR_Y_DIR_PIN;
    motor_y.status = false;
    motor_y.cnt_ena = false;
    motor_y.cnt = 0;
    motor_y.coil_length_steps = 0;

    while (true) {
        menu_update();
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
