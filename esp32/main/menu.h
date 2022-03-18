#ifndef MENU_H_
#define MENU_H_

#include <string>
#include <vector>
#include "menu_item.h"

/** Submenu or menu */
class Menu : public MenuItem {

 public:
  typedef void (*menu_delegate)(Menu* menu);

  Menu();
  Menu(Menu* parent, std::string label, int order = 0);

  void render();
  void on_event(MenuEvent evt);
  void on_modifyed();
  int get_line();
  void set_line(int n);
  Menu &add(MenuItem* item);
  bool is_menu();

  inline int size() { return items.size(); }
  inline std::vector<MenuItem*> get_items() {return items;}

  menu_delegate on_open;
  menu_delegate on_close;

private:
  void on_event_to_item(MenuEvent evt);

  std::vector<MenuItem*> items;
  int line;
};

#endif // MENU_H_
