#include <cstdio>
#include <math.h>
#include <type_traits>

#include "config.h"
#include "input_controller.h"
#include "menu_system.h"
#include "orthocyclic_round.h"
#include "kinematic.h"
#include "menu_event.h"
#include "menu_export.h"
#include "menu_item.h"
#include "display.h"
#include "mathlib.h"

static const char TAG[] = "orthocyclic-coil";
static const char style_strings[3][16] = {"equal", "1-short","1-long"};

/** The constructor */
OrthocyclicRound::OrthocyclicRound()
    : RoundCoil()
    , style(Style::Equal)
    , fill_last(true)
    , manual_direct(true)
    , accelerate_turns(ACCELERATE_TURNS)
    , deccelerate_turns(DECELERATE_TURNS)
    , stop_before(STOP_BEFORE_TURNS)
    , turns_odd(0)
    , turns_even(0)
    , turns_last(0)
    , total_turns(0)
    , speed(100)
{

}

/** Update with fixed frequency */
void OrthocyclicRound::update()
{
    if (version != menu->get_version())
        update_config();

    auto_winding = input_get_key(Button::A);

    if (auto_winding) {
        auto spd = speed+input_get_delta_position();
        speed = clamp(spd, 1, 100);
    }
    if (!one_turn_dir) {
        if (!auto_winding) {
            one_turn_dir = input_get_delta_position();
        }
        if (!auto_winding_allowed) {
            if (input_get_key_down(Button::A))
                one_turn_dir = 1;

            if (input_get_key_down(Button::B))
                change_layer = true;
        }
    }
}



void OrthocyclicRound::on_update_style(StringItem* item, MenuEvent evt)
{
    switch (evt) {
        case MenuEvent::Right:
            if ((int)style<2)
                style = (Style)((int)style+1);
            break;
        case MenuEvent::Left:
            if ((int)style>0)
                style = (Style)((int)style-1);
            break;
        default:
            break;
    }
    item->value = std::string(style_strings[(int)style]);
}

void OrthocyclicRound::init_menu(std::string path)
{
    Coil::init_menu(path);

    // Editable settings
    menu = MenuSystem::instance.get_or_create(path);
    menu->add(new StringItem(menu, "style",
                             [&] (StringItem* item) { on_update_style(item, MenuEvent::Null); },
                             [&] (StringItem* item, MenuEvent evt) { on_update_style(item, evt); }));
    menu->add(new BoolItem(menu, "fill-last", [&] () -> bool { return fill_last; },
                           [&](bool v) { fill_last = v; }));
    // Read only settings
    menu->add(new IntItem(menu, "-turns-odd", [&] () -> int { return turns_odd; }, nullptr));
    menu->add(new IntItem(menu, "-turns-even", [&] () -> int { return turns_even; }, nullptr));
    menu->add(new IntItem(menu, "-turns-last.", [&] () -> int { return turns_last; }, nullptr));

    menu->add(new FloatItem(menu, "-coil-od", [&] () -> float { return coil_od; }, nullptr));
    menu->add(new FloatItem(menu, "-coil-od-c.", [&] () -> float { return coil_od_cross; }, nullptr));
    menu->add(new FloatItem(menu, "-wind-l", [&] () -> float { return winding_len; }, nullptr));
    menu->add(new FloatItem(menu, "-wind-h", [&] () -> float { return winding_h; }, nullptr));
    // Actions
    menu->add(new ActionItem(menu, "start", [&] (MenuItem* it, MenuEvent e) { start(); }));
    menu->add(new ActionItem(menu, "stop", [&] (MenuItem* it, MenuEvent e) { stop(); }));
}

static const float sin60 = 0.86602540378;

/**
 *  The size of crossover is 15 degrees, so there will be 24
 *  possible positions.
 */
static const int num_crossover_sections = 24;
static const int max_crossover_section = num_crossover_sections - 1;
static const float crossover_size_norm = 1.0 / (float)num_crossover_sections;

/**
 * Get the position of cross over for given layer
 * The first layer starts from the last secrtion (23)
 */
