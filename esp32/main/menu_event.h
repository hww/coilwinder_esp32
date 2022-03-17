#ifndef MENU_EVENT_H_
#define MENU_EVENT_H_

// ==============================================================================
// Menu event's
// ==============================================================================

enum class MenuEvent {
  Null,
  Render,
  PressQuad,
  Up,
  Down,
  Left,
  Right,
  Reset,
  OpenMenu,
  CloseMenu,
  ToggleMenu
};

enum class MenuResult { Null, Reset, Modified, Called };


#endif // MENU_EVENT_H_
