#ifndef COIL_H_
#define COIL_H_

#include <vector>
#include <string>

class Menu;

/** *****************************************************************/
/** (((((((((((((((((((((((((( BASE COIL )))))))))))))))))))))))))) */
/** *****************************************************************/

/** Base class for all coils */
class Coil
{
        public:
                Coil();

                virtual void init_menu(std::string name);
                virtual void start() = 0;
                virtual void stop() = 0;
                virtual void update() = 0;
                virtual void update_config() = 0;

                float bob_len;
                float wire_od;
                int wire_turns;
                int version;
                Menu* menu;
};

/** ******************************************************************/
/** (((((((((((((( ROUND AND RECTANHULAR HELOCAL COIL )))))))))))))) */
/** ******************************************************************/

/** Any round coil */
class RoundCoil : public Coil
{
        public :
                RoundCoil();

                virtual void init_menu(std::string path);

                float coil_id;
};

/** Any rectanguar coil */
class RectCoil : public Coil
{
        public :
                RectCoil();

                virtual void init_menu(std::string path);

                float size_a;
                float size_b;
};

/** *******************************************************************/
/** (((((((((((((((((((((((((( HELOCAL COIL ))))))))))))))))))))))))))*/
/** *******************************************************************/

/** Helical round coil */
class HelicalRound : public RoundCoil {
        // TODO
};

/** Helical rectangular coil */
class HelicalRect : public RectCoil {
        // TODO
};


#endif // COIL_H_
