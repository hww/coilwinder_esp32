#ifndef KINEMATIC_H_
#define KINEMATIC_H_

#include <cstdint>
#include <string>

#include "step_motor.h"
#include "step_motor_config.h"
#include "typeslib.h"
#include "mathlib.h"

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
                void set_origin();

                inline float get_speed() { return target_speed; }
                inline void set_speed(float tgtv) { target_speed = tgtv; }
                inline void set_speed(float tgtv, float curv) {
                        curent_speed = clamp01(curv);
                        target_speed = clamp01(tgtv);
                }

                StepMotor xmotor;
                StepMotor rmotor;
                StepMotorConfig xconfig;
                StepMotorConfig rconfig;
                unit_t xvelocity;
                unit_t rvelocity;
                float rvelocity_k;
                int log;
                static Kinematic instance;

        private:
                float curent_speed;
                float target_speed;
                float speed_acc;
                Time time;
};


#endif // KINEMATIC_H_
