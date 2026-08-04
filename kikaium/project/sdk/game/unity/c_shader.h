#pragma once

#pragma pack(1)
class c_shader
{
public:
    c_shader *find(monoString *name)
    {
        if (!c_fn || !c_fn->shader_find)
            return nullptr;
        return (c_shader *)c_fn->shader_find(name);
    }
};
#pragma pack()
