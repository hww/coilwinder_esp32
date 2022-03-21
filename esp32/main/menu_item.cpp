#include <math.h>
#include <string>
#include <filesystem>
#include <assert.h>

#include "esp_log.h"
#include "assert.h"
#include "esp_timer.h"

#include "config.h"
#include "macro.h"
#include "display.h"

#include "menu_item.h"
#include "menu_system.h"
#include "menu.h"
#include "mathlib.h"


static const char TAG[] = "menu-item";

// ========================================================
// Basic items
// ========================================================

MenuItem::MenuItem(std::string _path, int _order)
  : parent(nullptr)
  , label()
  , order(_order)
{
  std::filesystem::path path{_path};
  auto folder = path.parent_path();
  auto file = path.filename();
  auto menu = MenuSystem::instance.get_or_create(folder.string());
  parent = menu;
  label = file.string();
}


void MenuItem::render() {}

void MenuItem::on_event(MenuEvent evt) {}

void MenuItem::on_modified() {
  if (parent->is_menu())
    ((Menu*)parent)->on_item_modified();
  if (MenuSystem::instance.is_visible)
    return;

  ESP_LOGI(TAG, "%s %s", label.c_str(), value.c_str());
}

bool MenuItem::is_menu() { return false; }

// ========================================================
// Floating point
// ========================================================

const int FloatItem::DEFAULT_STEP = 1;
const int FloatItem::DEFAULT_PRECISION = 2;
const char FloatItem::DEFAULT_FORMATS[7][8] = {"%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f"};


void FloatItem::render() {
  auto val = getter();
  char buf[32];
  snprintf(buf, sizeof(buf), format, val);
  value = buf;
}

void FloatItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    MenuSystem::instance.toggle_edit();
    break;

    case MenuEvent::Right:
    {
      auto fp_scale = pow(10, precision);
      auto v = getter() * fp_scale;
      /*auto d = sign(v) * 0.1;*/
      auto s = step;
      auto spd = MenuSystem::instance.get_speed();
      auto v1 = ((float)(round(v) + s * spd) / fp_scale);
      //printf("scaled: %f v: %f step: %f spd: %f -> %f\n", fp_scale, v, s, spd, v1);
      setter(v1);
      render();
      on_modified();
    }
    break;

    case MenuEvent::Left:
    {
      auto fp_scale = pow(10, precision);
      auto v = getter() * fp_scale;
      /*auto d = sign(v) * 0.1;*/
      auto s = step;
      auto spd = MenuSystem::instance.get_speed();
      auto v1 = ((float)(round(v) - s * spd) / fp_scale);
      //printf("scaled: %f v: %f step: %f spd: %f -> %f\n", fp_scale, v, s, spd, v1);
      setter(v1);
      render();
      on_modified();
    }
    break;

  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}

FloatItem& FloatItem::set_precision(int digits) {
  assert (digits >= 0 && digits < ARRAY_COUNT(DEFAULT_FORMATS));
  format = DEFAULT_FORMATS[digits];
  precision = digits;
  return *this;
}

// ========================================================
// Integer
// ========================================================

void IntItem::render() {
  auto val = getter();
  char buf[32];
  snprintf(buf, sizeof(buf), format, val);
  value = buf;
}

void IntItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    MenuSystem::instance.toggle_edit();
    break;

  case MenuEvent::Right:
    setter(getter() + step * MenuSystem::instance.get_speed());
    render();
    on_modified();
    break;

  case MenuEvent::Left:
    setter(getter() - step * MenuSystem::instance.get_speed());
    render();
    on_modified();
    break;

  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}

// ========================================================
// Boolean
// ========================================================

void BoolItem::render() {
  auto val = getter();
  char buf[32];
  snprintf(buf, sizeof(buf), format, val ? 'Y' : 'N');
  value = buf;
}

void BoolItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    MenuSystem::instance.toggle_edit();
    break;

  case MenuEvent::Right:
    setter(true);
    render();
    on_modified();
    break;

  case MenuEvent::Left:
    setter(false);
    render();
    on_modified();
    break;

  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}

// ========================================================
// String
// ========================================================

void StringItem::render() {
  getter(this);
}

void StringItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    MenuSystem::instance.toggle_edit();
    break;

  case MenuEvent::Right:
    setter(this, evt);
    render();
    on_modified();
    break;

  case MenuEvent::Left:
    setter(this, evt);
    render();
    on_modified();
    break;

  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}

// ========================================================
// Action
// ========================================================


void ActionItem::render() {
  auto time = esp_timer_get_time();
  auto highlight = time-last_call < 1000000;
  char buf[32];
  snprintf(buf, sizeof(buf), highlight ? "<!>" : "<F>");
  value = buf;
}

void ActionItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    action(this, evt);
    last_call = esp_timer_get_time();
    break;

  case MenuEvent::Right:
    break;

  case MenuEvent::Left:
    break;

  case MenuEvent::Reset:
    break;

  default:
    break;
  }
}
