#pragma once

void display_init();
void display_clear();
void display_set_font(const struct SSD1306_FontDef* Font );
void display_print(const char* text);
