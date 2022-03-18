#include "menu_system.h"
#include "menu_item.h"
#include "menu.h"
#include "display.h"
#include "menu_controller.h"
#include "config.h"
#include "esp_log.h"
#include <assert.h>
#include <cstdio>

static const char TAG[] = "menu-system";

MenuSystem MenuSystem::instance;

MenuSystem::MenuSystem()
  : root(nullptr)
  , current(nullptr)
  , is_edit(false)
  , is_visible(false)
  , modification_speed(1)
  , refresh_at(0) {
}
float MenuSystem::get_speed()
{
  return modification_speed;
}
void MenuSystem::set_speed(float speed) {
  assert(speed == 1 || speed == 0.1 || speed == 10);
  modification_speed = speed;
}

void MenuSystem::init(Menu* _root)
{
  root = _root;
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

  current = menu;
  if (current != nullptr) {
    ESP_LOGI(TAG, "Open '%s' menu with size %d and curent line %d ", current->label.c_str(), current->size(), current->get_line());
    if (line>=0)
      menu->set_line(line);

  } else {
    ESP_LOGI(TAG, "Open null menu");
  }
  display_clear();

  refresh_at = 0;
  is_visible = current != nullptr;
}

void MenuSystem::close_menu(Menu* menu) {
  ESP_LOGI(TAG, "Close menu");
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
    current->on_event(evt);
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

  display_clear();

  auto lineidx = current->get_line();
  auto page = (lineidx / MENU_MAX_LINES);
  auto first_line = page * MENU_MAX_LINES;
  auto last_line = first_line + MENU_MAX_LINES - 1;

  auto y = 0;

  for (auto &item : current->get_items()) {

    if (y > last_line)
      break;

    if (y >= first_line) {

      item->render();
      char buf[17];

      if (y == lineidx) {
        if (is_edit) {
          snprintf(buf, 16, "[%-6s %6s]", item->label.c_str(), item->value.c_str());
        } else {
          snprintf(buf, 16, ">%-6s %6s ", item->label.c_str(), item->value.c_str());
        }
      } else {
        snprintf(buf, 16, " %-6s %6s ", item->label.c_str(), item->value.c_str());
      }
      display_print(0, y-first_line, buf);
    }

    y++;
  }
  display_update();
}

