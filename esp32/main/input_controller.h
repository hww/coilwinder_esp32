#ifndef MENU_CONTROLLER_H_
#define MENU_CONTROLLER_H_

#include "menu_event.h"

void input_controller_init();
void input_controller_update();

enum class Button { Quad, A, B };

bool input_get_key(Button e);
bool input_get_key_up(Button e);
bool input_get_key_down(Button e);
int input_get_delta_position();

#endif
