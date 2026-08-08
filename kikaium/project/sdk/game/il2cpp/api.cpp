#pragma once
#include "api.h"
#include <unistd.h>
#include <dlfcn.h>
#include <android/log.h>
#include "includes/module_base.h"
#include "includes/halalium_hooks.h"
#include "sdk/OffsetsBridge.h"

void *img_to_asm(const char *assembly)
{
    if (!il2cpp_domain_get || !il2cpp_domain_assembly_open || !il2cpp_assembly_get_image)
        return nullptr;
    void *domain = il2cpp_domain_get();
    if (!domain)
        return nullptr;
    void *domain_asm = il2cpp_domain_assembly_open(domain, assembly);
    if (!domain_asm)
        return nullptr;
    return il2cpp_assembly_get_image(domain_asm);
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
    if (!il2cpp_class_from_name || !dll::unity)
        return nullptr;
    return il2cpp_class_from_name(dll::unity, a, b);
}

void *clazz_def(const char *a, const char *b)
{
    if (!il2cpp_class_from_name || !dll::charp)
        return nullptr;
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

using il2cpp_thread_attach_fn = void *(*)(void *);
static il2cpp_thread_attach_fn g_thread_attach = nullptr;

static void bind_api_rva(uintptr_t il2cpp_base)
{
    if (!il2cpp_base)
        return;
    if (!il2cpp_domain_get)
    {
        void *p = (void *)(il2cpp_base + Offsets::Api::il2cpp_domain_get);
        if (hhooks::looks_like_a64(p))
            il2cpp_domain_get = (decltype(il2cpp_domain_get))p;
    }
    if (!il2cpp_domain_assembly_open)
    {
        void *p = (void *)(il2cpp_base + Offsets::Api::il2cpp_domain_assembly_open);
        if (hhooks::looks_like_a64(p))
            il2cpp_domain_assembly_open = (decltype(il2cpp_domain_assembly_open))p;
    }
    if (!il2cpp_assembly_get_image)
    {
        void *p = (void *)(il2cpp_base + Offsets::Api::il2cpp_assembly_get_image);
        if (hhooks::looks_like_a64(p))
            il2cpp_assembly_get_image = (decltype(il2cpp_assembly_get_image))p;
    }
    if (!il2cpp_class_from_name)
    {
        void *p = (void *)(il2cpp_base + Offsets::Api::il2cpp_class_from_name);
        if (hhooks::looks_like_a64(p))
            il2cpp_class_from_name = (decltype(il2cpp_class_from_name))p;
    }
    if (!il2cpp_object_new)
    {
        void *p = (void *)(il2cpp_base + Offsets::Api::il2cpp_object_new);
        if (hhooks::looks_like_a64(p))
            il2cpp_object_new = (decltype(il2cpp_object_new))p;
    }
}

void init()
{
    if (!il2cpp_domain_get)
        il2cpp_domain_get = (decltype(il2cpp_domain_get))dlsym_il2cpp("il2cpp_domain_get");
    if (!il2cpp_domain_assembly_open)
        il2cpp_domain_assembly_open = (decltype(il2cpp_domain_assembly_open))dlsym_il2cpp("il2cpp_domain_assembly_open");
    if (!il2cpp_assembly_get_image)
        il2cpp_assembly_get_image = (decltype(il2cpp_assembly_get_image))dlsym_il2cpp("il2cpp_assembly_get_image");
    if (!il2cpp_class_from_name)
        il2cpp_class_from_name = (decltype(il2cpp_class_from_name))dlsym_il2cpp("il2cpp_class_from_name");
    if (!il2cpp_object_new)
        il2cpp_object_new = (decltype(il2cpp_object_new))dlsym_il2cpp("il2cpp_object_new");
    if (!g_thread_attach)
        g_thread_attach = (il2cpp_thread_attach_fn)dlsym_il2cpp("il2cpp_thread_attach");

    uintptr_t il2cpp = resolve_il2cpp_base();
    if (!il2cpp && base)
        il2cpp = base;
    bind_api_rva(il2cpp);

    if (!il2cpp_domain_get || !il2cpp_domain_assembly_open || !il2cpp_assembly_get_image || !il2cpp_class_from_name)
    {
        __android_log_print(ANDROID_LOG_WARN, "xxx",
                            "il2cpp API incomplete (optional) base=%p — ESP uses TypeInfo+RVA",
                            (void *)il2cpp);
        return;
    }

    if (g_thread_attach)
    {
        void *domain = il2cpp_domain_get();
        if (domain)
            g_thread_attach(domain);
    }

    for (int i = 0; i < 10; ++i)
    {
        if (!dll::charp)
            dll::charp = img_to_asm("Assembly-CSharp");
        if (!dll::unity)
            dll::unity = img_to_asm("UnityEngine.CoreModule");
        if (dll::charp && dll::unity)
            break;
        usleep(200000);
    }

    __android_log_print(ANDROID_LOG_INFO, "xxx",
                        "il2cpp init api=%p asm=%p unity=%p",
                        (void *)il2cpp_domain_get, dll::charp, dll::unity);
}
