#include "globals.hpp"

class esp {
    public:
    void render();
    Matrix matrix();
    bool update_matrix();
};

inline esp *c_esp = new esp();