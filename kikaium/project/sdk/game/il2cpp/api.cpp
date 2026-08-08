#pragma once
#include "api.h"
#include <unistd.h>
#include <thread>
#include <dlfcn.h>
#include <android/log.h>

void *img_to_asm(const char *assembly)
{
    void *domain = il2cpp_domain_get();
    void *domain_asm = il2cpp_domain_assembly_open(domain, assembly);
    void *asm_img = il2cpp_assembly_get_image(domain_asm);
    return asm_img;
}

MethodInfo *GetMethodFromClass(Il2CppClass *clz, const char *name, uint8_t parameters_count)
{
    if (!clz || !clz->methods)
        return nullptr;
    for (unsigned short i = 0; i < clz->method_count; i++)
    {
        auto method = ((MethodInfo **)clz->methods)[i];
        if (method && method->name && (strcmp(method->name, name) == 0 && method->parameters_count == parameters_count))
            return method;
    }
    return nullptr;
}

void *clazz_unity(const char *a, const char *b)
{
    return il2cpp_class_from_name(dll::unity, a, b);
}

void *clazz_def(const char *a, const char *b)
{
    return il2cpp_class_from_name(dll::charp, a, b);
}

static void *dlsym_il2cpp(const char *sym)
{
    static void *h = nullptr;
    if (!h)
        h = dlopen("libil2cpp.so", RTLD_NOW);
    if (!h)
        return nullptr;
    return dlsym(h, sym);
}

void init()
{
    // Prefer exported symbols — no Melodium/community Api RVAs.
    il2cpp_domain_get = (decltype(il2cpp_domain_get))dlsym_il2cpp("il2cpp_domain_get");
    il2cpp_domain_assembly_open = (decltype(il2cpp_domain_assembly_open))dlsym_il2cpp("il2cpp_domain_assembly_open");
    il2cpp_assembly_get_image = (decltype(il2cpp_assembly_get_image))dlsym_il2cpp("il2cpp_assembly_get_image");
    il2cpp_class_from_name = (decltype(il2cpp_class_from_name))dlsym_il2cpp("il2cpp_class_from_name");
    il2cpp_object_new = (decltype(il2cpp_object_new))dlsym_il2cpp("il2cpp_object_new");

    if (!il2cpp_domain_get || !il2cpp_domain_assembly_open || !il2cpp_assembly_get_image || !il2cpp_class_from_name)
    {
        __android_log_print(ANDROID_LOG_ERROR, "kikaium",
                            "il2cpp API dlsym failed — cannot bootstrap without non-Halalium RVAs");
        return;
    }

    dll::charp = img_to_asm(oxorany("Assembly-CSharp"));
    dll::unity = img_to_asm(oxorany("UnityEngine.CoreModule"));
}
