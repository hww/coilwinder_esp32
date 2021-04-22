#include <stdio.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <set>
#include <unordered_set>

#include <freertos/FreeRTOS.h>

#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>

#include <nvs_flash.h>
#include <nvs.h>

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/timer.h>

#include <cmath>

#include <lwip/sockets.h>
#include <lwip/netdb.h>

#include <cstring>
#include <string>

#include <sys/time.h>

#include <esp32/rom/ets_sys.h>

#include <esp_event.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h> 

#include "sdkconfig.h"
#include "display.h"

#define millis() (esp_timer_get_time() / 1000)
#define delay_ms(n) vTaskDelay(n / portTICK_PERIOD_MS)

#define STATUS_PIN GPIO_NUM_21

/* GPIO number per each LED color */
#define GPIO_R GPIO_NUM_2
#define GPIO_G GPIO_NUM_15
#define GPIO_B GPIO_NUM_12
/* GPIO for main led */
#define LED_PIN GPIO_R

/* GPIO for the motor stepper */
#define STEP1_PIN GPIO_NUM_25
#define DIR1_PIN GPIO_NUM_26
#define STEP2_PIN GPIO_NUM_27
#define DIR2_PIN GPIO_NUM_14

xQueueHandle gCmdQueue;
xQueueHandle gNetQueue;

SemaphoreHandle_t gMutex = nullptr;

const int CONNECTED_BIT = BIT0;

volatile bool gLaserOn = false;

volatile int gMainRPM=0;
volatile int gCoilLengthSteps=0;
volatile int gCnt1=0;

volatile uint64_t gTickCount=0;
volatile uint64_t gAlarmCount=0;

volatile float gX=0;
volatile float gY=0;
volatile float gXAccum=0;
volatile float gYAccum=0;
volatile float gE=0;

volatile int gDesiredDelay0=0;
volatile int gCurrentDelay0=0;
volatile int gDelayModifier=0;

volatile uint32_t gIntensity = 32;

#define STEPS_PER_MM (40.0f/6400.0f)

#define DELAY_BETWEEN_STEPS 10000

#define INV_OUT

#define MAX_DUTY (1 << LEDC_TIMER_10_BIT)

struct sMotion
{
  enum eState { IDLE, ACC, DEC, MAXSPD };
  eState state;
  int stepsUsed;
};

struct sCommand {
    enum CmdType { cmdSTOP, cmdSTART };
    sCommand(){}
    sCommand(CmdType t):type(t){}
    char pkt[3] = {'p', 'k', 't'};
    char type;
    uint32_t mainRPM=0;
    float wireDiameter=0;
    uint32_t coilLength=0;
    uint32_t direction=0;
};

struct sCommandReply {
  char data[4];
};

volatile sMotion gMotion;

std::unordered_set<int> mStateChanges;
std::list<sCommand> mCommands;

#define forever while(1)


class cFreeRTOS {
public:
  cFreeRTOS(){}
  ~cFreeRTOS(){}
  static void startTask(void task(void *), std::string taskName, void *param=nullptr, int stackSize = 4096) {
    ::xTaskCreate(task, taskName.data(), stackSize, param, 10, NULL);
  }
};


float distanceToSteps(float dist) {
  //printf("steps per mm %f\n", STEPS_PER_MM);
  return dist / STEPS_PER_MM;
}


void pwmInit()
{
    ledc_channel_config_t ledc_channel_left = {};
    ledc_channel_left.gpio_num = LED_PIN;
    ledc_channel_left.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel_left.channel = LEDC_CHANNEL_0;
    ledc_channel_left.intr_type = LEDC_INTR_DISABLE;
    ledc_channel_left.timer_sel = LEDC_TIMER_2;
    ledc_channel_left.duty = 0;

    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    // Deprecated!
    //ledc_timer.bit_num = LEDC_TIMER_10_BIT;
    ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
    ledc_timer.timer_num = LEDC_TIMER_2;
    ledc_timer.freq_hz = 500;

    ledc_channel_config(&ledc_channel_left);
    ledc_timer_config(&ledc_timer);
}


