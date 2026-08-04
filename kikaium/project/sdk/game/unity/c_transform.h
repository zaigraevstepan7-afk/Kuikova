#pragma once
#pragma pack(1)
class c_transform
{
public:
    Vector3 get_position()
    {
        if (!this || !c_fn || !c_fn->get_position)
            return Vector3{};
        return c_fn->get_position(this);
    }

    void set_position(Vector3 pos)
    {
        if (!this || !c_fn || !c_fn->set_position)
            return;
        c_fn->set_position(this, pos);
    }

    Vector3 get_forward()
    {
        if (!this || !c_fn || !c_fn->get_forward)
            return {};
        return c_fn->get_forward(this);
    }

    Quaternion get_rotation()
    {
        if (!this || !c_fn || !c_fn->get_rotation)
            return {};
        return c_fn->get_rotation(this);
    }

    Vector3 get_up()
    {
        if (!this || !c_fn || !c_fn->get_up)
            return {};
        return c_fn->get_up(this);
    }

    Vector3 get_euler_angles()
    {
        if (!this || !c_fn || !c_fn->get_euler_angles)
            return {};
        return c_fn->get_euler_angles(this);
    }

    void set_euler_angles(euler_angles_t pos)
    {
        if (!this || !c_fn || !c_fn->set_euler_angles)
            return;
        c_fn->set_euler_angles(this, pos);
    }
};
#pragma pack()
