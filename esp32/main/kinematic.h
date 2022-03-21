#ifndef KINEMATIC_H_
#define KINEMATIC_H_

#include <cstdint>
#include <string>

#include "step_motor.h"
#include "step_motor_config.h"
#include "typeslib.h"

class Kinematic {
        public:
                Kinematic();
                void init();
                void update(float time);
                void init_menu(std::string path);

                void move_to(unit_t x, unit_t r, percents_t rpm);
                void get_default_velocity(unit_t& x, unit_t& r);
                void get_velocity(unit_t& x, unit_t& r);
                void set_velocity(unit_t dx, unit_t dr);
                void get_position(unit_t& x, unit_t& r);
                void get_velocity(unit_t xdist, unit_t rdist, unit_t& vx, unit_t& vr);
                void set_origin();

                StepMotor xmotor;
                StepMotor rmotor;
                StepMotorConfig xconfig;
                StepMotorConfig rconfig;

                static Kinematic instance;
};


#endif // KINEMATIC_H_
