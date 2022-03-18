#include "menu.h"
#include "menu_system.h"
#include <cstdio>

// =============================================================================================
// The menu
// =============================================================================================

Menu::Menu()
    : MenuItem()
    , on_open()
    , on_close()
    , items()
    , line(0)
{
    value = "...";
}

Menu::Menu(Menu* parent, std::string label, int order)
    : MenuItem(parent, label, order)
    , on_open()
    , on_close()
    , items()
    , line(0)
{
    value = "...";
    if (parent != nullptr)
        add(parent);
}

bool Menu::is_menu() { return true; }

int Menu::get_line() {
  return line;
}

void Menu::set_line(int n) {
  line = line < 0 ? 0 : (line >= size() ? size() - 1 : n);
}

Menu& Menu::add(MenuItem* item) {
  items.push_back(item);
  return *this;
}

void Menu::render()
{
    // Do not do anythig. Because menu item renders all the time
    // constant label string
}

/** The event for menu item */
void Menu::on_event(MenuEvent evt) {

    switch (evt) {
        case MenuEvent::Render:
            render();
            break;
        case MenuEvent::PressQuad:
        {
            auto item = items[line];
            if (item->is_menu())
                MenuSystem::instance.open_menu((Menu*)item);
            else
                items[line]->on_event(evt);
        }
        break;
        case MenuEvent::Right:
            items[line]->on_event(evt);
            break;
        case MenuEvent::Left:
            items[line]->on_event(evt);
            break;
        case MenuEvent::Up:
            if (line > 0)
                line--;
            break;
        case MenuEvent::Down:
            if (line < size()-1)
              line++;
            break;
        case MenuEvent::Reset:
            break;
        default:
            break;
    }
}
