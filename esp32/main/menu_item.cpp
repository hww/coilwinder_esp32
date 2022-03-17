#include <math.h>
#include "esp_log.h"
#include "assert.h"


#include "config.h"
#include "macro.h"
#include "display.h"

#include "menu_item.h"
#include "menu_system.h"

static const char TAG[] = "menu-item";

// =============================================================================================
// Basic items
// ========================================================

void MenuItem::render() {}

void MenuItem::on_event(MenuEvent evt) {}

void MenuItem::on_modified() {
  if (MenuSystem::instance.is_visible)
    return;

  ESP_LOGI(TAG, "%s %s", label.c_str(), value.c_str());
}

// ========================================================
// Floating point
// ========================================================

const int FloatItem::DEFAULT_PRECISION = 2;
const char FloatItem::DEFAULT_FORMATS[7][16] = {"%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f"};

void FloatItem::render() {
  auto val = getter();
  char buf[32];
  auto len = snprintf(buf, sizeof(buf), format.c_str(), val);
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
      auto fp_scale = (int)pow(10, precision);
      setter((float)(floor(getter() * fp_scale + 0.1f) + step * MenuSystem::instance.modification_speed) / fp_scale);
      render();
      on_modified();
    }
    break;

    case MenuEvent::Left:
    {
      auto fp_scale = (int)pow(10, precision);
      setter((float)(floor(getter() * fp_scale + 0.1f) - step * MenuSystem::instance.modification_speed) / fp_scale);
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
  assert (digits < ARRAY_COUNT(DEFAULT_FORMATS));
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
  auto len = snprintf(buf, sizeof(buf), format.c_str(), val);
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
    setter(getter() + step * MenuSystem::instance.modification_speed);
    render();
    on_modified();
    break;

  case MenuEvent::Left:
    setter(getter() - step * MenuSystem::instance.modification_speed);
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
  auto len = snprintf(buf, sizeof(buf), format.c_str(), val ? "Y" : "N");
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
// Action
// ========================================================


void ActionItem::render() {
  char buf[32];
  auto len = snprintf(buf, sizeof(buf), "<F>");
  value = buf;
}

void ActionItem::on_event(MenuEvent evt) {
  switch (evt) {
  case MenuEvent::Render:
    render();
    break;

  case MenuEvent::PressQuad:
    action(this, evt);
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
