#include "menu_system.h"
#include "menu_item.h"
#include "menu.h"
#include "display.h"
#include "menu_controller.h"
#include "config.h"

MenuSystem MenuSystem::instance;

MenuSystem::MenuSystem()
  : root(nullptr)
  , current(nullptr)
  , is_edit(false)
  , is_visible(false)
  , refresh_at(0) {
}

void MenuSystem::init()
{
  menu_controller_init();
}

void MenuSystem::update(float time) {
  menu_controller_update();
  if (time > refresh_at) {
    refresh_at = time + 0.5f;
    render();
  }
}

void MenuSystem::open_menu(Menu* menu, int line) {
  if (line)
    menu->set_line(line);

  current = menu;
  refresh_at = 0;
  is_visible = current != nullptr;
}

void MenuSystem::close_menu(Menu* menu) {
  if (current == menu)
    open_menu(menu->parent);
}

void MenuSystem::toggle_edit() {
  is_edit = !is_edit;
}

/* The event for menu list  */
void MenuSystem::on_event(MenuEvent evt) {
  if (current == nullptr)
    return;

  switch (evt) {
  case MenuEvent::Render:
    render();
    break;
  case MenuEvent::PressQuad:
    MenuSystem::instance.toggle_edit();
    break;
  case MenuEvent::Up:
    current->on_event(evt);
    break;
  case MenuEvent::Down:
    current->on_event(evt);
    break;
  case MenuEvent::Right:
    current->on_event(evt);
    break;
  case MenuEvent::Left:
    current->on_event(evt);
    break;
  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}

void MenuSystem::render() {
  if (current == nullptr)
    return;

  int line = current->get_line();
  int y = 0;
  int page = (line / MENU_MAX_LINES);
  int first_line = page * MENU_MAX_LINES;
  int last_line = first_line + MENU_MAX_LINES - 1;

  for (auto &item : current->get_items()) {
    if (y > last_line)
      break;

    if (y >= first_line) {
      item->render();
      char buf[16];
      snprintf(&buf[1], 7, "%s", item->label.c_str());
      snprintf(&buf[8], 7, "%s", item->value.c_str());
      buf[0] = ' ';
      buf[15] = ' ';
      if (y == line) {
        if (is_edit) {
          buf[0] = '[';
          buf[15] = ']';
        } else {
          buf[0] = '>';
          buf[15] = '<';
        }
      }
      display_print(0, y, buf);
    }

    y++;
  }
  display_update();
}

