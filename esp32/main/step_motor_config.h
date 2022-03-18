#ifndef STEP_MOTOR_CONFIG_H_
#define STEP_MOTOR_CONFIG_H_

#include "gpiolib.h"
#include "typeslib.h"


class StepMotorConfig {
 public:

  int id;
  /** GPIO settings */
  gpio_num_t step_pin;
  gpio_num_t dir_pin;
  gpio_num_t enable_pin;
  gpio_num_t endstop_pin;
  bool dir_pin_reverse;
  bool step_pin_reverse;
  bool enable_pin_reverse;
  bool endstop_pin_reverse;

  /** Kinematic settings */
  unit_t max_velocity;
  unit_t max_accel;
  unit_t rotation_distance;
  int microsteps;
  int steps_per_turn;
  unit_t position_min;
  unit_t position_endstop;
  unit_t position_max;

  /** Homing settings */
  unit_t homing_speed;
  unit_t homing_retract_dist;
  unit_t second_homing_speed;
  bool homing_dir;

  /** Config */
  double distance_per_step;
  int microsteps_per_turn;

  StepMotorConfig();

  void init();
  unit_t steps_to_units(steps_t steps);
  steps_t units_to_steps(unit_t units);
  float units_to_fsteps(unit_t units);

};

#endif
