#include "menu.h"
#include "menu_system.h"

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
}

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
            MenuSystem::instance.open_menu(this);
            break;
        case MenuEvent::Right:
            break;
        case MenuEvent::Left:
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
