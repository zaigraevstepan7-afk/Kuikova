#pragma once

#pragma pack(1)
class c_shader;
class c_material
{
public:
    void *get_texture()
    {
        if (!c_fn || !c_fn->mat_get_texture)
            return nullptr;
        return c_fn->mat_get_texture(this);
    }

    void set_texture(uintptr_t tex)
    {
        if (!c_fn || !c_fn->mat_set_texture)
            return;
        c_fn->mat_set_texture(this, (void *)tex);
    }

    void *get_shader()
    {
        if (!c_fn || !c_fn->mat_get_shader)
            return nullptr;
        return c_fn->mat_get_shader(this);
    }

    void set_shader(void *s)
    {
        if (!c_fn || !c_fn->mat_set_shader)
            return;
        c_fn->mat_set_shader(this, s);
    }

    void new_material(c_shader *s)
    {
        if (!c_fn || !c_fn->mat_ctor_shader)
            return;
        c_fn->mat_ctor_shader(this, s);
    }

    void set_color(color_t c)
    {
        if (!c_fn || !c_fn->mat_set_color)
            return;
        c_fn->mat_set_color(this, c);
    }

    void set_int(monoString *name, int val)
    {
        if (!c_fn || !c_fn->mat_set_int)
            return;
        c_fn->mat_set_int(this, name, val);
    }

    void set_float(monoString *name, float val)
    {
        if (!c_fn || !c_fn->mat_set_float)
            return;
        c_fn->mat_set_float(this, name, val);
    }
};
#pragma pack()
