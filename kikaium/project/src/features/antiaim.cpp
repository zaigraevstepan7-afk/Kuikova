#include "antiaim.h"

void antiaim::force_move(c_player_controller *player, const ImVec2 &direction)
{
    if (!player)
        return;
    c_movement_controller *movement{};
    c_player_translation_parameters *params{};
    c_walk_parameters *walk_parameters{};
    c_translation_data *data{};
    blended_value_t<ImVec2> *dir{};
    float speed{};

    movement = player->m_pMovement;
    if (!movement)
        return;

    data = movement->m_pTranslationData;
    params = movement->m_pTranslationParams;

    if (!params || !data)
        return;

    walk_parameters = params->m_pWalkParameters;
    if (!walk_parameters)
        return;

    dir = *reinterpret_cast<blended_value_t<ImVec2> **>(reinterpret_cast<uintptr_t>(data) + oxorany(0xe0));
    if (!dir)
        return;

    speed = walk_parameters->m_fSpeed;

    dir->m_actual = {direction.x * speed, direction.y * speed};
}

void antiaim::movement_fix(float orig_yaw, float antiaim_yaw, c_player_inputs *inputs, c_player_controller *local)
{
    if (!local || !inputs)
        return;
    if (inputs->vertical == 0.f && inputs->horizontal == 0.f)
        return;

    auto const deg2rad = [](float deg)
    {
        return deg * static_cast<float>(M_PI) / 180.f;
    };

    auto const normalize_yaw = [](float yaw)
    {
        if (yaw >= 360.f)
            yaw = fmodf(yaw, 360.f);
        if (yaw < 0.f)
            yaw = 360.f + fmodf(yaw, 360.f);
        return yaw;
    };

    float const f1 = normalize_yaw(orig_yaw);
    float const f2 = normalize_yaw(antiaim_yaw);

    float yaw_delta = f2 - f1;
    if (yaw_delta > 180.f)
        yaw_delta -= 360.f;
    else if (yaw_delta < -180.f)
        yaw_delta += 360.f;

    float const forward = inputs->vertical;
    float const side = inputs->horizontal;

    float const rad = deg2rad(yaw_delta);
    float const cos_rad = cosf(rad);
    float const sin_rad = sinf(rad);

    ImVec2 fixed_move{
        cos_rad * side - sin_rad * forward,
        sin_rad * side + cos_rad * forward};

    float length = sqrtf(fixed_move.x * fixed_move.x + fixed_move.y * fixed_move.y);
    if (length > 0.f)
    {
        if (length > 1.f)
        {
            fixed_move.x /= length;
            fixed_move.y /= length;
        }
    }
    else
    {
        fixed_move = {0.f, 0.f};
    }

    this->force_move(local, fixed_move);
    inputs->vertical = fixed_move.y;
    inputs->horizontal = fixed_move.x;
}

