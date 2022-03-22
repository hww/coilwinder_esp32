#include <string>

#include <assert.h>
#include <cstdio>

#include "esp_log.h"

#include "config.h"
#include "display.h"
#include "menu_event.h"
#include "strlib.h"
#include "menu_system.h"
#include "menu_item.h"
#include "menu.h"
#include "input_controller.h"

static const char TAG[] = "menu-system";

MenuSystem MenuSystem::instance;

MenuSystem::MenuSystem()
  : root(nullptr)
  , current(nullptr)
  , is_edit(false)
  , is_visible(false)
  , menu_listeners()
  , event_listeners()
  , log(0)
  , modification_speed(1)
  , refresh_at(0)
{}

/**
 * Initialize the menu with new root element
 **/
void MenuSystem::init()
{
  input_controller_init();
  root = new Menu(nullptr, "root");
}

/**
 * Update the menum and refresh the screen
 **/
void MenuSystem::update(float time) {
  input_controller_update();
  if (time > refresh_at) {
    refresh_at = time + 0.5f;
    render();
  }
}

void MenuSystem::set_visible(bool v) {
  if (current!=nullptr) {
    if (log>0)
      ESP_LOGI(TAG, "Set menu %s visible %d", current->label.c_str(), v);
    if (is_visible && !v) {
      display_clear();
      display_update();
    }
    is_visible = v;
    render();
  } else {
    if (log>0)
      ESP_LOGI(TAG, "Set null menu visible %d",  v);
    is_visible = false;
  }
}

Menu* MenuSystem::get_root_menu(Menu* menu)
{
  while (menu->parent != nullptr)
    menu = menu->parent;
  return menu;
}
/**
 * Find or create the menu or submenu
 **/
Menu* MenuSystem::get_or_create(std::string path, int order) {
  auto lst = split(path, '/');
  auto cur = root;
  for (auto &label : lst) {
    auto item = cur->find(label);
    if (item == nullptr) {
      auto sub = new Menu(cur, label, order);
      cur->add(sub);
      cur = sub;
    } else {
      if (item->is_menu())
        cur = (Menu*)item;
      else {
        ESP_LOGE(TAG, "Can't create menu with path '%s'", path.c_str());
        return nullptr;
      }
    }
  }
  return (Menu*)cur;
}

/**
 * Find or create the menu or submenu
 **/
Menu* MenuSystem::get_or_create(const char* path, int order) {
  return get_or_create(std::string(path), order);
}

/**
 * Get and set the modification speed. The default value is 1
 **/
float MenuSystem::get_speed() { return modification_speed; }

void MenuSystem::set_speed(float speed) {
  assert(speed == 1 || speed == 0.1 || speed == 10);
  modification_speed = speed;
}


void MenuSystem::open_menu(Menu* menu, int line) {

  current = menu;
  display_clear();
  display_update();
  if (current != nullptr) {
    if (log>0)
      ESP_LOGI(TAG, "Open '%s' menu with size %d and curent line %d ", current->label.c_str(), current->size(), current->get_line());
    if (line>=0)
      menu->set_line(line);

  } else {
    ESP_LOGE(TAG, "Open null menu");
  }
  send_to_menu_listeners(menu, MenuEvent::OpenMenu);
  refresh_at = 0;
  is_visible = current != nullptr;
}

void MenuSystem::close_menu(Menu* menu) {
  if (log>0)
  ESP_LOGI(TAG, "Close menu");
  display_clear();
  display_update();
  send_to_menu_listeners(menu, MenuEvent::CloseMenu);
  if (current == menu)
    open_menu(menu->parent);
}

void MenuSystem::toggle_edit() {
  is_edit = !is_edit;
}

/** The event for menu list  */
void MenuSystem::on_event(MenuEvent evt) {

  if (is_edit) {
    modification_speed = input_get_key(Button::B) ? 10 : 1;
  } else {
    modification_speed = 1;
    // do not change menu if the A or B pressed
    if (input_get_key(Button::B) || input_get_key(Button::A))
      return;
  }

  for (auto &item : event_listeners) {
    item(evt);
  }

  if (evt == MenuEvent::PressQuad) {
    if (current == nullptr) {
      open_menu(root);
      return;
    } else if (!is_visible) {
      set_visible(true);
      return;
    }
  }

  if (is_visible) {
    switch (evt) {
      case MenuEvent::Render:
        render();
        break;
      case MenuEvent::PressQuad:
        current->on_event(evt);
        break;
      case MenuEvent::PressA:
        current->on_event(evt);
        break;
      case MenuEvent::PressB:
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
  } else {
    if (log>1)
      ESP_LOGI(TAG, "on_event to invisible menu");
  }
}

void MenuSystem::render() {

  if (!is_visible)
    return;
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
          snprintf(buf, 17, " %.14s ", item->label.c_str());
          display_print(0, y-first_line, buf);
          snprintf(buf, 17, "[%.14s]", item->value.c_str());
          display_print(0, y-first_line+1, buf);
        } else {
          snprintf(buf, 17, ">%.14s ", item->label.c_str());
          display_print(0, y-first_line, buf);
          snprintf(buf, 17, " %.14s ", item->value.c_str());
          display_print(0, y-first_line+1, buf);
        }
      } else {
        snprintf(buf, 17, " %.14s ", item->label.c_str());
        display_print(0, y-first_line, buf);
        snprintf(buf, 17, " %.14s ", item->value.c_str());
        display_print(0, y-first_line+1, buf);
      }

      //printf("%s\n", buf);
    }

    y++;
  }
  display_update();
}

void MenuSystem::send_to_menu_listeners(MenuItem* _item, MenuEvent evt)
{
  for (auto &item : menu_listeners) {
    item(_item, evt);
  }
}
