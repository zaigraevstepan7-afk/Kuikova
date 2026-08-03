#pragma once
#pragma pack(1)
class c_transform
{
public:
    Vector3 get_position()
    {
        if (!this || !c_offsets)
            return Vector3{};

        uintptr_t addr = base + c_offsets->get_position;
        if (!addr)
            return Vector3{};

        using fn = Vector3 (*)(c_transform *);
        fn f = reinterpret_cast<fn>(addr);
        if (!f)
            return Vector3{};

        return f(this);
    }

    void set_position(Vector3 pos)
    {
        if (!c_offsets)
            return;
        uintptr_t addr = base + c_offsets->set_position;
        using fn = void (*)(c_transform *, Vector3);
        fn f = reinterpret_cast<fn>(addr);
        if (f)
            f(this, pos);
    }

    Vector3 get_forward()
    {
        if (!c_offsets)
            return {};
        uintptr_t addr = base + c_offsets->get_forward;
        using fn = Vector3 (*)(c_transform *);
        fn f = reinterpret_cast<fn>(addr);
        return f ? f(this) : Vector3{};
    }

    Quaternion get_rotation()
    {
        if (!c_offsets)
            return {};
        uintptr_t addr = base + c_offsets->get_rotation;
        using fn = Quaternion (*)(c_transform *);
        fn f = reinterpret_cast<fn>(addr);
        return f ? f(this) : Quaternion{};
    }

    Vector3 get_up()
    {
        if (!c_offsets)
            return {};
        uintptr_t addr = base + c_offsets->get_up;
        using fn = Vector3 (*)(c_transform *);
        fn f = reinterpret_cast<fn>(addr);
        return f ? f(this) : Vector3{};
    }

    Vector3 get_euler_angles()
    {
        if (!c_offsets)
            return {};
        uintptr_t addr = base + c_offsets->get_euler_angles;
        using fn = Vector3 (*)(c_transform *);
        fn f = reinterpret_cast<fn>(addr);
        return f ? f(this) : Vector3{};
    }

    void set_euler_angles(euler_angles_t pos)
    {
        if (!c_offsets)
            return;
        uintptr_t addr = base + c_offsets->set_euler_angles;
        using fn = void (*)(c_transform *, euler_angles_t);
        fn f = reinterpret_cast<fn>(addr);
        if (f)
            f(this, pos);
    }
};
#pragma pack()