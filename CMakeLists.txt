cmake_minimum_required(VERSION 2.8)
project(StepMania)

# Put predefined targets in separate group.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Do setup.
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include(FindDirectX)

add_subdirectory(extern)
add_subdirectory(src)