cmake_minimum_required(VERSION 3.22)
project(zygisk_module)

set(CMAKE_CXX_STANDARD 17)

set(TO_COMPILE
    main.cpp
    includes/imgui/imgui_demo.cpp
    includes/imgui/imgui_draw.cpp
    includes/imgui/imgui_tables.cpp
    includes/imgui/imgui_widgets.cpp
    includes/imgui/imgui.cpp
    includes/imgui/backends/imgui_impl_android.cpp
    includes/imgui/backends/imgui_impl_opengl3.cpp
    includes/oxorany/Oxorany.cpp
    includes/egl/maps.cpp
    includes/egl/memory.cpp
    globals.cpp
    config.cpp
    src/features/update.cpp
    src/features/esp.cpp
    src/features/misc.cpp
    src/features/visual.cpp
    src/features/antiaim.cpp
    src/features/world.cpp
    src/features/chams.cpp
    src/features/exploits.cpp
    src/menu/gui.cpp
    src/menu/c_user_interface.cpp
    sdk/game/unity/photon/c_photon_player.cpp
    sdk/game/il2cpp/api.cpp
)

add_library(zygisk_module SHARED ${TO_COMPILE})

add_compile_definitions(BIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD)

set(FRIDA_GUM_DIR "${CMAKE_SOURCE_DIR}/includes")

target_include_directories(zygisk_module PUBLIC includes/imgui)
target_include_directories(zygisk_module PUBLIC includes/imgui/backends)
target_include_directories(zygisk_module PUBLIC .)

set_target_properties(zygisk_module PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(zygisk_module
        z
        log
        android
        EGL
        GLESv3
        OpenSLES
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(zygisk_module PRIVATE
            -O2
            -fPIC
            -fno-rtti
            -fno-exceptions
            -fdata-sections
            -ffunction-sections
            -fvisibility=hidden
            -fvisibility-inlines-hidden
            -fno-unroll-loops
            -w
    )

    set_source_files_properties(globals.cpp PROPERTIES COMPILE_FLAGS "-O2")

    set_target_properties(zygisk_module PROPERTIES LINK_FLAGS
            "-Wl,--gc-sections -Wl,--strip-all -Wl,--exclude-libs,ALL"
    )

    add_custom_command(
            TARGET zygisk_module
            POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:zygisk_module>
    )
endif()