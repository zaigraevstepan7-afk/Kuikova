#include "world.h"

void world::change_world(c_player_controller *local)
{
    // Halalium: only "Apply World Color" commits World / Solid World Color
    if (!g.b_apply_world || !local)
        return;

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

    g.b_apply_world = false;
}

void world::fog()
{
    // Melodium-only — Halalium has no Fog menu; never apply
    (void)0;
}

void world::init(c_player_controller *local)
{
    if (!local)
        return;

    this->change_world(local);
}