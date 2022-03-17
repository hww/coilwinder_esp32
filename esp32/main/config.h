#ifndef CONFIG_H_
#define CONFIG_H_

#include "driver/gpio.h"

extern float g_speed;

// ==============================================================
// TIME SETTINGS
// ==============================================================

#define MOTOR_UPDATE_PERIOD_MS 20

// ==============================================================
// Config
// ==============================================================

#define AR_DEFAULT_FONT Font_droid_sans_mono_7x13
#define MENU_NAME_COL 2
#define MENU_VAL_COL 10
// The screen fit only this amount of lines. Scroll it
#define MENU_MAX_LINES 2

// ==============================================================
// Cinematics
// ==============================================================

//The thread step
#define SHAFT_STEP 50


// ==============================================================
// END LIMITS
// ==============================================================

/** Dummy pin for all disabled and unused settings */
#define GPIO_NULL GPIO_NUM_0

/** All motors usualy have some amout of steps per turn */
#define MOTOR_STEPS_PER_TURN 200
#define MOTOR_STEP_PULSE_WIDTH_MS 1
#define MOTOR_DIR_PULSE_DELAY_MS 1
#define MOTOR_ENABLE_PULSE_DELAY_MS 10

#define MOTOR_X_STEP_PIN GPIO_NUM_25
#define MOTOR_X_DIR_PIN GPIO_NUM_26
#define MOTOR_X_ENABLE_PIN GPIO_NUM_13
#define MOTOR_X_ENDSTOP_PIN GPIO_NUM_35
#define MOTOR_X_STEP_PIN_REVERSE 1
#define MOTOR_X_DIR_PIN_REVERSE 1
#define MOTOR_X_ENABLE_PIN_REVERSE 1
#define MOTOR_X_ENDSTOP_PIN_REVERSE 0
#define MOTOR_X_MAX_VELOCITY 30/*mm/s*/
#define MOTOR_X_MAX_ACCELERATION 1000/*mm/s^2*/
#define MOTOR_X_ROTATION_DISTANCE 2/*mm*/
#define MOTOR_X_MICROSTEPS 8
#define MOTOR_X_POSITION_MIN 0
#define MOTOR_X_POSITION_ENDSTOP 0
#define MOTOR_X_POSITION_MAX 100
#define MOTOR_X_HOMING_SPEED 30
#define MOTOR_X_HOMING_RETRACT_DIST 20
#define MOTOR_X_SECOND_HOMING_SPEED 15
#define MOTOR_X_HOMING_DIR -1

#define MOTOR_R_STEP_PIN GPIO_NUM_27
#define MOTOR_R_DIR_PIN GPIO_NUM_14
#define MOTOR_R_ENABLE_PIN GPIO_NUM_4
#define MOTOR_R_ENDSTOP_PIN GPIO_NULL
#define MOTOR_R_STEP_PIN_REVERSE 1
#define MOTOR_R_DIR_PIN_REVERSE 1
#define MOTOR_R_ENABLE_PIN_REVERSE 1
#define MOTOR_R_ENDSTOP_PIN_REVERSE 0
#define MOTOR_R_MAX_VELOCITY 5/*turns/s*/
#define MOTOR_R_MAX_ACCELERATION 20/*turns/s^2*/
#define MOTOR_R_ROTATION_DISTANCE 1/*turn*/
#define MOTOR_R_MICROSTEPS 8
#define MOTOR_R_POSITION_MIN 0
#define MOTOR_R_POSITION_ENDSTOP 0
#define MOTOR_R_POSITION_MAX 100
#define MOTOR_R_HOMING_SPEED 100
#define MOTOR_R_HOMING_RETRACT_DIST 50
#define MOTOR_R_SECOND_HOMING_SPEED 50
#define MOTOR_R_HOMING_DIR -1

// ==============================================================
// ROTARY ENCODER
// ==============================================================

#define ROT_ENC_A_GPIO GPIO_NUM_16
#define ROT_ENC_B_GPIO GPIO_NUM_17
// Set to true to enable tracking of rotary encoder at half step resolution
#define ROT_ENABLE_HALF_STEPS true
// The encoder button
#define GP_BUTTON_ROT (gpio_num_t)32

// Additional buttons
#define GP_BUTTON_A (gpio_num_t)33
#define GP_BUTTON_B (gpio_num_t)15

#endif // CONFIG_H_
