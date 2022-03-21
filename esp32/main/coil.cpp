#include <assert.h>
#include <math.h>
#include <string>

#include "coil.h"
#include "step_motor.h"
#include "menu_export.h"

// ========================================================
// Base classes
// ========================================================

Coil::Coil()
    : bob_len(10)
    , wire_od(1)
    , wire_turns(1)
    , version(0)
    , menu(nullptr)
{

}

void Coil::init_menu(std::string path)
{
    auto menu = MenuSystem::instance.get_or_create(path);
    menu->add(new FloatItem(menu, "bobb_len",
                            [&]() -> float { return bob_len; },
                            [&](float t) { bob_len = t; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(1);
    menu->add(new FloatItem(menu, "wire_od",
                            [&]() -> float { return wire_od; },
                            [&](float t) { wire_od = t; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(2);
    menu->add(new IntItem(menu, "wire_turns",
                          [&]() -> int { return wire_turns; },
                          [&](int t) { wire_turns = t; }));
}

// ========================================================
// Generic round coil
// ========================================================

RoundCoil::RoundCoil ()
    : Coil()
    , coil_id(10)
{

}

void RoundCoil::init_menu(std::string path)
{
    Coil::init_menu(path);

    auto menu = MenuSystem::instance.get_or_create(path);
    // Edit settings
    menu->add(new FloatItem(menu, "-coil-id",
                            [&] () -> float { return coil_id; },
                            [&] (float v) { coil_id = v; }));
}


// ========================================================
// Generic rectangular coil
// ========================================================

RectCoil::RectCoil ()
    : Coil()
    , size_a(10)
    , size_b(10)
{

}


void RectCoil::init_menu(std::string path) {
    // TODO
}
