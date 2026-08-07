cmake_minimum_required(VERSION 3.22)
project(nova LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
set(DOBBY_LIB ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libdobby.a)

add_library(nova SHARED
    src/main.cpp
    src/game.cpp
    src/overlay.cpp
    src/chams.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(nova PRIVATE
    src
    third_party
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_compile_definitions(nova PRIVATE
    IMGUI_IMPL_OPENGL_ES3
    ANDROID
)

target_compile_options(nova PRIVATE
    -fvisibility=hidden
    -ffunction-sections
    -fdata-sections
    -O2
    -Wall
)

target_link_options(nova PRIVATE
    -Wl,--gc-sections
    -Wl,-z,max-page-size=16384
)

target_link_libraries(nova
    ${DOBBY_LIB}
    android
    log
    EGL
    GLESv3
    dl
)

set_target_properties(nova PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    OUTPUT_NAME "nova"
    PREFIX "lib"
    SUFFIX ".so"
)
