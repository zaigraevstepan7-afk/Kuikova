#include "globals.hpp"

class update
{
    public:
    void init();
    // TypeInfo GameController refresh (Halalium: no GC VMT). Safe from egl/LateUpdate.
    void tick_lobby_cleanup();
};


inline update *c_update = new update();