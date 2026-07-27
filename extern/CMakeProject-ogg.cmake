set(OGG_SRC "ogg/src/bitwise.c"
            "ogg/src/framing.c")

set(OGG_HPP "ogg/ogg/config_types.h"
            "ogg/include/ogg/ogg.h"
            "ogg/include/ogg/os_types.h")

source_group("Source Files" FILES ${OGG_SRC})
source_group("Header Files" FILES ${OGG_HPP})

add_library("ogg" STATIC ${OGG_SRC} ${OGG_HPP})

set_property(TARGET "ogg" PROPERTY FOLDER "External Libraries")

disable_project_warnings("ogg")

target_include_directories("ogg" PUBLIC
  "ogg/include"
  "${CMAKE_CURRENT_BINARY_DIR}/ogg"
)

configure_file("config.ogg.types.in.h" "ogg/ogg/config_types.h")
