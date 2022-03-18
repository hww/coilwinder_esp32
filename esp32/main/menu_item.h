#ifndef MENU_ITEM_H_
#define MENU_ITEM_H_

#include <string>
#include "menu_event.h"
#include <functional>

// ==============================================================================
// Generic menu interface
// ==============================================================================

class Menu;

/** Any kind of menu items */
class MenuItem {
 public:
  MenuItem() : label() {}
  MenuItem(Menu* parent, std::string label, int order = 0) : parent(parent), label(label), order(order)  {}

  /** render the item on the display */
  virtual void render();
  virtual void on_event(MenuEvent evt);
  virtual void on_modified();
  virtual bool is_menu();

  Menu* parent;
  std::string label;
  std::string value;
  int order;
};




// ==============================================================================
// Different types of values
// ==============================================================================

class FloatItem : public MenuItem {

  public:
  static const int DEFAULT_STEP;
  static const int DEFAULT_PRECISION;
  static const char DEFAULT_FORMATS[7][16];

  // Delegates for floating point formats
  typedef std::function<float()> getter_t;
  typedef std::function<void(float)> setter_t;

  FloatItem()
    : MenuItem()
    , getter()
    , setter()
    , format(DEFAULT_FORMATS[DEFAULT_PRECISION])
    , precision(DEFAULT_PRECISION)
    , step(DEFAULT_STEP)
  {}
  FloatItem(Menu* parent, std::string label, getter_t g, setter_t s, float _step = DEFAULT_STEP, int order = 0)
    : MenuItem(parent, label, order)
    , getter(g)
    , setter(s)
    , format(DEFAULT_FORMATS[DEFAULT_PRECISION])
    , precision(DEFAULT_PRECISION)
    , step(_step) {
  }

  getter_t getter;
  setter_t setter;
  std::string format;
  int precision;
  float step;

  void render();
  void on_event(MenuEvent evt);

  inline FloatItem &set_order(int _order) {
    order = _order;
    return *this;
  }
  inline FloatItem &set_format(std::string format) {
    format = format;
    return *this;
  }
  inline FloatItem &set_step(int _step) {
    step = _step;
    return *this;
  }
  FloatItem &set_precision(int _precision);
};


class IntItem : public MenuItem {

 public:
  // Delegates for inting point formats
  typedef std::function<int()> getter_t;
  typedef std::function<void(int)> setter_t;

  IntItem() : MenuItem(), getter(), setter(), format("%d"), step(1) {}
  IntItem(Menu* parent, std::string label, getter_t g, setter_t s, int order = 0)
    : MenuItem(parent, label, order)
    , getter(g)
    , setter(s)
    , format("%d")
    , step(1) {
  }

  getter_t getter;
  setter_t setter;
  std::string format;
  int step;

  void render();
  void on_event(MenuEvent evt);

  inline IntItem &set_order(int _order) {
    order = _order;
    return *this;
  }
  inline IntItem &set_format(std::string _format) {
    format = _format;
    return *this;
  }
  inline IntItem &set_step(int _step) {
    step = _step;
    return *this;
  }
};


class BoolItem : public MenuItem {

 public:
  // Delegates for booling pobool formats
  typedef std::function<bool()>getter_t;
  typedef std::function<void(bool)> setter_t;

  BoolItem() : MenuItem(), getter(), setter(), format("%d") {}
  BoolItem(Menu* parent, std::string label, getter_t g, setter_t s, bool order = 0)
    : MenuItem(parent, label, order)
    , getter(g)
    , setter(s)
    , format("%d") {
  }

  getter_t getter;
  setter_t setter;
  std::string format;

  void render();
  void on_event(MenuEvent evt);

  inline BoolItem &set_order(bool _order) {
    order = _order;
    return *this;
  }
  inline BoolItem &set_format(std::string _format) {
    format = _format;
    return *this;
  }
};

// ==============================================================================
// Adwanced item
// ==============================================================================


class ActionItem : public MenuItem {

 public:
  typedef std::function<void(MenuItem* item, MenuEvent evt)> action_t;

  ActionItem() : MenuItem(), action() {}
  ActionItem(Menu* parent, std::string label, action_t action, bool order = 0)
    : MenuItem(parent, label, order)
    , action(action) {
  }

  void render();
  void on_event(MenuEvent evt);
  void on_modified();

  inline ActionItem &set_order(bool _order) {
    order = _order;
    return *this;
  }
  action_t action;
  uint64_t last_call;
};


#endif // MENU_ITEM_H_
