cmake_minimum_required(VERSION 3.10)
project(my_app LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)

# Provide our own fork of imgui, disable the one provided by hello_imgui
set (HELLOIMGUI_BUILD_IMGUI OFF CACHE BOOL "" FORCE)
set(imgui_dir ${CMAKE_CURRENT_LIST_DIR}/external/imgui)
add_library(imgui
    ${imgui_dir}/imgui_demo.cpp
    ${imgui_dir}/imgui_draw.cpp
    ${imgui_dir}/imgui_internal.h
    ${imgui_dir}/imgui_widgets.cpp
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui.h
    )
target_include_directories(imgui PUBLIC ${imgui_dir})

option(IMGUI_HELLODEMO_CAN_WRITE_IMGUI_DEMO_CPP OFF)

set(HELLOIMGUI_USE_SDL_OPENGL3 ON CACHE STRING "" FORCE)
add_subdirectory(external/hello_imgui)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${HELLOIMGUI_BASEPATH}/hello_imgui_cmake)
if (EMSCRIPTEN)
    include(${HELLOIMGUI_BASEPATH}/hello_imgui_cmake/emscripten/hello_imgui_emscripten_global_options.cmake)
endif()
add_subdirectory(src)
