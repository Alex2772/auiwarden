# Standard routine
cmake_minimum_required(VERSION 3.16)
project(auiwarden
        VERSION 0.0.12 # CI_PROJECT_VERSION
    )

option(BUILD_SHARED_LIBS "Build shared libs" OFF)
option(AUIB_FORCE_PRECOMPILED "AUI.Boot: force precompiled dependencies" OFF)
option(AUIB_NO_PRECOMPILED "AUI.Boot: disable precompiled dependencies" OFF)

set(AUI_VERSION bf86b65e21a4a19ac1e03806f482ae436f9c84a2)

# Use AUI.Boot
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

# import AUI
auib_import(aui https://github.com/aui-framework/aui
        COMPONENTS core views curl json crypt updater
        VERSION ${AUI_VERSION})


if(NOT AUI_PLATFORM_LINUX)
    message(FATAL_ERROR "This software is designed specifically for Linux.")
endif ()


# Create the executable. This function automatically links all sources from the src/ folder,
# creates CMake target and places the resulting executable to bin/ folder.
aui_executable(${PROJECT_NAME})

# Link required libs
aui_link(${PROJECT_NAME} PRIVATE aui::core aui::views aui::updater)

# Compile assets
aui_compile_assets(${PROJECT_NAME})

# Enable testing
aui_enable_tests(${PROJECT_NAME})

# Setup icon, display name, etc
aui_app(TARGET ${PROJECT_NAME}
        NAME "AUI App Example"
        VENDOR "AUI Project"
        ICON "assets/img/icon.svg"
)

if(AUI_PLATFORM_LINUX)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/wayland-generated-protocols)
    file(GLOB_RECURSE WAYLAND_PROTOCOLS wayland-protocols/*.xml)
    foreach (WAYLAND_PROTOCOL_PATH ${WAYLAND_PROTOCOLS})
        get_filename_component(WAYLAND_PROTOCOL_NAME ${WAYLAND_PROTOCOL_PATH} NAME)
        set(OUTPUT_NAME ${CMAKE_CURRENT_BINARY_DIR}/include/wayland-generated/protocols/${WAYLAND_PROTOCOL_NAME})
        add_custom_command(OUTPUT ${OUTPUT_NAME}.h
                COMMAND wayland-scanner client-header ${WAYLAND_PROTOCOL_PATH} ${OUTPUT_NAME}.h)
        add_custom_command(OUTPUT ${OUTPUT_NAME}.c
                COMMAND wayland-scanner private-code ${WAYLAND_PROTOCOL_PATH} ${OUTPUT_NAME}.c)
        target_sources(${PROJECT_NAME} PRIVATE ${OUTPUT_NAME}.h ${OUTPUT_NAME}.c)
    endforeach ()
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE wayland-client)
endif()
