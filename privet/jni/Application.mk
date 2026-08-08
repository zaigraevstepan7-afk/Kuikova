# Application.mk

APP_ABI := arm64-v8a

APP_STRIP_MODE := none

APP_PLATFORM := android-23

APP_STL := c++_static

APP_CPPFLAGS := \
    -std=c++17 \
    -fno-rtti \
    -fno-exceptions \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden \
    -fno-unwind-tables \
    -fno-asynchronous-unwind-tables \
    -Oz

APP_LDFLAGS := \
    -Wl,--gc-sections \
    -Wl,--build-id=none \
    -Wl,--hash-style=gnu \
    -Wl,--no-undefined \
    -Wl,-z,relro \
    -Wl,-z,now \
    -Wl,-z,noexecstack
