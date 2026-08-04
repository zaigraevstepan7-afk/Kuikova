#pragma once
class c_material;
#pragma pack(1)
class c_renderer
{
public:
    void *get_material()
    {
        if (!c_fn || !c_fn->renderer_get_material)
            return nullptr;
        return c_fn->renderer_get_material(this);
    }

    void set_material(c_material *tex)
    {
        if (!c_fn || !c_fn->renderer_set_material)
            return;
        c_fn->renderer_set_material(this, tex);
    }

    monoArray<c_material *> *get_materials()
    {
        if (!c_fn || !c_fn->renderer_get_materials)
            return nullptr;
        return (monoArray<c_material *> *)c_fn->renderer_get_materials(this);
    }

    void set_materials(monoArray<c_material *> *tex)
    {
        if (!c_fn || !c_fn->renderer_set_materials)
            return;
        c_fn->renderer_set_materials(this, tex);
    }
};
#pragma pack()
