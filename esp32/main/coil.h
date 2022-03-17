#ifndef COIL_H_
#define COIL_H_

#include <stdint,h>
#include <vector>

/** The turn phase */
struct Phase
{
    int x;
    int r;
    float x_velocity;
    float r_velocity;
    int ticks;
}

/** Base class for all coils */
class Coil
{
public:
    Coil();
    int m_turns;
    float m_bobin_len;
    float m_wiredia;
    Phase m_phases[3];
    int m_count;

    virtual void Start();
    void AddPhase(int x, int y, float xvel, float yvel);

protected:
    int m_ticks_per_turn;
    int m_x_ticks_per_turn;
    int m_y_ticks_per_turn;


}
/** Any round coil */
class RoundCoil : public Coil
{
public :
    RoundCoil();
    float m_coil_id;
    float m_coil_od;
    float m_wire_h;
    float m_coil_len;
}
/** Any rectanguar coil*/
class RectCoil : public Coil
{
public :
    RectCoil();
    float m_size_a;
    float m_size_b;
}


class HelicalRound : public RoundCoil {

};

class HelicalRect : public RectCoil {

};

class OrthocyclicRound : public RoundCoil
{
public:

    OrthocyclicRound();

    void Start();
    int GetLayersNum(int& last_layer_turns);

    enum class Style { Equal, FirstWider, SecondWider }
    Style m_style;
    int m_turns_odd;
    int m_turns_even;
    int m_turns_last
};



#endif // COIL_H_
