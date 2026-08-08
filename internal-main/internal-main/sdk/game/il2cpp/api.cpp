#pragma once
#include "api.h"
#include <unistd.h>
#include <thread>

void *img_to_asm(const char *assembly)
{
    void *domain = il2cpp_domain_get();
    void *domain_asm = il2cpp_domain_assembly_open(domain, assembly);
    void *asm_img = il2cpp_assembly_get_image(domain_asm);
    return asm_img;
}

MethodInfo *GetMethodFromClass(Il2CppClass *clz, const char *name, uint8_t parameters_count)
{
    for (unsigned short i = 0; i < clz->method_count; i++)
    {
        auto method = ((MethodInfo **)clz->methods)[i];
        if (method && (strcmp(method->name, name) == 0 && method->parameters_count == parameters_count))
            return method;
    }
    return nullptr;
}

void *(clazz_unity)(const char *a, const char *b)
{
    return il2cpp_class_from_name(dll::unity, a, b);
}

void *(clazz_def)(const char *a, const char *b)
{
    return il2cpp_class_from_name(dll::charp, a, b);
}

void init()
{
    il2cpp_domain_get = (decltype(il2cpp_domain_get))(base + c_offsets->il2cpp_domain_get);
    il2cpp_domain_assembly_open = (decltype(il2cpp_domain_assembly_open))(base + c_offsets->il2cpp_domain_assembly_open);
    il2cpp_assembly_get_image = (decltype(il2cpp_assembly_get_image))(base + c_offsets->il2cpp_assembly_get_image);
    il2cpp_class_from_name = (decltype(il2cpp_class_from_name))(base + c_offsets->il2cpp_class_from_name);
    il2cpp_object_new = (decltype(il2cpp_object_new))(base + c_offsets->il2cpp_object_new);
    dll::charp = img_to_asm(oxorany("Assembly-CSharp"));
    dll::unity = img_to_asm(oxorany("UnityEngine.CoreModule"));
}