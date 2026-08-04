#include "globals.hpp"
#include <vector>

class visual
{
public:
    void hits(c_player_controller *);
    void draw_hits();
    void remove(c_player_controller *local);
    void hitmarker();
    void third_view(c_transform * player_transform);
};

struct hit_t
{
    Vector3 start;
    Vector3 end;
    float time;
    int damage;
    c_player_controller *player{};
};
static std::vector<hit_t> hitted;

inline visual *c_visual = new visual();