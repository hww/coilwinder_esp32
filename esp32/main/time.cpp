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

#include "time.hpp"

// =========================================================================
//

Time::Time() : m_time(0), m_delta_time(0) {

}

void Time::update()
{
    double timeus = (double)esp_timer_get_time();
    float timesec = timeus / 1000000;
    m_delta_time = timesec - m_time;
    m_time = timesec;
}
