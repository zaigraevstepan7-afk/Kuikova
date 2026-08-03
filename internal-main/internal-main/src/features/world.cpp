#include "world.h"

void world::change_world(c_player_controller *local)
{
   if (!(g.b_solid || g.b_world))
        return;
    if (local)
    {
        c_type type{};
        c_shader shader_{};

        c_type *mesh_type = type.get_type(c_methods->new_str(oxorany("UnityEngine.MeshRenderer, UnityEngine.CoreModule")));
        if (!mesh_type)
            return;

        c_shader *shader = shader_.find(c_methods->new_str(oxorany("Legacy Shaders/Diffuse")));
        if (!shader)
            return;

        auto *worldRender = mesh_type->find_objects_of_type();
        if (!worldRender)
            return;

        for (int i = 0; i < worldRender->capacity; i++)
        {
            c_renderer *renderer = worldRender->m_Items[i];
            if (!renderer)
                continue;

            auto *materials = renderer->get_materials();
            if (!materials)
                continue;

            for (int j = 0; j < materials->capacity; j++)
            {
                c_material *material = materials->m_Items[j];
                if (!material)
                    continue;

                material->set_shader(shader);

                if (g.b_solid)
                    material->set_texture(NULL);

                material->set_color(float4tocolor(g.m_world));
            }

            renderer->set_materials(materials);
        }

        g.b_solid = false;
        g.b_world = false;
    }
}

void world::fog()
{
    if (g.b_fog)
    {
        // UnityEngine.RenderSettings direct APIs (0.39.2)
        using set_fog_t = void (*)(bool);
        using set_color_t = void (*)(color_t);
        using set_float_t = void (*)(float);
        using set_mode_t = void (*)(int);

        auto set_fog = (set_fog_t)(base + c_offsets->rendersettings_set_fog);
        auto set_fog_color = (set_color_t)(base + c_offsets->rendersettings_set_fog_color);
        auto set_fog_start = (set_float_t)(base + c_offsets->rendersettings_set_fog_start);
        auto set_fog_end = (set_float_t)(base + c_offsets->rendersettings_set_fog_end);
        auto set_fog_mode = (set_mode_t)(base + c_offsets->rendersettings_set_fog_mode);

        if (set_fog && set_fog_color && set_fog_start && set_fog_end)
        {
            set_fog(true);
            if (set_fog_mode)
                set_fog_mode(2); // Linear
            set_fog_color(float4tocolor(g.m_fog));
            set_fog_start(g.f_start);
            set_fog_end(g.f_end);
        }
    }
}

void world::init(c_player_controller *local)
{
    if (!local)
        return;

    this->change_world(local);
    this->fog();
}