
#include "menu_system.h"
#include "config.h"
#include "macro.h"
#include "display.h"

// =============================================================================================
// Basic items
// =============================================================================================

void MenuItem::display(int x,int y, MenuRenderOption state) {
    if (state != MenuRenderOption::Default)
        display_print(x, y, ">");
    if (m_name.empty())
        display_print(x+MENU_NAME_COL, y, "NoName");
    else
        display_print(x+MENU_NAME_COL, y, m_name.c_str());
}

bool MenuItem::event(MenuEvent& evt) {
    evt.eres = MenuEventRes::Nop;
    return false;
}

// =============================================================================================
// The menu
// =============================================================================================

Menu::Menu(std::string name, Menu* parent) : MenuItem(name), m_items(), m_line_num(0)
{ }

void Menu::display_menu(bool edit_mode)
{
    int y = 0;
    int page = (m_line_num / MENU_MAX_LINES);
    int first_line = page * MENU_MAX_LINES;
    int last_line = first_line + MENU_MAX_LINES - 1;
    for (auto& item : m_items) {
        if (y>last_line)
            break;
        if (y>=first_line) {
            // Display cursor
            MenuRenderOption opt = MenuRenderOption::Default;
            if (y == m_line_num) {
                opt = edit_mode ? MenuRenderOption::Edit : MenuRenderOption::Select;
            }
            item->display(0, y-first_line, opt);
        }
        y++;
    }
}

/** The event for menu item */
bool Menu::event(MenuEvent& evt) {
    switch (evt.etype) {
        case MenuEventType::OnPress:
            evt.eres = MenuEventRes::MenuOpen;
            evt.menu = this;
            break;
        default:
            break;
    }
    return true;
}

/* The event for menu list  */
bool Menu::menu_event(MenuEvent& evt) {
    switch (evt.etype) {
        case MenuEventType::OnRotate:
            if (evt.edit) {
                // Edit the menu line
                return m_items[m_line_num]->event(evt);
            } else {
                // Choose the menu line
                m_line_num += evt.dir;
                if (m_line_num < 0)
                    m_line_num = 0;
                else if (m_line_num >= m_items.size())
                    m_line_num = m_items.size() - 1;
                return true;
            }

        case MenuEventType::OnPress:
            return m_items[m_line_num]->event(evt);

        default:
            break;
    }
    return true;
}

// =============================================================================================
// Floating point
// =============================================================================================

const char FloatItem::DEFAULT_FORMAT[] = "%.2f";
const char FloatItem::DEFAULT_FORMATS[7][16] = {"%.0f", "%.1f", "%.2f", "%.3f", "%.4f", "%.5f", "%.6f"};

FloatItem::FloatItem() : MenuItem(), m_val(0), m_min(-999999), m_max(+999999), m_inc(1), m_fmt(DEFAULT_FORMAT)
{
}
FloatItem::FloatItem(std::string name, float* val, float min, float max, float inc)
    : MenuItem(name), m_val(val), m_min(min), m_max(max), m_inc(inc), m_fmt(DEFAULT_FORMAT)
{
}
FloatItem::FloatItem(std::string name, float* val, float min, float max, float inc, std::string format)
    : MenuItem(name), m_val(val), m_min(min), m_max(max), m_inc(inc), m_fmt(format)
{
}
FloatItem::FloatItem(std::string name, float* val, float min, float max, float inc, int digits)
    : MenuItem(name), m_val(val), m_min(min), m_max(max), m_inc(inc)
{
    assert (digits<ARRAY_COUNT(DEFAULT_FORMATS));
    m_fmt = DEFAULT_FORMATS[digits];
}

void FloatItem::display(int x,int y, MenuRenderOption opt) {
    MenuItem::display(x,y,opt);
    // Render the value
    char bufa[16];
    auto len = snprintf(&bufa[1], sizeof(bufa)-2, m_fmt.c_str(), *m_val);
    // Render the value with hightigh edit mode
    if (opt == MenuRenderOption::Edit) {
        bufa[0] = '[';
        bufa[len+1] = ']';
    }
    else {
        bufa[0] = ' ';
        bufa[len+1] = ' ';
    }
    bufa[len+2] = 0;
    display_print(x+MENU_VAL_COL, y, bufa);
}

