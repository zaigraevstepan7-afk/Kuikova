#pragma once

#pragma pack(1)
class c_shader {
    public:
    c_shader *find(monoString *name)
    {
        using a = c_shader *(*)(monoString *sdfds);
        a b = reinterpret_cast<a>(base + c_offsets->find);
        return b(name);
    }
};
#pragma pack()