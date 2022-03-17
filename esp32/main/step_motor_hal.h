#ifndef STEP_MOTOR_HAL_H_
#define STEP_MOTOR_HAL_H_

class StepMotorConfig;
class StepMotor;

/** The basic motor control with GPIO pins */
class StepMotorHAL {
 public:
  StepMotorHAL();

  void init(StepMotor* mot);
  bool get_enable();
  void set_enable(bool);
  bool get_direction();
  void set_direction(bool);
  void set_step(bool);
  bool get_endpoint();

  int id;
  StepMotorConfig* config;
};


#endif // STEP_MOTOR_HAL_H_
