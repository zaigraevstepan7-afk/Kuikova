#include "chams.h"
#include <sdk/game/il2cpp/il2cpp.h>

c_material *create_material(const char *name)
{
    c_shader a;
    static Il2CppClass *klaz{};

    c_shader *shader = a.find(c_methods->new_str(name));
    if (shader)
    {

        if (!klaz)
            klaz = (Il2CppClass *)il2cpp_class_from_name(dll::unity, oxorany("UnityEngine"), oxorany("Material"));

        if (klaz)
        {

            c_material *mat = (c_material *)il2cpp_object_new(klaz);
            if (mat)
            {

                mat->new_material(shader);
                return mat;
            }
        }
    }
    return nullptr;
}

void set_materials(c_player_controller *player, c_material *material)
{
    if (player && material)
    {
        auto lod = player->m_pCharacterLOD;
        if (lod)
        {
            c_renderer *mesh_render = lod->skinned_mesh_render;
            if (mesh_render)
            {
                mesh_render->set_material(material);
            }
        }
    }
}

void set_gloves(c_player_controller *player, c_material *material)
{
    if (player && material)
    {
        c_arms_lod_group *arms{};
        c_renderer *gloves{};

        arms = player->m_pLOD;
        if (arms)
        {

            gloves = arms->gloves;

            if (gloves)
            {

                gloves->set_material(material);
            }
        }
    }
}

void chams::enemy(c_player_controller *player)
{
    if (!g.b_players)
        return;

    if (!player) return;
    if (!player->m_pPhoton) return;
    if (player->m_pPhoton->get_health() < 0) return;

    c_material *mat{};

    switch (g.i_players)
    {
        case 0:
            mat = create_material(oxorany("Legacy Shaders/Diffuse"));
            break;
        case 1:
            mat = create_material(oxorany("Hidden/Internal-Colored"));
            break;
        case 2:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_SrcBlend")), 1.5f);
                mat->set_float(c_methods->new_str(oxorany("_DstBlend")), 2.0f);
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 0.5f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 2.0f);
            }
            break;
        case 3:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 5.0f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 5.0f);
            }
            break;
        case 4:
            mat = create_material(oxorany("Transparent/Diffuse"));
            break;
        default:
            break;
    }

    if (mat)
    {
        if (g.i_players == 4)
            g.m_players[3] = 0.5f;

        mat->set_color(float4tocolor(g.m_players));
        set_materials(player, mat);
        mat->set_float(c_methods->new_str(oxorany("_ZTest")), 8.0f);
        mat->set_float(c_methods->new_str(oxorany("_ZWrite")), 0.0f);
    }
}

void apply_weapon(c_renderer* renderer)
{
    if (!g.b_weapon)
        return;

    c_material* mat{};

    switch (g.i_weapon)
    {
        case 0:
            mat = create_material(oxorany("Legacy Shaders/Diffuse"));
            break;

        case 1:
            mat = create_material(oxorany("Hidden/Internal-Colored"));
            break;

        case 2:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_SrcBlend")), 1.5f);
                mat->set_float(c_methods->new_str(oxorany("_DstBlend")), 2.0f);
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 0.5f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 2.0f);
            }
            break;

        case 3:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 5.0f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 5.0f);
            }
            break;

        case 4:
            mat = create_material(oxorany("Transparent/Diffuse"));
            break;

        default:
            break;
    }

    if (!mat)
        return;

    if (g.i_weapon == 4)
        g.m_weapon[3] = 0.5f;

    mat->set_color(float4tocolor(g.m_weapon));

    mat->set_float(c_methods->new_str(oxorany("_ZTest")), 8.0f);
    mat->set_float(c_methods->new_str(oxorany("_ZWrite")), 0.0f);

    renderer->set_material(mat);
}


void chams::weapon(c_player_controller *player) {
    // Weapon LOD mesh list offset is NOT in Halalium profile — feature disabled.
    (void)player;
    return;
}

