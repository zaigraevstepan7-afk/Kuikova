cmake_minimum_required(VERSION 3.22)
project(kikaium)

set(CMAKE_CXX_STANDARD 17)

# Kikaium — Halalium-architecture cheat (Melodium-style sources + Halalium RE offsets)
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

add_library(kikaium SHARED ${TO_COMPILE})

add_compile_definitions(BIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD KIKAIUM=1)

target_include_directories(kikaium PUBLIC includes/imgui)
target_include_directories(kikaium PUBLIC includes/imgui/backends)
target_include_directories(kikaium PUBLIC .)

set_target_properties(kikaium PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "kikaium"
    PREFIX "lib"
)

target_link_libraries(kikaium
        z
        log
        android
        EGL
        GLESv3
        OpenSLES
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(kikaium PRIVATE
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

    set_target_properties(kikaium PROPERTIES LINK_FLAGS
            "-Wl,--gc-sections -Wl,--strip-all -Wl,--exclude-libs,ALL"
    )

    add_custom_command(
            TARGET kikaium
            POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:kikaium>
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/../bin"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kikaium> "${CMAKE_SOURCE_DIR}/../bin/libkikaium.so"
            COMMENT "Install libkikaium.so -> kikaium/bin/"
    )
endif()
