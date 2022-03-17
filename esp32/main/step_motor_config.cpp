
#include "config.h"
#include "step_motor_config.h"

/** ******************************************/
/** The step motor configuration structuure  */
/** ******************************************/

StepMotorConfig::StepMotorConfig() {}

void StepMotorConfig::init() {
  // Configure motor gemotery and characteristics
  microsteps_per_turn = microsteps * steps_per_turn;
  distance_per_step = rotation_distance / microsteps_per_turn;
}
/** Convert steps quantity to the real units */
unit_t StepMotorConfig::steps_to_units(steps_t steps) {
  float rotations = (float)steps / (float)microsteps_per_turn;
  return rotations * rotation_distance;
}
/** Convert steps quantity to the real units */
steps_t StepMotorConfig::units_to_steps(unit_t units) {
  float rotations = (float)units / rotation_distance;
  return (steps_t)(rotations * microsteps_per_turn);
}