void antiaim::init(c_player_controller *local, c_player_inputs *inputs)
{
    if (!local || !inputs)
        return;

    if (g.b_duck)
    {
        if (target_t.b_found)
            inputs->duck = true;
    }

    if (g.b_stop)
    {
        if (target_t.b_found)
        {
            inputs->horizontal = 0.f;
            inputs->vertical = 0.f;
        }
    }

    if (!g.b_antiaim && !g.b_spin)
        return;

    bool stop = false;

    c_aim_controller *c_aim = local->m_pAim;
    if (!c_aim)
        return;

    c_aiming_data *aim_data = c_aim->m_pAimingData;
    if (!aim_data)
        return;

    if (orig_camera_angles.is_empty())
        orig_camera_angles = {aim_data->cur_aim_ang.pitch, aim_data->cur_euler_ang.yaw, 0.f};

    orig_camera_angles.normalize(70.f);
    orig_camera_angles.pitch -= inputs->delta_aim_angles.x;
    orig_camera_angles.yaw += inputs->delta_aim_angles.y;

    euler_angles_t aa_angles{orig_camera_angles.pitch, orig_camera_angles.yaw, 0.f};

    switch (g.i_pitch)
    {
    case 0:
        aa_angles.pitch = 0.f;
        break;
    case 1:
        aa_angles.pitch = -90.f;
        break;
    case 2:
        aa_angles.pitch = 90.f;
        break;
    default:
        break;
    }

    float base_yaw = orig_camera_angles.yaw;

    switch (g.i_yaw)
    {
    case 0:
        if (stop)
            return;
        base_yaw = 0.f;
        break;
    case 1:
        if (stop)
            return;
        base_yaw = 165.f;
        break;
    case 2:
        if (stop)
            return;
        static float ang = 0.f;
        static float rad = 0.f;
        ang += 8.f;
        rad += 0.5f;
        if (ang >= 360.f)
            ang = rad = 0.f;
        base_yaw = ang + (sinf(rad) * 180.f);
        break;
    case 3:
        if (stop)
            return;
        aa_angles.pitch = static_cast<float>(rand() % 179 - 89);
        base_yaw = static_cast<float>(rand() % 360);
        break;
    default:
        break;
    }

    if (g.b_chaos && inputs->jump)
    {
        if (stop)
            return;
        aa_angles.pitch = static_cast<float>(rand() % 179 - 89);
        base_yaw = static_cast<float>(rand() % 360);
    }

    if (g.f_speed != 0.f)
    {
        if (stop)
            return;
        static float spin_angle = 0.f;
        spin_angle += g.f_speed;
        if (spin_angle >= 360.f)
            spin_angle -= 360.f;
        else if (spin_angle < 0.f)
            spin_angle += 360.f;
        base_yaw = spin_angle;
    }

    if (g.b_jitter && g.f_speed == 0.f)
    {
        static int frames{};
        static bool flip{};

        if (frames >= g.frames) {
            frames = 0;
            flip = !flip;
        }
        ++frames;

        int range = g.i_range;

        base_yaw = Vector3::NormalizeAngle(base_yaw + ( flip ? range : -range ));




        // static float timer = 0.f;
        //
        //
        // float range = g.i_range * 0.5f;
        // float speed = std::clamp(g.f_jitter_speed, 0.1f, 1.0f);
        //
        // timer += speed;
        //
        // if (timer >= 1.0f)
        // {
        //     base_yaw += flip ? range : -range;
        //     flip = !flip;
        //     timer = 0.f;
        // }
    }

    float yaw_angle = Vector3::NormalizeAngle(orig_camera_angles.yaw + base_yaw);
    aa_angles.yaw = yaw_angle;
    aa_angles.normalize(70.f);

    if ((inputs->fire || target_t.fire) && c_globals->holding_gun())
        stop = true;

    if (!stop)
    {
        aim_data->cur_aim_ang.pitch = aa_angles.pitch;
        aim_data->cur_euler_ang.yaw = aa_angles.yaw;
    }
    else
    {
        base_yaw = 0.f;
        aa_angles.pitch = orig_camera_angles.pitch;
        aa_angles.yaw = orig_camera_angles.yaw;
    }

    if (aim_data)
    {
        aim_data->cur_aim_ang.pitch = aa_angles.pitch;
        aim_data->cur_euler_ang.yaw = aa_angles.yaw;
        movement_fix(orig_camera_angles.yaw, aim_data->cur_euler_ang.yaw, inputs, local);
    }
}

void (*og_filter)(c_delegate *thisptr, c_player_inputs *);
void hk_filter(c_delegate *thisptr, c_player_inputs *inputs)
{
    auto local = c_player->local;
    if (!og_filter)
        return;

    if (!local || !inputs)
    {
        c_antiaim->orig_camera_angles = {0, 0, 0};
        return og_filter(thisptr, inputs);
    }

    c_local_player::m_old_cmd = *inputs;
    c_local_player::m_cmd = inputs;

    c_antiaim->init(local, inputs);

    return og_filter(thisptr, inputs);
}

void antiaim::update()
{
    c_player_controls *controls{};
    controls = c_player->controls;
    if (!controls)
        return;

    auto filter = controls->input_filter();
    if (filter)
        filter->hook((void *)hk_filter, (void **)&og_filter);
}

void antiaim::late_update(c_player_controller *local)
{
    if ((!g.b_antiaim && !g.b_spin) || !local)
        return;

    c_transform *holder{};

    holder = local->m_pMainCameraHolder;
    if (!holder)
        return;

    holder->set_euler_angles(this->orig_camera_angles);
}