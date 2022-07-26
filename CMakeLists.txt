cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0076 NEW)

project(follia VERSION 0.1 DESCRIPTION "game framework")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(follia SHARED
    src/core/fl_log.c
    src/core/fl_primitives.c
    src/core/fl_system.c
    src/core/fl_timer.c
    src/core/fl_utils.c
    src/video/fl_font.c
    src/video/fl_texture.c
)

target_include_directories(follia PUBLIC include)
target_link_libraries(follia PRIVATE -lm -lX11 -lXext)

add_subdirectory(examples)
add_subdirectory(tests)
