#include <assert.h>
#include <math.h>
#include <string>

#include "coil.h"
#include "menu_system.h"
#include "step_motor.h"
#include "menu_export.h"

// ========================================================
// Base classes
// ========================================================

Coil::Coil()
    : wire_od(1)
    , wire_turns(0)
    , wire_layers(0)
    , version(-1)
    , menu(nullptr)
{

}

void Coil::init_menu(std::string path)
{

    auto menu = MenuSystem::instance.get_or_create(path);
    menu->add(new FloatItem(menu, "wire_od",
                            [&]() -> float { return wire_od; },
                            [&](float t) { wire_od = t; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(2);
    menu->add(new IntItem(menu, "*wire_turns+1",
                          [&]() -> int { return wire_turns; },
                          [&](int t) { wire_turns = t; }));
    menu->add(new IntItem(menu, "*wire_turns+10",
                          [&]() -> int { return wire_turns; },
                          [&](int t) { wire_turns = t; }));
    menu->get_last<IntItem>().set_step(10);
    menu->add(new IntItem(menu, "*wire_layers(",
                          [&]() -> int { return wire_layers; },
                          [&](int t) { wire_layers = t; }));
}

void Coil::inspect()
{
    printf("Coil:\n");
    printf("  Wire OD         = %.2f mm\n", wire_od);
    printf("  Wire turns      = %d\n", wire_turns);
    printf("  Wire layers     = %d\n", wire_layers);
}

// ========================================================
// Generic round coil
// ========================================================

RoundCoil::RoundCoil ()
    : Coil()
    , bob_len(10)
    , bob_id(10)
    , bob_od(0)
{

}

void RoundCoil::init_menu(std::string path)
{
    Coil::init_menu(path);

    auto menu = MenuSystem::instance.get_or_create(path);
    // Edit settings
    menu->add(new FloatItem(menu, "bob-len",
                            [&] () -> float { return bob_len; },
                            [&] (float v) { bob_len = v; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(1);
    menu->add(new FloatItem(menu, "bob-id",
                            [&] () -> float { return bob_id; },
                            [&] (float v) { bob_id = v; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(1);
    menu->add(new FloatItem(menu, "*bob-od",
                            [&] () -> float { return bob_od; },
                            [&] (float v) { bob_od = v; }));
    menu->get_last<FloatItem>().set_step(1).set_precision(1);
}

void RoundCoil::inspect()
{
    Coil::inspect();
    printf("Round coil:\n");
    printf("  Bobin length     = %.2f mm\n", bob_len);
    printf("  Bobin ID         = %.2f mm\n", bob_id);
    printf("  Bobin OD         = %.2f mm\n", bob_od);
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