static int get_crossover_section(int layer) {
    return (num_crossover_sections - layer) % num_crossover_sections;
}

/**
 * Get crosover normalized position
 */
static float get_crossover_norm(int layer) {
    return get_crossover_section(layer) / num_crossover_sections;
}

static float get_normalized_position(float v, float min, float max) {
    return (v-min) / (max-min);
}

/**
 * Slow down ant the layer's enad and accelerate at begin
 **/
unit_t OrthocyclicRound::get_velocity_factor(int layer_turn, int layer_turns) {
    if (layer_turn<=accelerate_turns) {
        return clamp01(get_normalized_position(layer_turn, 0, (float)accelerate_turns));
    } else if (layer_turn >= (layer_turns-deccelerate_turns+1)) {
        return clamp01(get_normalized_position(layer_turn, (float)(layer_turns-deccelerate_turns+1), (float)layer_turns));
    } else {
        return (unit_t)1.0f;
    }
}


static void display_status(int turn, int turns, int layer, int layers, float x, float rpm)
{
    ESP_LOGI(TAG, "Wind turn: %d of %d layer: %d of %d x: %f rpm: %f",
             turn, turns, layer, layer, x, rpm);

    MenuSystem::instance.set_visible(false);
    display_clear();
    char buf[17];
    std::snprintf(buf, 16, "T: %d/%d L: %d/%d", turn, turns, layer, layers);
    display_print(0,0, buf);
    std::snprintf(buf, 16, "X%.2f F%.1f", x, rpm);
    display_print(0,1, buf);
    display_update();
}

///////////////////////////////////////////////////////////////////////////////
// Build gcode from the wire diameter and make it "orthocyclic equal"        //
// see the manual https://en.wikipedia.org/wiki/Coil_winding_technology      //
// The orthocyclic equal has the same amout of turns at each layer           //
// but each layer is shifted for half of wire diameter. See image below:     //
//                                                                           //
//  |( )( )( )( )( ) |                                                       //
//  | ( )( )( )( )( )|                                                       //
//  |( )( )( )( )( ) |                                                       //
//  |----------------|                                                       //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// The display is the menu until we start the winding.                       //
// While we are wining the display shows the stats on the screen.            //
// Quad button can enable the menu. To disable we have to use menu           //
// option or one other A or B buttons.                                       //
//                                                                           //
// ** Table 1: The functions when the winding process started                //
// |     | Not Winding         | Winding       | Completing                | //
// |-----|---------------------|---------------|---------------------------| //
// |  Ab | Released            | Hold          | (Click) One turn          | //
// |  Bb |                     |               | (Click) Next layer        | //
// | Q-+ | Unwind -1 turn      | *ControlSpeed | Wind + 1 turn             | //
// |  Qb | Menu                |               | Menu                      | //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

