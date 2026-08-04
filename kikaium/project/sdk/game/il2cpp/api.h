#pragma once
#include "il2cpp.h"
#include "globals.hpp"

inline void *(*il2cpp_domain_get)();
inline void *(*il2cpp_domain_assembly_open)(void *, const char *);
inline void *(*il2cpp_assembly_get_image)(void *);
inline void *(*il2cpp_object_new)(void *);
inline void *(*il2cpp_class_from_name)(void *, const char *, const char *);
void *clazz_unity(const char *a, const char *b);
void *clazz_def(const char *a, const char *b);

namespace dll
{
    inline void *charp;
    inline void *unity;
}
MethodInfo *GetMethodFromClass(Il2CppClass *clz, const char *name, uint8_t parameters_count);
void init();
