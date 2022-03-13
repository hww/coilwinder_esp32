#ifndef MENU_SYSTEM_H_
#define MENU_SYSTEM_H_

#include <string>
#include <array>
#include <vector>

// ==============================================================================
// Generic menu interface
// ==============================================================================

class Menu;
class MenuItem;

/** The stae of menu */
enum MenuRenderOption { Default, Select, Edit };
enum MenuEventType { Nope, OnRotate, OnPress, OnPressA, OnPressB };
enum MenuEventRes { Nop, EditStart, EditStop, MenuOpen  };

struct MenuEvent {
    bool edit;
    MenuEventType etype;
    int dir;
    MenuEventRes eres;
    Menu* menu;
};

/** Any kind of menu items */
class MenuItem {
    public:
        MenuItem() : m_name() {}
        MenuItem(std::string name) { m_name = name; }
        std::string m_name;
        /** Render the item on the display */
        virtual void display(int x,int y, MenuRenderOption state);
        virtual bool event(MenuEvent& evt);
};

/** Submenu or menu */
class Menu : public MenuItem {
    public:
        Menu() { }
        Menu(std::string name, Menu* parent);
        std::vector<MenuItem*> m_items;
        int m_line_num;
        Menu* m_parentl;
        inline void Add(MenuItem* item) { m_items.push_back(item); }
        void display_menu(bool edit_mode);
        bool event(MenuEvent& evt);
        bool menu_event(MenuEvent& evt);
};

/** Simple integer value */
class FloatItem : public MenuItem {
        static const char DEFAULT_FORMAT[];
        static const char DEFAULT_FORMATS[7][16];
    public:
        FloatItem();
        FloatItem(std::string name, float* val, float min, float max, float inc);
        FloatItem(std::string name, float* val, float min, float max, float inc, std::string format);
        FloatItem(std::string name, float* val, float min, float max, float inc, int digits);

        float* m_val;
        float m_min;
        float m_max;
        float m_inc;
        std::string m_fmt;

        void display(int x,int y, MenuRenderOption opt);
        bool event(MenuEvent& evt);
};

/** Simple integer value */
class IntItem : public MenuItem {
        static const char DEFAULT_FORMAT[];
    public:
        IntItem();
        IntItem(std::string name, int* val, int min, int max, int inc);
        IntItem(std::string name, int* val, int min, int max, int inc, std::string format);

        int* m_val;
        int m_min;
        int m_max;
        int m_inc;
        std::string m_fmt;

        void display(int x,int y, MenuRenderOption opt);
        bool event(MenuEvent& evt);
};

typedef int (*getter)(MenuItem* item, char* outbuf, int maxlen);
typedef void (*setter)(MenuItem *item, int dir);

/** Simple integer value */
class AdvancedItem : public MenuItem {

    public:
        AdvancedItem();
        AdvancedItem(std::string name, getter g, setter s);

        getter m_get;
        setter m_set;

        void display(int x,int y, MenuRenderOption opt);
        bool event(MenuEvent& evt);
};


#endif // MENU_SYSTEM_H_