void OrthocyclicRound::process()
{
    // Make current position as (0,0)
    Kinematic::instance.set_origin();
    // Set the global position x and truns counter 0
    auto posx = 0.0f;
    auto turn = 0;

    // The layer iterator
    for (auto layer = 1; layer<=layers; layer++) {

        // At each layer activate autowinding feature
        // and defautivate 'change direction' and
        // 'single turn'
        auto_winding_allowed = true;
        change_layer = false;
        one_turn_dir = 0;

        // Compute the layer's turns and direction
        auto odd_layer = layer & 1;
        auto last_layer = layer == layers;
        auto layer_turns = last_layer ? turns_last : (odd_layer ? turns_odd : turns_even);
        auto direction = layer & 1;

        // Crossover position should change every layer
        auto cross_section = get_crossover_section(layer);
        auto cross_starts = get_crossover_norm(layer);
        auto cross_ends = (cross_starts + crossover_size_norm);
        ESP_LOGI(TAG, "Crossover section %d range [0 - %f - %f - 1.0]", cross_section, cross_starts, cross_ends);

        // Add extra turns for the manual direction
        auto max_turns = layer_turns + (manual_direct ? ALLOW_EXTRA_TURNS : 0);
        // Deactivate auto-winding before end of layer
        auto auto_stop_at = layer_turns - stop_before + 1;

        // The layer's turns iterator
        int layer_turn = 0;
        do {

            // Wait operator's control.
            while (true) {
                if (auto_winding_allowed) {
                    // The autowinding feature <enabled>
                    // Waits the 'auto_winding' only.
                    if (auto_winding)
                        break;
                } else  {
                    // The autowinding feature <disabled>
                    // The operator can start the one turn
                    // or to complete the layer
                    if (one_turn_dir || change_layer)
                        break;
                }
                vTaskDelay(1/portTICK_PERIOD_MS);
            }

            if (!change_layer) {
                // There is no request to change the layer
                // make single turen forward or backaward
                if (auto_winding || one_turn_dir > 0) {
                    // The forward turn
                    turn++;
                    layer_turn++;
                    // Get velocity for current turn
                    auto v = get_velocity_factor(layer_turn, layer_turns);
                    auto rpm = lerp(v, 20.0f, (float)speed);
                    // Increment or decrement X position and remeber previous
                    auto oldx = posx;
                    posx += (direction ? wire_od : -wire_od);

                    // display current turn and layer on LCD
                    display_status(turn, total_turns, layer, layers, posx, rpm);

                    if (cross_section == 0) {
                        // crossover at the begin of turn
                        Kinematic::instance.move_to(posx, ((turn-1) + cross_ends), rpm);
                        Kinematic::instance.move_to(posx, turn, rpm);
                    } else if (cross_section == max_crossover_section) {
                        // crossover at the end of turn
                        Kinematic::instance.move_to(oldx, ((turn-1) + cross_starts), rpm);
                        Kinematic::instance.move_to(posx, turn, rpm);
                    } else {
                        // crossover at the middle of turn
                        Kinematic::instance.move_to(oldx, ((turn-1) + cross_starts), rpm);
                        Kinematic::instance.move_to(posx, ((turn-1) + cross_ends), rpm);
                        Kinematic::instance.move_to(posx, turn, rpm);
                    }

                } else if (one_turn_dir < 0) {
                    // Unwind single turn only for current layer
                    if (layer_turn > 1) {
                        // Decrease amount of turns
                        layer_turn--;
                        turn--;
                        // Get the velocity
                        auto v = get_velocity_factor(layer_turn, layer_turns);
                        auto rpm = lerp(v, 20.0f, (float)speed);

                        // Increment or decrement X pposition
                        posx -= (direction ? wire_od : -wire_od);

                        // Display the status
                        display_status(turn, total_turns, layer, layers, posx, rpm);

                        // Perform operation
                        Kinematic::instance.move_to(posx, turn, rpm);
                    } else {
                        ESP_LOGW(TAG, "Can't unwind more than the current layer");
                    }
                }
                // Deactivate single turn
                one_turn_dir = 0;

                // Stop autowinding for manual reversing
                if (manual_direct && layer_turn >= auto_stop_at) {
                    auto_winding_allowed = false;
                    auto_winding = false;
                }

                vTaskDelay(1/portTICK_PERIOD_MS);
            }
        } while (layer_turn<max_turns && !change_layer);

        ESP_LOGW(TAG, "Change the layerr");
        // Change layer and direction
        posx += (odd_layer ? xshift_odd : xshift_even);
        Kinematic::instance.move_to(posx, turn, (float)speed);
    }
    printf("\nCOMPLETE %d LAYERS AND %d TURNS\n", layers, turn);
    winding_task_handle = NULL;
    vTaskDelete(NULL);
}

#define WINDING_TASK_PRIO 2

void c_winding_task(void* arg)
{
    ((OrthocyclicRound*)arg)->process();
}
/** Start winding process */
void OrthocyclicRound::start()
{
    if (is_winding()) {
        ESP_LOGW(TAG,"The coild is already winding");
    } else {
        ESP_LOGI(TAG, "Start winding task");
        inspect();
        xTaskCreate(c_winding_task, "winding_task", 4096, this, WINDING_TASK_PRIO, &winding_task_handle);
    }
}

