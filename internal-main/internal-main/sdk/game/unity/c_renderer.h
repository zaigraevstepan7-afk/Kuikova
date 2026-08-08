#pragma once
class c_material;
#pragma pack(1)
class c_renderer
{
public:
    void *get_material()
    {
        using a = void *(*)(c_renderer *);
        a b = reinterpret_cast<a>(base + c_offsets->get_material);
        return b(this);
    }

    void set_material(c_material *tex)
    {
        using a = void (*)(c_renderer *, c_material *poss);
        a b = reinterpret_cast<a>(base + c_offsets->set_material);
        return b(this, tex);
    }

    monoArray<c_material *> *get_materials()
    {
        using a = monoArray<c_material *> *(*)(c_renderer *);
        a b = reinterpret_cast<a>(base + c_offsets->get_materials);
        return b(this);
    }

    void set_materials(monoArray<c_material *> *tex)
    {
        using a = void (*)(c_renderer *, monoArray<c_material *> *poss);
        a b = reinterpret_cast<a>(base + c_offsets->set_materials);
        return b(this, tex);
    }
};
#pragma pack()