#ifndef MENU_SYSTEM_H_
#define MENU_SYSTEM_H_

#include "menu_event.h"

class Menu;

class MenuSystem {
  public:

    MenuSystem();

    void init(Menu* root);
    void update(float time);
    void render();
    void on_event(MenuEvent evt);
    void open_menu(Menu* menu, int line_num = -1);
    void close_menu(Menu* menu);
    void toggle_edit();
    float get_speed();
    void set_speed(float speed);

    Menu* root;
    Menu* current;
    bool is_edit;
    bool is_visible;

    static MenuSystem instance;

  private:
    float modification_speed;
    float refresh_at;

};


#endif // MENU_SYSTEM_H_
