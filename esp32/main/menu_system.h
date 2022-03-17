#ifndef MENU_SYSTEM_H_
#define MENU_SYSTEM_H_

#include "menu_event.h"

class Menu;

class MenuSystem {
 public:

  MenuSystem();

  void init();
  void update(float time);
  void render();
  void on_event(MenuEvent evt);
  void open_menu(Menu* menu, int line_num = -1);
  void close_menu(Menu* menu);
  void toggle_edit();

  Menu* root;
  Menu* current;
  bool is_edit;
  bool is_visible;
  float refresh_at;
  float modification_speed;

  static MenuSystem instance;

};


#endif // MENU_SYSTEM_H_
