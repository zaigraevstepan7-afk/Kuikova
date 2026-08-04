#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include "includes/structs.h"
#include "includes/oxorany/Oxorany.hpp"
#include <android/log.h>
#include "includes/matrix.h"
#include "includes/Obfuscate.hpp"
#include "includes/halalium_mem.h"
#include <vector>
#include <atomic>
#include "sdk/OffsetsBridge.h"
using namespace structs;
inline uintptr_t base;
// Wired once after soft update::init — egl must not call game APIs before this.
inline std::atomic<bool> g_sdk_ready{false};

#define str_(text) \
    []() noexcept { \
        static char buff[sizeof(text)]; \
        if (buff[0] == 0) { \
            memcpy(buff, oxorany(text), sizeof(buff) - 1); \
            buff[sizeof(buff) - 1] = '\0'; \
        } \
        return buff; }()

// Write to /sdcard so user can pull the file without logcat filters.
inline void kikaium_file_log(const char *fmt, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    const char *paths[] = {
        "/sdcard/Download/kikaium.log",
        "/sdcard/kikaium.log",
        "/storage/emulated/0/Download/kikaium.log",
        "/storage/emulated/0/kikaium.log",
    };
    for (const char *p : paths)
    {
        FILE *f = fopen(p, "a");
        if (!f)
            continue;
        fprintf(f, "%s\n", buf);
        fclose(f);
        break;
    }
}