void chams::hit(c_player_controller *player)
{
    if (!g.hit_chams)
        return;

    if (!player) return;

    c_material *mat{};

    switch (g.i_hit)
    {
        case 0:
            mat = create_material(oxorany("Legacy Shaders/Diffuse"));
            break;
        case 1:
            mat = create_material(oxorany("Hidden/Internal-Colored"));
            break;
        case 2:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_SrcBlend")), 1.5f);
                mat->set_float(c_methods->new_str(oxorany("_DstBlend")), 2.0f);
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 0.5f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 2.0f);
            }
            break;
        case 3:
            mat = create_material(oxorany("Standard"));
            if (mat)
            {
                mat->set_float(c_methods->new_str(oxorany("_Metallic")), 5.0f);
                mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 5.0f);
            }
            break;
        case 4:
            mat = create_material(oxorany("Transparent/Diffuse"));
            break;
        default:
            break;
    }

    if (mat)
    {
        if (g.i_hit == 4)
            g.m_hit[3] = 0.5f;

        mat->set_color(float4tocolor(g.m_hit));
        set_materials(player, mat);
    }
}

void chams::arms(c_player_controller *player)
{
    if (g.b_gloves)
    {
        if (player)
        {

            c_material *mat{};

            switch (g.i_gloves)
            {
            case 0:
                mat = create_material(oxorany("Legacy Shaders/Diffuse"));
                break;
            case 1:
                mat = create_material(oxorany("Hidden/Internal-Colored"));
                break;
            case 2:
                mat = create_material(oxorany("Standard"));
                if (mat)
                {
                    mat->set_float(c_methods->new_str(oxorany("_SrcBlend")), 1.5f);
                    mat->set_float(c_methods->new_str(oxorany("_DstBlend")), 2.0f);
                    mat->set_float(c_methods->new_str(oxorany("_Metallic")), 0.5f);
                    mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 2.0f);
                }
                break;
            case 3:
                mat = create_material(oxorany("Standard"));
                if (mat)
                {
                    mat->set_float(c_methods->new_str(oxorany("_Metallic")), 5.0f);
                    mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 5.0f);
                }
                break;
            case 4:
                mat = create_material(oxorany("Transparent/Diffuse"));
                break;
            default:
                break;
            }

            if (mat)
            {
                if (g.i_gloves == 4)
                    g.m_gloves[3] = 0.5f;

                mat->set_color(float4tocolor(g.m_gloves));
                set_gloves(player, mat);
            } // gloves
        }
    }
}

void chams::local(c_player_controller *player)
{
    if (g.b_local)
    {
        if (player)
        {
            c_material *mat{};

            switch (g.i_local)
            {
            case 0:
                mat = create_material(oxorany("Legacy Shaders/Diffuse"));
                break;
            case 1:
                mat = create_material(oxorany("Hidden/Internal-Colored"));
                break;
            case 2:
                mat = create_material(oxorany("Standard"));
                if (mat)
                {
                    mat->set_float(c_methods->new_str(oxorany("_SrcBlend")), 1.5f);
                    mat->set_float(c_methods->new_str(oxorany("_DstBlend")), 2.0f);
                    mat->set_float(c_methods->new_str(oxorany("_Metallic")), 0.5f);
                    mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 2.0f);
                }
                break;
            case 3:
                mat = create_material(oxorany("Standard"));
                if (mat)
                {
                    mat->set_float(c_methods->new_str(oxorany("_Metallic")), 5.0f);
                    mat->set_float(c_methods->new_str(oxorany("_Glossiness")), 5.0f);
                }
                break;
            case 4:
                mat = create_material(oxorany("Transparent/Diffuse"));
                break;
            default:
                break;
            }

            if (mat)
            {
                if (g.i_local == 4)
                    g.m_local[3] = 0.5f;

                mat->set_color(float4tocolor(g.m_local));
                set_materials(player, mat);
            } // local
        }
    }
}