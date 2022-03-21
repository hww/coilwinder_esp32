#ifndef MENU_H_
#define MENU_H_

#include <string>
#include <vector>
#include "menu_item.h"
#include "menu_system.h"

/** Submenu or menu */
class Menu : public MenuItem {

    public:

        Menu();
        Menu(Menu* parent, std::string label, int order = 0);

        void render();
        void on_event(MenuEvent evt);
        void on_modifyed();
        void on_item_modified();
        int get_line();
        void set_line(int n);
        Menu &add(MenuItem* item);
        bool is_menu();
        MenuItem* find(std::string label);

        inline int size() { return items.size(); }
        inline std::vector<MenuItem*> get_items() {return items;}
        inline MenuItem& get_last() { return *items[size()-1]; }
        template<typename T>
        inline T& get_last() { return *((T*)items[size()-1]); }
        inline int get_version() { return version; }
        inline bool is_current() { return MenuSystem::instance.current == this; }
        inline bool is_visible() { return is_current() && MenuSystem::instance.is_visible; }

    private:
        void on_event_to_item(MenuEvent evt);

        std::vector<MenuItem*> items;
        int line;
        int version;
};


#endif // MENU_H_
