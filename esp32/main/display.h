#pragma once

bool display_init();
void display_clear();
void display_set_font(const struct SSD1306_FontDef* font);
void display_print(const char* text);
void display_print(int x, int y, const char* text );
void display_update();
