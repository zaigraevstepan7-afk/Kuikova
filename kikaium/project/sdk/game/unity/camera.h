#pragma once

#pragma pack(1)
class c_camera
{
public:
    void *get_main()
    {
        if (!c_fn || !c_fn->camera_get_main)
            return nullptr;
        return c_fn->camera_get_main();
    }

    void set_fov(float c)
    {
        // Halalium UnityMethod::set_fov (libunity)
        if (!c_fn || !c_fn->set_fov)
            return;
        c_fn->set_fov((void *)this, c);
    }

    Matrix get_projection_matrix_injected()
    {
        Matrix ret{};
        if (!c_fn || !c_fn->get_proj_injected)
            return ret;
        c_fn->get_proj_injected((void *)this, &ret);
        return ret;
    }

    Matrix get_world_to_camera_matrix_injected()
    {
        Matrix ret{};
        if (!c_fn || !c_fn->get_w2c_injected)
            return ret;
        c_fn->get_w2c_injected((void *)this, &ret);
        return ret;
    }
};
#pragma pack()
