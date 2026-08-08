#include "globals.hpp"
#include <mutex>
#include <vector>
#include <atomic>

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
    void draw_status(); // always-on top+bottom diagnostic strip
    Matrix matrix();
    bool update_matrix();
    void cache_matrix();  // Unity thread: Melodium VP nest + Halalium P*V
    void snapshot();      // Unity thread ONLY: cache feet/heads for EGL draw
    void clear_matrix();

    // Diagnostics (updated on Unity thread)
    std::atomic<int> dbg_players{0};
    std::atomic<int> dbg_enemies{0};
    std::atomic<int> dbg_snap{0};
    std::atomic<int> dbg_local{0};
    std::atomic<int> dbg_matrix{0};
    std::atomic<int> dbg_stage{0}; // 0 none,1 pmc,2 nest,3 ptr,4 mat fail,5 ok
    std::atomic<int> dbg_sdk{0};
    std::atomic<int> dbg_sdk_stage{0}; // 0 idle … 8 ready (see update::init)

private:
    Matrix m_cached{};
    bool m_have_matrix{false};
    Vector3 m_cam_pos{};
    std::mutex m_snap_mu;
    std::vector<EspSnap> m_snap;
};

inline esp *c_esp = new esp();
