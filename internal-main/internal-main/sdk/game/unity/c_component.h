#pragma once
class c_transform;
#pragma pack(1)
class c_component 
{
    public:
    c_transform *get_transform(void *c)
    {
        using a = c_transform*(*)(void*css);
        a b = reinterpret_cast<a>(base + c_offsets->get_transform);
        return b(c);
    }

    void *get_game_object()
    {
        using a = void*(*)(c_component *a);
        a b = reinterpret_cast<a>(base + c_offsets->get_game_object);
        return b(this);
    }

    // void set_enabled(c_transform *c)
    // {
    //     using a = void(*)(c_transform*css, bool);
    //     a b = reinterpret_cast<a>(base + c_offsets->set_enabled);
    //     return b(c, true);
    // }
    //
    // monoString *get_tag()
    // {
    //     using a = monoString*(*)(c_component *a);
    //     a b = reinterpret_cast<a>(base + c_offsets->get_tag);
    //     return b(this);
    // }
};
#pragma pack()