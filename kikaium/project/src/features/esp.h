#include "globals.hpp"

class esp {
public:
    void render();
    Matrix matrix();
    bool update_matrix();
    void cache_matrix(); // call from Unity thread (LateUpdate)
    void clear_matrix();

private:
    Matrix m_cached{};
    bool m_have_matrix{false};
};

inline esp *c_esp = new esp();
