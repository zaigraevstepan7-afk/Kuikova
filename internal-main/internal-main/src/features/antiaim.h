#include "globals.hpp"
#include <imgui.h>

#pragma pack(1)
class c_player_inputs
{
    char pad[0x10];

public:
    float horizontal;
    float vertical;
    bool duck;
    bool asda;
    char padd[0x2];
    int sda;
    bool adsad;
    bool fire;
    bool jump;
    char padfgdd[0x5];
    ImVec2 delta_aim_angles; // 0x28
};
#pragma pack()

class c_local_player
{
public:
    ~c_local_player() = default;
    c_local_player() = default;

    static c_player_inputs *m_cmd;
    static c_player_inputs m_old_cmd;
};

inline c_player_inputs *c_local_player::m_cmd{nullptr};
inline c_player_inputs c_local_player::m_old_cmd{};

class antiaim
{
public:
    euler_angles_t orig_camera_angles{0, 0, 0};
    void update();
    void late_update(c_player_controller *local);
    void force_move(c_player_controller *player, const ImVec2 &direction);
    void movement_fix(float orig_yaw, float antiaim_yaw, c_player_inputs *inputs, c_player_controller *local_player);
    void init(c_player_controller *local, c_player_inputs *inputs);
    void iniliaze(c_player_inputs *inputs, float pitch, float yaw, bool stop);
};

template <typename T>
struct blended_value_t
{
    char pad[sizeof(void *) * 2];

public:
    T m_actual;
    T m_actual_prev_frame;
    T m_blended;
    T m_previous;
    float m_time_fix;
    float m_blend_duration;
};

inline antiaim *c_antiaim = new antiaim();