#pragma once

#pragma pack(1)
class c_shader;
class c_material
{
public:
    void *get_texture()
    {
        using a = void *(*)(c_material *);
        a b = reinterpret_cast<a>(base + c_offsets->get_texture);
        return b(this);
    }

    void set_texture(uintptr_t tex)
    {
        using a = void (*)(c_material *, uintptr_t poss);
        a b = reinterpret_cast<a>(base + c_offsets->set_texture);
        return b(this, tex);
    }

    void *get_shader()
    {
        using a = void *(*)(c_material *);
        a b = reinterpret_cast<a>(base + c_offsets->get_shader);
        return b(this);
    }

    void set_shader(void *s)
    {
        using a = void (*)(c_material *, void *poss);
        a b = reinterpret_cast<a>(base + c_offsets->set_shader);
        return b(this, s);
    }

    void new_material(c_shader *s)
    {
        using a = void (*)(c_material *, c_shader *poss);
        a b = reinterpret_cast<a>(base + c_offsets->new_material);
        return b(this, s);
    }

    void set_color(color_t c)
    {
        using a = void (*)(c_material *, color_t poss);
        a b = reinterpret_cast<a>(base + c_offsets->set_color);
        return b(this, c);
    }

    void set_int(monoString *name, int val)
    {
        using a = void (*)(c_material *, monoString *sdfds, int asssd);
        a b = reinterpret_cast<a>(base + c_offsets->set_int);
        return b(this, name, val);
    }

    void set_float(monoString *name, float val)
    {
        using a = void (*)(c_material *, monoString *sdfds, float asssd);
        a b = reinterpret_cast<a>(base + c_offsets->set_float);
        return b(this, name, val);
    }
};
#pragma pack()