#pragma once
class c_renderer;
#pragma pack(1)
class c_type
{
    public:
    c_type* get_type(monoString* asdad)
    {
        using a = c_type*(*)(monoString* c);
        a b = reinterpret_cast<a>(base + c_offsets->get_type);
        return b(asdad);
    }

    monoArray<c_renderer *> * find_objects_of_type()
    {
        using a = monoArray<c_renderer *> *(*)(c_type* c);
        a b = reinterpret_cast<a>(base + c_offsets->find_objects_of_type);
        return b(this);
    }

};
#pragma pack()