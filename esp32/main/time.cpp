#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <memory>
#include <codecvt>
#include <locale>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"

#include "time.h"

// =========================================================================
//

Time::Time()
    : time(0)
    , delta_time(0)
    , speed(1)
{
    last_time = esp_timer_get_time();
}

void Time::update()
{
    auto t = esp_timer_get_time();
    auto dt = t-last_time;
    last_time = t;
    float dtsec = (float)dt / 1000000;
    delta_time = dtsec * speed;
    time += delta_time;
}
