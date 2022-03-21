#ifndef MENU_SYSTEM_H_
#define MENU_SYSTEM_H_

#include <string>
#include <functional>
#include <vector>

#include "menu_event.h"
#include "input_controller.h"

class MenuItem;
class Menu;

class MenuSystem {
        public:
                typedef std::function<void(MenuItem* item, MenuEvent evt)> menu_listener_t;
                typedef std::function<void(MenuEvent evt)> event_listener_t;

                MenuSystem();

                void init();
                void update(float time);
                void render();
                void on_event(MenuEvent evt);
                void open_menu(Menu* menu, int line_num = -1);
                void close_menu(Menu* menu);
                void toggle_edit();
                void set_visible(bool v);
                float get_speed();
                void set_speed(float speed);
                void send_to_menu_listeners(MenuItem* item, MenuEvent evt);

                Menu* get_root_menu(Menu* menu);
                Menu* get_or_create(const char* path, int order = 0);
                Menu* get_or_create(std::string path, int order = 0);

                inline void add_menu_listener(menu_listener_t a) { menu_listeners.push_back(a); }
                inline void add_event_listener(event_listener_t a) { event_listeners.push_back(a); }

                Menu* root;
                Menu* current;
                bool is_edit;
                bool is_visible;
                std::vector<menu_listener_t> menu_listeners;
                std::vector<event_listener_t> event_listeners;
                int log;

                static MenuSystem instance;

        private:

                float modification_speed;
                float refresh_at;

};


#endif // MENU_SYSTEM_H_