void OrthocyclicRound::stop() {
    if (is_winding()) {
        ESP_LOGI(TAG,"Stop winding");
        vTaskDelete(winding_task_handle);
        winding_task_handle = NULL;
    }
}

void OrthocyclicRound::update_config() {
    version = menu->get_version();
    auto wire_rad = wire_od / 2;
    auto layer_max_len = bob_len;
    auto turns_per_row = 0;

    switch (style) {
        case Style::Equal:
            //  make the maximum lenght smaller by one ire radius
            layer_max_len -= wire_rad;
            turns_per_row = floor(layer_max_len / wire_od);
            turns_odd = turns_per_row;
            turns_even = turns_per_row;
            xshift_odd = wire_rad;
            xshift_even = -wire_rad;
            break;
        case Style::FirstShort:
            turns_per_row = floor(layer_max_len / wire_od);
            turns_odd = turns_per_row-1;
            turns_even = turns_per_row;
            xshift_odd = wire_rad;
            xshift_even = -wire_rad;
            break;
        case Style::FirstLong:
            turns_per_row = floor(layer_max_len / wire_od);
            turns_odd = turns_per_row;
            turns_even = turns_per_row-1;
            xshift_odd = -wire_rad;
            xshift_even = wire_rad;
            break;
        default:
            break;
    }

    // Compute the layers quantity by iteractyion
    auto turns = 0;
    auto layer = 1;

    for ( ; layer<1000; layer++) {
        turns_last = (layer&1) ? turns_odd : turns_even;
        turns += turns_last;
        if (turns >= wire_turns)
            break;
    }

    layers = layer;

    if (fill_last) {
        // Make turns amout to fill the last layer
        total_turns = turns;
    } else {
        // Make exact amount of turns
        auto overflow_turns = turns - wire_turns;
        turns_last -= overflow_turns;
        total_turns = turns - overflow_turns;
    }

    {
        // The lenght of the coil is different from
        // amount of layers and the style of coil.
        // Single layer coil can be short, it depends
        // of turns.
        if (layers == 1) {
            winding_len = turns_per_row * wire_od;
        } else {
            winding_len = turns_per_row * wire_od;
            // But the Equal mode will expland
            // the lenght by the wire radius
            if (style == Style::Equal)
                winding_len += wire_rad;
        }
    }

    // Calculation of the winding height in the layer cross
    winding_h = wire_od * (1 + (sin60 * (layers - 1)));
    // section area just add 5%
    winding_h_cross = winding_h * 1.05;
    coil_od = coil_id + (2 * winding_h);
    coil_od_cross = coil_id + (2 * winding_h_cross);
    // Display result
    inspect();
}

/**
 * Print all settings to the  terminal
 **/
void OrthocyclicRound::inspect()
{
    // Print the arguments
    printf("Inputs:\n");
    printf("  Wire diameter   = %f mm\n", wire_od);
    printf("  Wire turns      = %d\n", wire_turns);
    printf("  Coil length     = %f mm\n", bob_len);
    printf("  Coil int. diam  = %f mm\n", coil_id);
    printf("  Coil style      = %s\n", style_strings[(int)style]);
    printf("  Coil fill last  = %d mm\n", fill_last);
    // Computed parameters
    printf("Computed:\n");
    printf("  Turns odd       = %d\n", turns_odd);
    printf("  Turns even      = %d\n", turns_even);
    printf("  Turns last      = %d\n", turns_last);
    printf("  Total Turns     = %d\n", total_turns);
    printf("  X-shift odd     = %f\n", xshift_odd);
    printf("  X-shift even    = %f\n", xshift_even);
    printf("  Layers num      = %d\n", layers);
    printf("  Winding len     = %f mm\n", winding_len);
    printf("  Winding H       = %f mm\n", winding_h);
    printf("  Winding H cros s= %f mm\n", winding_h_cross);
    printf("  Coil OD         = %f\n", coil_od);
    printf("  Coil OD cross   = %f mm\n", coil_od_cross);
}