#define LOGD(fmt, ...) \
    do { \
        __android_log_print(ANDROID_LOG_DEBUG, "tenmi", fmt, ##__VA_ARGS__); \
        kikaium_file_log(fmt, ##__VA_ARGS__); \
    } while (0)

#define LOGI(fmt, ...) \
    do { \
        __android_log_print(ANDROID_LOG_INFO, "kikaium", fmt, ##__VA_ARGS__); \
        kikaium_file_log(fmt, ##__VA_ARGS__); \
    } while (0)
class c_player_controller;
class c_playermanager;
class c_player_controls;
class c_game_controller;
class c_photon_player;
class c_component;
class c_weapon_controller;
class c_gun_controller;
class c_weapon_parameters;
class c_gun_parameters;
class globals
{
public:
    bool is_allocated(void *x);
    void *get_lazysingleton_typeinfo(uintptr_t addr);
    void *type_info_instance(uintptr_t addr, uintptr_t field);
    Vector3 world2screen(const Matrix &viewMatrix, const Vector3 &pos);
    bool is_bone_visible(Vector3 start, Vector3 end_pos);
    bool is_alive(c_player_controller *player);
    bool is_enemy(c_player_controller *local, c_player_controller *enemy);
    bool holding_gun();
    bool is_sniper();
    void updateGun();
    void update();
    void updateTarget();
    void init();
    bool is_scoped();
};

inline struct target
{
public:
    bool b_found{};
    Vector3 pos{};
    Vector3 camera_pos{};
    bool fire;
    bool lag;
    c_player_controller *player{};
    bool visible{};
    bool head{};
    int damage{};
    c_player_controller *hitted{};
} target_t;

class methods
{
public:
    int (*get_width)();
    int (*get_heigth)();
    int (*get_count)();
    UnityEngine_Touch_o (*get_touch)(int id);
    void *(*game_controller)();
    bool (*linecast)(Vector3 start, Vector3 end, raycast_hit_t *hitInfo, int layerMask);
    bool (*sphere_cast)(ray_t ray, float radius, raycast_hit_t *hit, float max, int layer);
    monoString *(*create_string)(void *, const char *, int, int);
    monoString *new_str(const char *text)
    {
        return create_string(NULL, text, 0, std::strlen(text));
    }
    void (*set_active)(void *, bool val);
    void (*set_flags)(void *, uint8_t);
    void (*set_bagcolor)(void *, color_t);
    c_component *(*get_collider)(raycast_hit_t);
    uint8_t (*from_tag)(c_component *);
    void *(*object_instantiate)(void *game_object, Vector3 pos, Quaternion rot);
    void (*object_destroy)(void *game_object, float time);
    bool (*is_native_object_alive)(void *game_object);

};

class offsets
{
public:
    // --- TypeInfo RVAs (Halalium emu / script.json via OffsetsBridge) ---
    const uint32_t c_anticheatmanager = (uint32_t)Offsets::TypeInfo::AntiCheatManager;
    const uint32_t c_player_controls = (uint32_t)Offsets::TypeInfo::PlayerControls;
    const uint32_t c_player_manager = (uint32_t)Offsets::TypeInfo::PlayerManager;
    const uint32_t c_game_controller = (uint32_t)Offsets::TypeInfo::GameController;
    const uint32_t c_player_controller = (uint32_t)Offsets::TypeInfo::PlayerController;

    // Il2CppClass.static_fields offset (Halalium-confirmed 0.39.2)
    const uint32_t il2cpp_static_fields = (uint32_t)Offsets::Il2Cpp::klass_static_fields;

    // inputs (UnityEngine.Input)
    const uint32_t get_touch = oxorany(0x684EDAC); // GetTouch
    const uint32_t get_count = oxorany(0x684E370); // get_touchCount

    // screen
    const uint32_t get_width = oxorany(0x5FB3224);
    const uint32_t get_heigth = oxorany(0x5FB5478);

    // transform
    const uint32_t get_position = oxorany(0x6005138);
    const uint32_t set_position = oxorany(0x6009694);
    const uint32_t get_forward = oxorany(0x60062E0);
    const uint32_t get_up = oxorany(0x6002608);
    const uint32_t get_euler_angles = oxorany(0x6006BF0);
    const uint32_t set_euler_angles = oxorany(0x5FEC6CC);
    const uint32_t get_rotation = oxorany(0x5FF2184);

    // playercontroller — adjacent void() pair near class end (view switch); set_visible = refresh helper
    const uint32_t set_tps = oxorany(0x8E7E63C);     // ECAFHEAAGDAAHHH
    const uint32_t set_fps = oxorany(0x8E7EC48);     // FCBDBECBFEABFCD
    const uint32_t set_visible = oxorany(0x8E880E4); // AFAEGDBAFECEHDD
    const uint32_t set_view_mode = oxorany(0x8E91C64); // ADEAAACFHADDAFG(GGBGGDAGBGGGACD)

    // shader
    const uint32_t find = oxorany(0x6A95144); // Shader.Find

    // material
    const uint32_t get_texture = oxorany(0x6A92FA4);  // get_mainTexture
    const uint32_t set_texture = oxorany(0x6A81174);  // set_mainTexture
    const uint32_t get_shader = oxorany(0x6A82020);
    const uint32_t set_shader = oxorany(0x6A88CA8);
    const uint32_t new_material = oxorany(0x6A98518); // Material..ctor(Shader)
    const uint32_t set_color = oxorany(0x6A96904);
    const uint32_t set_int = oxorany(0x6A84BE4);   // SetInt(string,int)
    const uint32_t set_float = oxorany(0x6A8BF28); // SetFloat(string,float)

    // type
    const uint32_t get_type = oxorany(0x5CF2D40);             // Type.GetType(string)
    const uint32_t find_objects_of_type = oxorany(0x5FF2770); // Object.FindObjectsOfType(Type)

    // object
    const uint32_t object_instantiate = oxorany(0x5FEFA04); // Instantiate(Object, Vector3, Quaternion)
    const uint32_t object_destroy = oxorany(0x5FF508C);     // Destroy(Object, float)
    const uint32_t is_native_object_alive = oxorany(0x5FF2608);

    // renderer
    const uint32_t get_material = oxorany(0x6A962B4);
    const uint32_t set_material = oxorany(0x6A91498);
    const uint32_t get_materials = oxorany(0x6A9149C);
    const uint32_t set_materials = oxorany(0x6A8EFD8); // SetMaterialArray

    // gameobject
    const uint32_t set_active = oxorany(0x6009294); // SetActive

    // fog — UnityEngine.RenderSettings (prefer over old manager path)
    const uint32_t rendersettings_set_fog = oxorany(0x6A8C9C8);
    const uint32_t rendersettings_set_fog_color = oxorany(0x6A96AA8);
    const uint32_t rendersettings_set_fog_start = oxorany(0x6A8DAAC);
    const uint32_t rendersettings_set_fog_end = oxorany(0x6A93CD8);
    const uint32_t rendersettings_set_fog_mode = oxorany(0x6A85504);
    // legacy names kept for any leftover refs (point at set_fog / set_fog_color)
    const uint32_t rendersettings_manager = oxorany(0x6A8C9C8);
    const uint32_t call_rendersettings_update = oxorany(0x6A96AA8);

    // physics
    const uint32_t linecast = oxorany(0x7A020F4);    // Linecast(V3,V3,RaycastHit,int)
    const uint32_t sphere_cast = oxorany(0x79FFF6C); // SphereCast(Ray,float,RaycastHit,float,int)

    // charactercontroller
    const uint32_t get_velocity = oxorany(0x7A05344);

    // raycast icall slot — still needs live libil2cpp scan on device if hook fails
    const uint32_t ray = oxorany(0x84DB9F0); // TODO: verify Present/icall table on 0.39.2 binary

    // icall
    const uint32_t icall = oxorany(0x5CAAE90); // il2cpp_resolve_icall

    // camera
    const uint32_t get_main = oxorany(0x5FACE04);
    const uint32_t set_fov = oxorany(0x5FB7BC0);      // set_fieldOfView
    const uint32_t set_flags = oxorany(0x5FCAD3C);    // set_clearFlags
    const uint32_t set_bagcolor = oxorany(0x5FC5C24); // set_backgroundColor
    const uint32_t fsdfds = oxorany(0x5FBA5F4);       // get_worldToCameraMatrix_Injected
    const uint32_t get_projection_matrix_injected = oxorany(0x5FA9898);

    // component
    const uint32_t get_transform = oxorany(0x5FF113C);
    const uint32_t get_game_object = oxorany(0x5FFAEFC);

    // other — String.CreateString(sbyte*/object, int, int)
    const uint32_t create_string = oxorany(0x5DE99CC);

    // api (api_0.39.2.txt)
    const uint32_t il2cpp_domain_get = oxorany(0x5C86D54);
    const uint32_t il2cpp_domain_assembly_open = oxorany(0x5C86D58);
    const uint32_t il2cpp_assembly_get_image = oxorany(0x5C86BFC);
    const uint32_t il2cpp_class_from_name = oxorany(0x9D5B510);
    const uint32_t il2cpp_array_new = oxorany(0x5C86BE0);
    const uint32_t il2cpp_object_new = oxorany(0x5C86EF8);
};

struct Il2CppClass;
class callback
{
public:
    c_player_controller *local_player{};
    c_player_controller *enemy_player{};
    c_playermanager *player_manager{};
    c_player_controls *player_controls{};
    Il2CppClass *playermanager;
    void *weaponry_controller{};
    void *weapon_controller{};
    void *weapon_parameters{};
    c_photon_player *photon{};
};

class egl
{
public:
    int heigth;
    int width;
};

// Halalium-style safe load: maps-checked LDR (see includes/halalium_mem.h).
template <typename T>
struct safe_type
{
    T value{};
    bool has_value{false};

    static safe_type load(uintptr_t addr)
    {
        safe_type out{};
        out.has_value = hmem::read(addr, out.value);
        return out;
    }

    static safe_type field(void *obj, uintptr_t offset)
    {
        if (!obj)
            return {};
        return load(reinterpret_cast<uintptr_t>(obj) + offset);
    }
};


class player
{
public:
    c_player_controller *enemy{};
    std::vector<c_player_controller *> list{};
    std::vector<c_player_controller *> entity{};
    c_player_controller *local{};

    void reset();
    void collect(c_player_controller *player);
    void after_match();
    void update();

    c_weapon_controller *weapon_controller{};
    c_gun_controller *gun_controller{};
    c_weapon_parameters *weapon_parameters{};
    c_gun_parameters *gun_parameters{};
    c_game_controller *game{};
    c_player_controls *controls{};
};

inline globals *c_globals = new globals();
inline offsets *c_offsets = new offsets();
inline callback *c_callback = new callback();
inline egl *c_egl = new egl();
inline methods *c_methods = new methods();
inline player *c_player = new player();

inline const char *sky_flags[] = {"none", "skybox", "color", "solid", "depth", "nothing"};
inline const char *pitch_[] = {("local"), ("up"), ("down")};
inline const char *yaw_[] = {("local"), ("backward"), ("spiral"), ("chaos")};
inline const char *enemy_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *weapon_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *local_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *hands_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *arms_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *hit_[] = {("solid"), ("flat"), ("glass"), ("glow"), ("transparent")};
inline const char *knifes[] = {"m9", "karambit", "jkommando", "butterfly", "flip", "kunai", "scorpion", "tanto", "daggers", "kukri", "stilet", "mantis", "fang", "sting", "hands"};

inline struct g_
{
    // esp — Halalium menu_body @0x1db874 only
    bool b_esp;
    bool b_line;
    bool b_rect;
    int i_box_type = 0;       // Halalium "Box Type" 0=Full 1=Corner
    float f_corner_size = 10.f; // Halalium "Corner Size"
    bool b_health;
    bool b_skeleton;          // Halalium "Bone" (visual)
    bool b_distance = true;   // Halalium "Distance"
    float m_distance[4] = {1.f, 1.f, 1.f, 1.f};
    float m_accent[4] = {0.91f, 0.66f, 0.35f, 1.f}; // Halalium "Accent Color"
    // Melodium-only leftovers — forced OFF, no UI
    bool b_name = false;
    bool b_ammo = false;
    bool b_eweapon = false;
    float m_ammo[4] = {0.077f, 0.251f, 0.605f, 1.0f};
    float m_health[4] = {0.025f, 0.560f, 0.025f, 1.0f};
    float m_rect[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float m_skeleton[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool b_tracer = false;
    float m_tracer[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool b_marker = false;
    float m_marker[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool b_dmarker = false;
    bool b_sk = false;
    float m_sk[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    bool b_watermark = true;

    // rage — Halalium: Silent Aim / Bone / Auto Fire / Auto Wall / No spread / Wallshot / Inf Ammo / Fire Rate
    bool b_silent;
    bool b_silent_bone = true; // Halalium rage "Bone" (prefer head)
    bool b_fire;
    bool b_endless;       // Inf Ammo
    bool b_nospread;      // No spread
    bool b_autowall;      // Auto Wall
    bool b_through_walls; // Through Walls
    bool b_wallshot;      // Wallshot
    bool b_fov_check;     // Fov Check
    float f_fov_check = 120.f;
    float m_fov_color[4] = {1.f, 0.2f, 0.2f, 0.35f};
    bool b_duck = false;
    bool b_stop = false;
    int hitchance = 75;
    bool hitbox[4] = {true, false, false, false};
    bool b_dt = false;
    bool predict = false;
    float factor = 1;
    int fak = 5;
    float dist = 1.65;
    float mom = 2.5;

    // skins
    bool b_skin_changer;
    int i_skin_weapon = 0;
    int i_skin_id = 0;

    // anti aim — Halalium: Anti Aim / Pitch / Spin / Reverse Spin
    bool b_antiaim;
    bool b_spin;
    bool b_reverse_spin = false;
    int i_pitch = 0;
    int i_yaw = 0;
    bool b_jitter = false;
    int i_range = 0;
    float f_speed = 10.f;
    bool b_chaos = false;
    float f_jitter_speed = 0.3f;
    int frames = 1;

    // misc — Melodium-only forced off
    bool b_strafer = false;
    bool b_walk = false;
    bool b_third;
    int knife = 0;
    bool b_peek = false;
    bool b_aspect = false;
    float f_aspect = 1.5;
    bool b_scope = false;
    float m_fov = 59.9f;

    // chams
    bool b_players;
    bool b_local;
    bool b_gloves = false;
    float m_players[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_local[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_gloves[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_hit[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_weapon[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    int i_local = 0;
    int i_players = 0;
    int i_gloves = 0;
    int i_hit = 0;
    int i_weapon = 0;
    bool hit_chams = false;
    bool weapon_chams = false;

    // world — Halalium World / Solid / Apply (no Melodium fog/sky)
    bool b_world;
    bool b_solid = false;
    bool b_apply_world = false;
    bool b_fog = false;
    float m_world[4] = {0.500f, 0.500f, 0.500f, 0.700f};
    float m_fog[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float f_start = 5.0f;
    float f_end = 20.0f;
    bool b_sky = false;
    float m_sky[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    int c_flags = 2;

    // exploits — Melodium god/OHK off; Fire Rate is Halalium
    bool b_onehit = false;
    bool b_frate = false;
    bool b_god = false;
    bool sethp = false;
    float dur = 100;
    bool b_weapon = false;
    bool b_bomb = false;

    int steps = 2500;
    bool update_matrix = true;

} g;
enum class team_t : uint8_t;

inline bool fire{};
inline struct abc
{
    void *player{};
} abcd;
// Halalium: menu starts CLOSED; open only via ##wm_click on watermark.
inline bool open = false;
#include "sdk/game/include.h"
