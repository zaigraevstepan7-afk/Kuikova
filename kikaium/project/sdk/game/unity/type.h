#pragma once
class c_renderer;
#pragma pack(1)
class c_type
{
public:
    c_type *get_type(monoString *asdad)
    {
        if (!c_fn || !c_fn->type_get_type)
            return nullptr;
        return (c_type *)c_fn->type_get_type(asdad);
    }

    monoArray<c_renderer *> *find_objects_of_type()
    {
        if (!c_fn || !c_fn->find_objects_of_type)
            return nullptr;
        return (monoArray<c_renderer *> *)c_fn->find_objects_of_type(this);
    }
};
#pragma pack()
