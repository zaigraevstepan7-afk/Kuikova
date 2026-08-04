#include "globals.hpp"

class world
{
    public:
    void init(c_player_controller *local);
    void change_world(c_player_controller *local);
    void fog();
};

inline world *c_world = new world();