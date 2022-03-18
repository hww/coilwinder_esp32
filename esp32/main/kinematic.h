#ifndef KINEMATIC_H_
#define KINEMATIC_H_

#include "step_motor.h"
#include "step_motor_config.h"
#include <string>

class Kinematic {
    public:
        Kinematic();
        void init();
        void update(float time);
        void init_menu(Menu* parent, std::string name);

        StepMotor xmotor;
        StepMotor rmotor;
        StepMotorConfig xconfig;
        StepMotorConfig rconfig;

        static Kinematic instance;
};


#endif // KINEMATIC_H_
