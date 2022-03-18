#include "config.h"
#include "kinematic.h"
#include "menu_export.h"
#include "step_motor_config.h"

Kinematic Kinematic::instance;

static bool motors_enabled;

Kinematic::Kinematic()
{
}

void Kinematic::init()
{
  /** X Motor */
  xconfig.id = 0;
  // GPIO
  xconfig.step_pin = MOTOR_X_STEP_PIN;
  xconfig.dir_pin = MOTOR_X_DIR_PIN;
  xconfig.enable_pin = MOTOR_X_ENABLE_PIN;
  xconfig.endstop_pin = MOTOR_X_ENDSTOP_PIN;
  xconfig.dir_pin_reverse = MOTOR_X_DIR_PIN_REVERSE;
  xconfig.step_pin_reverse = MOTOR_X_STEP_PIN_REVERSE;
  xconfig.enable_pin_reverse = MOTOR_X_ENABLE_PIN_REVERSE;
  xconfig.endstop_pin_reverse = MOTOR_X_ENDSTOP_PIN_REVERSE;
  // Kinematic
  xconfig.max_velocity = MOTOR_X_MAX_VELOCITY;
  xconfig.max_accel = MOTOR_X_MAX_ACCELERATION;
  xconfig.rotation_distance = MOTOR_X_ROTATION_DISTANCE;
  xconfig.microsteps = MOTOR_X_MICROSTEPS;
  xconfig.steps_per_turn = MOTOR_STEPS_PER_TURN;
  xconfig.position_min = MOTOR_X_POSITION_MIN;
  xconfig.position_endstop = MOTOR_X_POSITION_ENDSTOP;
  xconfig.position_max = MOTOR_X_POSITION_MAX;
  xconfig.homing_speed = MOTOR_X_HOMING_SPEED;
  xconfig.homing_retract_dist = MOTOR_X_HOMING_RETRACT_DIST;
  xconfig.second_homing_speed = MOTOR_X_SECOND_HOMING_SPEED;
  xconfig.homing_dir = MOTOR_X_HOMING_DIR;

  /** R motor */
  rconfig.id = 1;
  // GPIO
  rconfig.step_pin = MOTOR_R_STEP_PIN;
  rconfig.dir_pin = MOTOR_R_DIR_PIN;
  rconfig.enable_pin = MOTOR_R_ENABLE_PIN;
  rconfig.endstop_pin = MOTOR_R_ENDSTOP_PIN;
  rconfig.dir_pin_reverse = MOTOR_R_DIR_PIN_REVERSE;
  rconfig.step_pin_reverse = MOTOR_R_STEP_PIN_REVERSE;
  rconfig.enable_pin_reverse = MOTOR_R_ENABLE_PIN_REVERSE;
  // Kinematic
  rconfig.max_velocity = MOTOR_R_MAX_VELOCITY;
  rconfig.max_accel = MOTOR_R_MAX_ACCELERATION;
  rconfig.rotation_distance = MOTOR_R_ROTATION_DISTANCE;
  rconfig.microsteps = MOTOR_R_MICROSTEPS;
  rconfig.steps_per_turn = MOTOR_STEPS_PER_TURN;
  rconfig.position_min = MOTOR_R_POSITION_MIN;
  rconfig.position_endstop = MOTOR_R_POSITION_ENDSTOP;
  rconfig.position_max = MOTOR_R_POSITION_MAX;
  rconfig.homing_speed = MOTOR_R_HOMING_SPEED;
  rconfig.homing_retract_dist = MOTOR_R_HOMING_RETRACT_DIST;
  rconfig.second_homing_speed = MOTOR_R_SECOND_HOMING_SPEED;
  rconfig.homing_dir = MOTOR_R_HOMING_DIR;
  // Initialize motors
  xmotor.init(&xconfig);
  rmotor.init(&rconfig);

  motors_enabled = true;
  Kinematic::instance.xmotor.set_enable(motors_enabled);
  Kinematic::instance.rmotor.set_enable(motors_enabled);
}


void Kinematic::update(float time) {
  xmotor.update(time);
  rmotor.update(time);
}

static void toggle_drivers(MenuItem* item, MenuEvent evt) {
  motors_enabled = !motors_enabled;
  Kinematic::instance.xmotor.set_enable(motors_enabled);
  Kinematic::instance.rmotor.set_enable(motors_enabled);
}

static void start_homing_task(MenuItem* item, MenuEvent evt) {
  Kinematic::instance.xmotor.move_to_home();
}

void Kinematic::init_menu(Menu* parent, std::string name)
{
  auto menu = new Menu(parent, name);
  parent->add(menu);
  menu->add(new ActionItem(menu, "m-onoff", &toggle_drivers));
  menu->add(new ActionItem(menu, "homing", &start_homing_task));
  xmotor.init_menu(menu, "Mot X");
  rmotor.init_menu(menu, "Mot R");
}