void pwmSet(uint32_t duty)
{
  //printf("max duty: %d, duty: %d\n", MAX_DUTY, duty);

  #ifdef INV_OUT
    duty = MAX_DUTY - duty;
  #endif

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// MAIN TASK 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mainTask(void *pvParameter)
{
  forever
  {
    sCommand cmd;
    if(mCommands.size())
    {
      cmd = mCommands.front(); mCommands.pop_front();

      gMainRPM = cmd.mainRPM;
      printf("main RPM: %d\n", gMainRPM);

      uint32_t stepDelay = 40000000 / ((gMainRPM / 60.0f) * 6400);
      printf("step delay1: %d\n", stepDelay);

      // 2 is the distance in mm / revolution eg the thread steepness
      uint32_t stepDelay2 = stepDelay * (2/cmd.wireDiameter);
      printf("step delay2: %d\n", stepDelay2);

      gCoilLengthSteps = (6400/2) * cmd.coilLength;
      printf("coil length steps: %d\n", gCoilLengthSteps);
      gCnt1 = gCoilLengthSteps;

      printf("direction: %d\n", cmd.direction);
      gpio_set_level(DIR2_PIN, cmd.direction);
      gpio_set_level(STATUS_PIN, cmd.direction);

      // calc step delay based on RPM and timer freq
      timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, stepDelay);
      timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, stepDelay2);

      //timer_start(TIMER_GROUP_0, TIMER_0);
      //timer_start(TIMER_GROUP_0, TIMER_1);
    }

    printf("gCnt1: %d\n", gCnt1);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}


void enableGPIO(gpio_num_t pin) {
  gpio_pad_select_gpio(pin);
  gpio_set_direction(pin, static_cast<gpio_mode_t>(GPIO_MODE_INPUT_OUTPUT));
  gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);
  gpio_set_level(pin, 0);
}


static void timer0Handler(void *arg) {

    TIMERG0.int_clr_timers.t0 = 1;
    TIMERG0.hw_timer[0].update=1;
    TIMERG0.hw_timer[0].config.alarm_en = 1;

    if(gMainRPM > 0)
    {
      gpio_set_level(STEP1_PIN, 1);
      ets_delay_us(1);
      gpio_set_level(STEP1_PIN, 0);
    }
}


static void togglePin(gpio_num_t pin)
{
  int level = gpio_get_level(pin);
  level = !level;
  gpio_set_level(pin, level);
}


static void timer1Handler(void *arg) {

    TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[1].update=1;
    TIMERG0.hw_timer[1].config.alarm_en = 1;

    if(gMainRPM > 0) {

      if(gCnt1 <= 0) {
        gCnt1 = gCoilLengthSteps;
        togglePin(motor->dir_pin);
        togglePin(motor->status_pin);
      }
      
      gpio_set_level(STEP2_PIN, 1);
      ets_delay_us(1);
      gpio_set_level(STEP2_PIN, 0);
      gCnt1--;
    }
}


esp_err_t configTimer(timer_group_t tgrp, timer_idx_t tmr, void (*fn)(void*), void * arg)
{
  esp_err_t res;

  timer_config_t timerCfg = {
    TIMER_ALARM_EN,
    TIMER_PAUSE,
    TIMER_INTR_LEVEL,
    TIMER_COUNT_UP,
    TIMER_AUTORELOAD_EN,
    2 }; // divider

  if((res = timer_init(tgrp, tmr, &timerCfg)) == ESP_OK) {
    printf("TIMER initialized!\n");
  } else {
    printf("TIMER not initialized!\n");
    return res;
  }

  timer_set_alarm_value(tgrp, tmr, DELAY_BETWEEN_STEPS);
  timer_set_counter_value(tgrp, tmr, 0);

  timer_enable_intr(tgrp, tmr);

  timer_isr_handle_t *handle = 0;
  if((res = timer_isr_register(tgrp, tmr, fn, arg, 0, handle)) == ESP_OK) {
    printf("TIMER isr registered!!! :)\n");
  } else {
    printf("TIMER isr not registered!\n");
    return res;
  }

  if((res = timer_start(tgrp, tmr)) == ESP_OK) {
    printf("TIMER started!\n");
  } else {
    printf("TIMER not started!\n");
    return res;
  }
  return ESP_OK;
}

extern "C" void app_main()
{

  nvs_flash_init();

  gMutex = xSemaphoreCreateMutex();

  gCmdQueue = xQueueCreate(5, sizeof(sCommand));
  gNetQueue = xQueueCreate(5, sizeof(sCommandReply));

  enableGPIO(LED_PIN);
  enableGPIO(STATUS_PIN);
  enableGPIO(STEP1_PIN);
  enableGPIO(DIR1_PIN);
  enableGPIO(STEP2_PIN);
  enableGPIO(DIR2_PIN);

  if(configTimer(TIMER_GROUP_0, TIMER_0, timer0Handler, nullptr) == ESP_OK) {
    printf("timer0 initialized!\n");
  }

  if(configTimer(TIMER_GROUP_0, TIMER_1, timer1Handler, nullptr) == ESP_OK) {
    printf("timer1 initialized!\n");
  }

  gpio_set_level(DIR1_PIN, 1);
  gpio_set_level(DIR2_PIN, 1);

  pwmInit();

  display_init();

  pwmSet(16);

//  cFreeRTOS::startTask(mainTask, "Main task");
}
