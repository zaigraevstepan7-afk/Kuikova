#pragma once

#pragma pack(1)
class c_camera
{
public:
    void *get_main()
    {
        using a = void *(*)(); 
        a b = reinterpret_cast<a>(base + c_offsets->get_main);
        return b(); 
    }

    void set_fov(float c)
    {
        using a = void(*)(void*, float afd);
        a b = reinterpret_cast<a>(base + c_offsets->set_fov);
        return b((void*)this, c);
    }

    Matrix get_projection_matrix_injected() {
        Matrix ret{};
        using a = void(*)(c_camera*, Matrix&);
        a b = reinterpret_cast<a>(base + c_offsets->get_projection_matrix_injected);
        b(this, ret);
        return ret;
    }

    Matrix get_world_to_camera_matrix_injected() {
        Matrix ret{};
        using a = void(*)(c_camera*, Matrix&);
        a b = reinterpret_cast<a>(base + c_offsets->fsdfds);
        b(this, ret);
        return ret;
    }
};
#pragma pack()