bool FloatItem::event(MenuEvent& evt) {
    switch (evt.etype) {
        case MenuEventType::OnRotate:
            *m_val += m_inc *  evt.dir;
            if (*m_val < m_min)
                *m_val = m_min;
            else if (*m_val > m_max)
                *m_val = m_max;
            break;
        case MenuEventType::OnPress:
            evt.eres = evt.edit ? MenuEventRes::EditStop : MenuEventRes::EditStart;
            break;
        default:
            break;
    }
    return true;
}

// =============================================================================================
// Int value
// =============================================================================================

const char IntItem::DEFAULT_FORMAT[] = "%d";

IntItem::IntItem() : MenuItem(), m_val(0), m_min(-999999), m_max(+999999), m_inc(1), m_fmt(DEFAULT_FORMAT)
{
}
IntItem::IntItem(std::string name, int* val, int min, int max, int inc)
    : MenuItem(name), m_val(val), m_min(min), m_max(max), m_inc(inc), m_fmt(DEFAULT_FORMAT)
{
}
IntItem::IntItem(std::string name, int* val, int min, int max, int inc, std::string format)
    : MenuItem(name), m_val(val), m_min(min), m_max(max), m_inc(inc), m_fmt(format)
{
}
void IntItem::display(int x,int y, MenuRenderOption opt) {
    MenuItem::display(x,y,opt);
    // Render the value
    char bufa[16];
    auto len = snprintf(&bufa[1], sizeof(bufa)-3, m_fmt.c_str(), *m_val);
    // Render the value with hightigh edit mode
    if (opt == MenuRenderOption::Edit) {
        bufa[0] = '[';
        bufa[len+1] = ']';
    }
    else {
        bufa[0] = ' ';
        bufa[len+1] = ' ';
    }
    bufa[len+2] = 0;
    display_print(x+MENU_VAL_COL, y, bufa);
}

bool IntItem::event(MenuEvent& evt) {
    switch (evt.etype) {
        case MenuEventType::OnRotate:
            *m_val += evt.dir * m_inc;
            if (*m_val < m_min)
                *m_val = m_min;
            else if (*m_val > m_max)
                *m_val = m_max;
            break;
        case MenuEventType::OnPress:
            evt.eres = evt.edit ? MenuEventRes::EditStop : MenuEventRes::EditStart;
            break;
        default:
            break;
    }
    return true;
}


// =============================================================================================
// Advance Item
// =============================================================================================

AdvancedItem::AdvancedItem() : MenuItem(), m_get(), m_set()
{
}
AdvancedItem::AdvancedItem(std::string name, getter g, setter s)
    : MenuItem(name), m_get(g), m_set(s)
{
}

void AdvancedItem::display(int x,int y, MenuRenderOption opt)
{
    MenuItem::display(x,y,opt);
    // Render the value
    char bufa[16];
    if (m_get != nullptr) {
        auto len = m_get(this, &bufa[1], sizeof(bufa)-3);
        // Render the value with hightigh edit mode
        if (opt == MenuRenderOption::Edit) {
            bufa[0] = '[';
            bufa[len+1] = ']';
        }
        else {
            bufa[0] = ' ';
            bufa[len+1] = ' ';
        }
        bufa[len+2] = 0;
    }
    display_print(x+MENU_VAL_COL, y, bufa);
}

bool AdvancedItem::event(MenuEvent& evt)
{
    switch (evt.etype) {
        case MenuEventType::OnRotate:
            if (m_set != nullptr) {
                m_set(this, evt.dir);
            }
            break;
        case MenuEventType::OnPress:
            evt.eres = evt.edit ? MenuEventRes::EditStop : MenuEventRes::EditStart;
            break;
        default:
            break;
    }
    return true;
}
