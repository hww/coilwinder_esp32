#include <asserts.h>
#include <math.h>

#include "coil.h"
#include "step_motor.h"




// ========================================================
// Base classes
// ========================================================

Coil::Coil()
    : m_turns(1)
    , m_bobin_len(10)
    , m_wiredia(1)
    , m_winding(false)
    , m_progress(0)
    , m_phases()
    , m_count(0)
{

}
/** Start winding process */
void Coil::Start(int& tick)
{
    // Reset position
    tick = 0;
    // Get motor's data
    m_x_ticks_per_turn = motor_x.steps_per_turn * motor_x.microsteps;
    m_r_ticks_per_turn = motor_r.steps_per_turn * motor_r.microsteps;
    m_ticks_per_turn = m_x_ticks_per_turn * m_r_ticks_per_turn;

}

void Coil::AddPhase(int x, int y, float xvel, float yvel){
    m_phases[0].x = x;
    m_phases[0].y = y;
    m_phases[0].x_velocity = xvel;
    m_phases[0].y_velocity = yvel;
    m_count++;
}

// ========================================================
// Generic round coil
// ========================================================

RoundCoil::RoundCoil ()
    : Coil()
    , m_diameter(10)
{

}

void RoundCoil::Start()
{
    Coil::Start();
}

// ========================================================
// Generic rectangular coil
// ========================================================

RectCoil::RectCoil ()
    : Coil()
    , m_size_a(10)
    , m_size_b(10)
{
    Coil::Start();
}



// ========================================================
// The practical implementations
// ========================================================

OrthocyclicRound::OrthocyclicRound()
    : RoundCoil()
    , m_style(Style::Equal)
    , m_turns_odd(0)
    , m_turns_even(0)
    , m_turns_last(0)
    , m_wire_h(0)
    , m_coil_od(0)
{

}

int OrthocyclicRound::GetLayersNum(int& last_layer_turns)
{
    const int MAX_LAYERS = 1000;
    int turns = 0;       // count turns
    int layer = 1;       // layer number
    int layer_turns = 0; // later's turns amount
    for (; layer<MAX_LAYERS; layer++) {
        layer_turns = (layer & 1) ? m_turns_odd : m_turns_even;
        turns += layer_turns;
        if (turns >= m_turns) {
            // actual turns on the last layer
            last_layer_turns = layer_turns - (turns - m_turns);
            return layer;
        }
    }
    ESP_LOGE(TAG, "Can't estimate layer's number");
    return layer;
}
/** Start winding process */
void OrthocyclicRound::Start(int& tick)
{
    Coil::Start(tick);

    // The colil's characteristics and geometry
    // Amount of turns on layers
    int layer_turns = floore(m_bobin_len / m_wire_dia);
    // The lenght of the layer
    float layer_len = layer_turns * m_wire_od;
    // The amount of turn per odd, even and last layers
    switch (m_style) {
        case Style::Equal:
            // The layer have to be half-wire diaemeter longer
            // because each next layer will be shifted one side
            layer_len += 0.5 * m_wire_od;
            // Decrease turns if the layer does not fit
            if (layer_len > m_lenght) {
                layer_turns--;
                layer_len -= m_wire_od;
            }
            m_turns_odd = layer_turns;
            m_turns_even = layer_turns;
            m_coil_len = layer_len;
            break;
        case Style::FirstWider:
            m_turns_odd = layer_turns;
            m_turns_even = layer_turns-1;
            m_coil_len = layer_len;
            break;
        case Style::SecondWider:
            m_turns_odd = layer_turns-1;
            m_turns_even = layer_turns;
            m_coil_len = layer_len;
            break;
    }
    // Count amount of layers by iterations
    m_layers = GetLayersNum(m_turns_last);
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // The value 0.75 is a coefficient taken
    // form the manual
    // https://en.wikipedia.org/wiki/Coil_winding_technology#Orthocyclic_winding
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    m_wire_h = m_layers * m_wire_dia * 0.75;
    // The coil result values
    m_coil_od = m_coil_id + 2 * m_wire_h;

    AddPhase()

}

/**
 *
 * */
Coil::update(int& tick)
{
    if (motor_x.ready || motor_y.ready) {

        auto turn_num = tick / m_ticks_per_turn;
        auto turn_pos = tick % x_ticks_per_turn;
        auto phase_a = turn_pos < x+tickes_per_phase_a;
        auto turn_on_laye = turn_num / turns_per_layer;
        auto layer_even = .....;

        if (phase_a) {
            start_job
            current_tick += x_ticks_per_phase_a;
        } else {
            start_job
            current_tick += x_ticks_per_phase_b;
        }
    }
}


void Coil::pause(bool state)
{

}

void Coil::undo(bool state)
{
  /*
    auto coil_menu = new Menu("Coil...", nullptr);
    main_menu->Add(coil_menu);
    coil_menu->Add(main_menu);
    coil_menu->Add(new IntItem("Turns", &turns, 0,10000,1));
    coil_menu->Add(new IntItem("Layers", &layers, 0,10000,1));
  */

}
