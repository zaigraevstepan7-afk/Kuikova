cmake_minimum_required(VERSION 3.22)
project(xxx)

set(CMAKE_CXX_STANDARD 17)

# xxx — new Standoff 2 0.39.2 internal (menu + ESP + bypass)
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
    src/features/skins.cpp
    src/menu/gui.cpp
    src/menu/c_user_interface.cpp
    sdk/game/unity/photon/c_photon_player.cpp
    sdk/game/il2cpp/api.cpp
)

add_library(xxx SHARED ${TO_COMPILE})

add_compile_definitions(BIONIC_IOCTL_NO_SIGNEDNESS_OVERLOAD KIKAIUM=1 XXX=1)

target_include_directories(xxx PUBLIC includes/imgui)
target_include_directories(xxx PUBLIC includes/imgui/backends)
target_include_directories(xxx PUBLIC .)

set_target_properties(xxx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "xxx"
    PREFIX "lib"
)

target_link_libraries(xxx
        z
        log
        android
        EGL
        GLESv3
        OpenSLES
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(xxx PRIVATE
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

    set_target_properties(xxx PROPERTIES LINK_FLAGS
            "-Wl,--gc-sections -Wl,--strip-all -Wl,--exclude-libs,ALL"
    )

    add_custom_command(
            TARGET xxx
            POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:xxx>
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/../bin"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:xxx> "${CMAKE_SOURCE_DIR}/../bin/libxxx.so"
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:xxx> "${CMAKE_SOURCE_DIR}/../../xxx/bin/libxxx.so"
            COMMENT "Install libxxx.so -> kikaium/bin + xxx/bin"
    )
endif()
