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
  inline std::vector<MenuItem*> get_items() {return items;}
  inline Menu &add(MenuItem* item);
  inline int size() { return items.size(); }

  menu_delegate on_open;
  menu_delegate on_close;

private:
  std::vector<MenuItem*> items;
  int line;
};

#endif // MENU_H_
