#include "globals.hpp"
#include <mutex>
#include <vector>

struct EspSnap
{
    c_player_controller *player{};
    Vector3 foot{};
    Vector3 head{};
    int health{100};
    monoString *name{};
};

class esp {
public:
    void render();
    Matrix matrix();
    bool update_matrix();
    void cache_matrix();  // Unity thread — Halalium camera nest
    void snapshot();      // Unity thread — cache positions (no EGL get_position)
    void clear_matrix();

private:
    Matrix m_cached{};
    bool m_have_matrix{false};
    Vector3 m_cam_pos{};
    std::mutex m_snap_mu;
    std::vector<EspSnap> m_snap;
};

inline esp *c_esp = new esp();
