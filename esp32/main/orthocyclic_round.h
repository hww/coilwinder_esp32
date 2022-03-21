#ifndef ORTHOCYCLIC_H_
#define ORTHOCYCLIC_H_

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "coil.h"
#include "menu_event.h"
#include "menu_item.h"
#include "typeslib.h"

/** *******************************************************************/
/** ((((((((((((((((((((((( ORTHOCYCLIC ROUND ))))))))))))))))))))))) */
/** *******************************************************************/

/** Orthocyclic round coil */
class OrthocyclicRound : public RoundCoil
{
    public:

        OrthocyclicRound();

        void init_menu(std::string path);
        void start();
        void stop();
        void update();
        void update_config();

        void on_update_style(StringItem* item, MenuEvent evt);
        void inspect();
        void process();

        inline bool is_winding() { return winding_task_handle != NULL; }

        enum class Style { Equal, FirstShort, FirstLong };

        Style style;
        bool fill_last;
        bool manual_direct;
        // Slow accelerate for this amount of turns
        int accelerate_turns;
        // Slow decelerate this amount of turns
        int deccelerate_turns;
        // For manual direct stop befor end this amount of turns
        int stop_before;

        int turns_odd;
        int turns_even;
        int turns_last;
        int total_turns;
        float xshift_odd;
        float xshift_even;
        int layers;
        float coil_od;
        float coil_od_cross;   // where is wire cross over other wire
        float winding_len;
        float winding_h;
        float winding_h_cross;

        bool pause;
private:

        unit_t get_velocity_factor(int layer_turn, int layer_turns);

        /** Thread */
        TaskHandle_t winding_task_handle;
        bool auto_winding;
        bool auto_winding_allowed;
        int one_turn_dir;
        bool change_layer;
        int speed;
};


#endif // ORTHOCYCLIC_H_
