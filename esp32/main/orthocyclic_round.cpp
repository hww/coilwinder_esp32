#include <cstdio>
#include <math.h>
#include <type_traits>

#include "coil.h"
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
    , num_csections(12)
    , stop_before(STOP_BEFORE_TURNS)
    , turns_odd(0)
    , turns_even(0)
    , turns_last(0)
    , total_turns(0)
    , feed_rate(50)
    , feed_rate_norm(0)
{
    /*
     * The coil for the plastic bobin the AR prototype 4
     * wire_od = 0.45;
     * bob_len = 23.9;
     * bob_id = 24.0;
     * bob_od = 33.7;
     */



    wire_od = 0.45;
    bob_len = 24.9;
    bob_id = 24.0;
    bob_od = 33.0;
}

/** Update with fixed frequency */
void OrthocyclicRound::update()
{
    if (version != menu->get_version())
        update_config();

    if (is_winding())
    {
        // If the winding is tarted there are two possible opptions
        //
        // 1. When the menu is open and visible the buttons A and B
        //    doing exit from the menu.
        // 2. When menu is closed
        if (MenuSystem::instance.is_visible) {
            if (input_get_key_up(Button::A) || input_get_key_up(Button::B)) {
                MenuSystem::instance.set_visible(false);
            }
        } else {
            if (one_turn_dir == 0 && !change_layer)
            {
                if (!wind_extra_turns) {
                    // we do not winding extra turns then allow
                    // press and hold the button A
                    if (input_get_key(Button::A))
                        one_turn_dir = 1;

                    if (input_get_key_down(Button::B))
                        change_layer = true;

                } else {
                    // We are winding extra turns, be more careful
                    // and allow only step by step mode
                    if (input_get_key_down(Button::A))
                        one_turn_dir = 1;

                    if (input_get_key_down(Button::B))
                        change_layer = true;

                }

                if (one_turn_dir == 0 && change_layer == false) {
                    one_turn_dir = input_get_delta_position();
                }

            }
            // go to minimum speed
            if (one_turn_dir == 0 && !change_layer)
                reset_feed_rate_norm();
        }
    } else {
        if (!MenuSystem::instance.is_edit) {
            // When menu is not in edit mode and the winding
            // was not started -- just control the positions
            // of the motors X and R
            auto dir = input_get_delta_position();
            if (dir!=0) {
                if (input_get_key(Button::A)) {
                    auto pos = Kinematic::instance.xmotor.get_position();
                    Kinematic::instance.xmotor.set_target_position(pos + dir * 0.1f);
                }
                if (input_get_key(Button::B)) {
                    auto pos = Kinematic::instance.rmotor.get_position();
                    Kinematic::instance.rmotor.set_target_position(pos + dir * 0.1f);
                }
            }
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
    RoundCoil::init_menu(path);

    // Editable settings
    menu = MenuSystem::instance.get_or_create(path);
    menu->add(new StringItem(menu, "style",
                             [&] (StringItem* item) { on_update_style(item, MenuEvent::Null); },
                             [&] (StringItem* item, MenuEvent evt) { on_update_style(item, evt); }));
    menu->add(new BoolItem(menu, "fill-last", [&] () -> bool { return fill_last; },
                           [&](bool v) { fill_last = v; }));
    menu->add(new IntItem(menu, "num-csect",
                          [&] () -> int { return num_csections; },
                          [&] (int v) { num_csections = v; }));
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
float OrthocyclicRound::crossover_size_norm() {return 1.0 / (float)num_csections; }

/**
 * Get the position of cross over for given layer
 * The first layer starts from the last secrtion (23)
 */
int OrthocyclicRound::get_crossover_section_num(int layer) {
    return (num_csections - layer) % num_csections;
}

/**
 * Get crosover normalized position
 */
float OrthocyclicRound::get_crossover_norm(int layer) {
    return (float)get_crossover_section_num(layer) / (float)num_csections;
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


static void display_message(const char* msg) {
    MenuSystem::instance.set_visible(false);
    display_clear();
    display_print(0,0, msg);
    display_update();
    ESP_LOGI(TAG, "MSG '%s'",msg);
}

static int feed_rate_cnt = 0;

void OrthocyclicRound::reset_feed_rate_norm() {
    feed_rate_cnt = 0;
    feed_rate_norm = 0;
    Kinematic::instance.set_speed(MINIMUM_SPEED_FACTOR,MINIMUM_SPEED_FACTOR);
}

void OrthocyclicRound::update_feed_rate_norm() {
    feed_rate_cnt++;
    if (feed_rate_cnt == INCREASE_SPEED_EACH_N_TURNS) {
        feed_rate_cnt = 0;
        feed_rate_norm = clamp01(feed_rate_norm + INCREASE_SPEED_STEP);
    }
}
float OrthocyclicRound::get_max_feed_rate()
{
    return (float)feed_rate;
}
float OrthocyclicRound::get_min_feed_rate()
{
    return (float)feed_rate * (float)MINIMUM_SPEED_FACTOR;
}

float OrthocyclicRound::get_feed_rate()
{
    return lerp(feed_rate_norm, get_min_feed_rate(), get_max_feed_rate());
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
    MenuSystem::instance.set_visible(false);

    // Make current position as (0,0)
    Kinematic::instance.set_origin();
    Kinematic::instance.set_velocity(wire_od, crossover_size_norm());
    reset_feed_rate_norm();

    // Set the global position x and truns counter 0
    auto posx = 0.0f;
    auto turn = 0;

    // The layer's turns iterator
    int layer_turn = 0;

    // The layer iterator
    for (auto layer = 1; layer<=layers; layer++) {

        // At each layer activate autowinding feature
        // and defautivate 'change direction' and
        // 'single turn'
        wind_extra_turns = false;
        change_layer = false;
        one_turn_dir = 0;

        // Compute the layer's turns and direction
        auto odd_layer = layer & 1;
        auto last_layer = layer == layers;
        auto layer_turns = last_layer ? turns_last : (odd_layer ? turns_odd : turns_even);
        auto direction = layer & 1;

        // Crossover position should change every layer
        auto cross_section = get_crossover_section_num(layer);
        auto cross_starts = get_crossover_norm(layer);
        auto cross_ends = (cross_starts + crossover_size_norm());
        ESP_LOGI(TAG, "Crossover section %d range [%f .. %f]", cross_section, cross_starts, cross_ends);


        // Add extra turns for the manual direction
        auto max_turns = layer_turns + (manual_direct ? ALLOW_EXTRA_TURNS : 0);
        // Deactivate auto-winding before end of layer
        auto auto_stop_at = layer_turns - stop_before;

        do {

            // Wait operator's control.
            while (true) {
                // The operator can start the one turn
                // or to complete the layer
                if (one_turn_dir || change_layer)
                    break;
                vTaskDelay(1/portTICK_PERIOD_MS);
            }

            if (!change_layer) {
                // There is no request to change the layer
                // make single turen forward or backaward
                if (one_turn_dir > 0) {
                    // The forward turn
                    turn++;
                    layer_turn++;
                    // Get velocity for current turn
                    auto rpm = get_feed_rate();
                    // Increment or decrement X position and remeber previous
                    auto oldx = posx;
                    posx += (direction ? wire_od : -wire_od);

                    // display current turn and layer on LCD
                    display_status(turn, total_turns, layer, layers, posx, rpm);

                    if (cross_section == 0) {
                        // crossover at the begin of turn
                        Kinematic::instance.move_to(posx, ((turn-1) + cross_ends), rpm);
                        Kinematic::instance.move_to(posx, turn, rpm);
                    } else if (cross_section == (num_csections-1)) {
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
                    if (layer_turn > 0) {
                        // Decrease amount of turns
                        layer_turn--;
                        turn--;
                        // Get the velocity
                        auto rpm = get_feed_rate();

                        // Increment or decrement X pposition
                        posx -= (direction ? wire_od : -wire_od);

                        // Display the status
                        display_status(turn, total_turns, layer, layers, posx, rpm);

                        // Perform operation
                        Kinematic::instance.move_to(posx, turn, rpm);
                    } else {
                        one_turn_dir = 0;
                        wind_extra_turns = false;
                        if (layer > 2) {
                            layer--;
                            ESP_LOGW(TAG, "Goto previous layer");
                            display_status(turn, total_turns, layer, layers, posx, 0);
                            goto CHANGE_LAYER_BACKWARD;
                        } else {
                            ESP_LOGW(TAG, "Unwind all coil");
                            goto EXIT;
                        }
                    }
                }
                // Deactivate single turn
                one_turn_dir = 0;

                // Stop autowinding for manual reversing
                if (manual_direct) {
                    wind_extra_turns = layer_turn >= auto_stop_at;
                    if (wind_extra_turns)
                        display_message("Manual dir");
                }

                //vTaskDelay(1/portTICK_PERIOD_MS);
            }

            update_feed_rate_norm();

        } while (layer_turn<max_turns && !change_layer);

        ESP_LOGW(TAG, "Change the layer forward");
        layer_turn = 0;
        posx += (odd_layer ? xshift_odd : xshift_even);
        goto AFTER_CHANGE_LAYER;

    CHANGE_LAYER_BACKWARD:
        ESP_LOGW(TAG, "Change the layer forward");
        layer_turn = layer_turns;
        posx -= (odd_layer ? xshift_odd : xshift_even);

    AFTER_CHANGE_LAYER:
        reset_feed_rate_norm();
        Kinematic::instance.move_to(posx, turn, get_feed_rate());
    }
EXIT:
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

void OrthocyclicRound::update_coil_od(int _layers)
{
    layers = _layers;
    // Calculation of the winding height in the layer cross
    winding_h = wire_od * (1 + (sin60 * (_layers - 1)));
    // section area just add 5%
    winding_h_cross = winding_h * 1.05;
    coil_od = bob_id + (2 * winding_h);
    coil_od_cross = bob_id + (2 * winding_h_cross);
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
    // Recomend better wire OD (actualy the turn step)
    better_wire_od = (bob_len - wire_rad) / turns_per_row;

    // Compute the layers quantity by iteractyion
    auto turns = 0;
    auto layer = 1;

    if (wire_layers > 0) {
        // Make coil based on target layers
        for ( ; layer<=wire_layers; layer++) {
            turns_last = (layer&1) ? turns_odd : turns_even;
            turns += turns_last;
        }
        total_turns = turns;
        update_coil_od(layer);
    } else if (bob_od > 0) {
        // Make the coil based on target external diameter
        auto max_od = bob_od - (2*wire_od*sin60);
        for ( ; layer<=999; layer++) {
            turns_last = (layer&1) ? turns_odd : turns_even;
            turns += turns_last;
            update_coil_od(layer);
            if (coil_od_cross > max_od)
                break;
        }
        total_turns = turns;
    } else {
        // Make coil based on the total amout of turns
        for ( ; layer<=999; layer++) {
            turns_last = (layer&1) ? turns_odd : turns_even;
            turns += turns_last;
            if (turns >= wire_turns)
                break;
        }
        update_coil_od(layer);

        if (fill_last) {
            // Make turns amout to fill the last layer
            total_turns = turns;
        } else {
            // Make exact amount of turns
            auto overflow_turns = turns - wire_turns;
            turns_last -= overflow_turns;
            total_turns = turns - overflow_turns;
        }
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
    // The air gap at the end
    winding_gap = bob_len - winding_len;
    // Display result
    inspect();
}

/**
 * Print all settings to the  terminal
 **/
void OrthocyclicRound::inspect()
{
    RoundCoil::inspect();
    printf("Orthocyclic coil:\n");
    // Print the arguments
    printf("  Coil style      = %s\n", style_strings[(int)style]);
    printf("  Coil fill last  = %d\n", fill_last);
    // Computed parameters
    printf("Orthocyclic computed:\n");

    printf("  Total Turns     = %d\n", total_turns);
    printf("  Layers num      = %d\n", layers);
    printf("  Turns odd       = %d\n", turns_odd);
    printf("  Turns even      = %d\n", turns_even);
    printf("  Turns last      = %d\n", turns_last);
    printf("  X-shift odd     = %.2f mm\n", xshift_odd);
    printf("  X-shift even    = %.2f mm\n", xshift_even);
    printf("  Winding len     = %.2f mm\n", winding_len);
    printf("  Winding H       = %.2f mm\n", winding_h);
    printf("  Winding H cross = %.2f mm\n", winding_h_cross);
    printf("  Winding gap     = %.2f mm\n", winding_gap);
    printf("  Better wire OD  = %.2f mm\n", better_wire_od);
    printf("  Coil OD         = %.2f mm\n", coil_od);
    printf("  Coil OD cross   = %.2f mm\n", coil_od_cross);
}
