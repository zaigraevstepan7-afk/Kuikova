#pragma once
class c_transform;
#pragma pack(1)
class c_component
{
public:
    c_transform *get_transform(void *c)
    {
        if (!c_fn || !c_fn->get_transform)
            return nullptr;
        return (c_transform *)c_fn->get_transform(c);
    }

    void *get_game_object()
    {
        // Halalium UnityMethod::get_game_object
        if (!c_fn || !c_fn->get_game_object)
            return nullptr;
        return c_fn->get_game_object(this);
    }
};
#pragma pack()
