# Android.mk

LOCAL_PATH := $(call my-dir)

# ---------------- payload ----------------
include $(CLEAR_VARS)

LOCAL_MODULE := xxxstux

LOCAL_SRC_FILES := \
    ../src/main.cpp \
    ../includes/imgui/imgui.cpp \
    ../includes/imgui/imgui_draw.cpp \
    ../includes/imgui/imgui_tables.cpp \
    ../includes/imgui/imgui_widgets.cpp \
    ../includes/imgui/backends/imgui_impl_opengl3.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../includes/imgui \
    $(LOCAL_PATH)/../includes/imgui/backends \
    $(LOCAL_PATH)/../src

LOCAL_CPPFLAGS := \
    -std=c++17 \
    -fno-rtti \
    -fno-exceptions \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden \
    -fno-unwind-tables \
    -fno-asynchronous-unwind-tables \
    -fomit-frame-pointer \
    -ffunction-sections \
    -fdata-sections \
    -fmerge-all-constants \
    -fno-ident \
    -Oz \
    -g

LOCAL_LDFLAGS := \
    -Wl,--gc-sections \
    -Wl,--build-id=none \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,-z,noexecstack \
    -Wl,--export-dynamic-symbol=JNI_OnLoad \
    -Wl,--export-dynamic-symbol=payload_entry \
    -Wl,--export-dynamic-symbol=EntryPoint \
    -Wl,--export-dynamic-symbol=xxxstux_entry

# Keep .symtab for custom inj + Kitty findSymbol; never strip entry exports.
LOCAL_STRIP_MODE := none

LOCAL_LDLIBS := \
    -llog \
    -landroid \
    -lEGL \
    -lGLESv3 \
    -lm

include $(BUILD_SHARED_LIBRARY)

# ---------------- injector ----------------
include $(CLEAR_VARS)

LOCAL_MODULE := inj

LOCAL_SRC_FILES := \
    ../inj/main.cpp

LOCAL_CPPFLAGS := \
    -std=c++17 \
    -fno-rtti \
    -fno-exceptions \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden \
    -fomit-frame-pointer \
    -ffunction-sections \
    -fdata-sections \
    -fno-ident \
    -Oz

LOCAL_LDFLAGS := \
    -Wl,--gc-sections \
    -Wl,--strip-all \
    -Wl,--build-id=none \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,-z,noexecstack \
    -pie

LOCAL_LDLIBS := \
    -llog

include $(BUILD_EXECUTABLE)
