set(OGG_DIR "${SM_EXTERN_DIR}/newogg")

list(APPEND OGG_SRC "${OGG_DIR}/src/bitwise.c" "${OGG_DIR}/src/framing.c")

list(APPEND OGG_HPP
            "${OGG_DIR}/include/ogg/config_types.h"
            "${OGG_DIR}/include/ogg/ogg.h"
            "${OGG_DIR}/include/ogg/os_types.h")

source_group("Source Files" FILES ${OGG_SRC})
source_group("Header Files" FILES ${OGG_HPP})

add_library("ogg" STATIC ${OGG_SRC} ${OGG_HPP})

set_property(TARGET "ogg" PROPERTY FOLDER "External Libraries")

disable_project_warnings("ogg")

target_include_directories("ogg" PUBLIC "${OGG_DIR}/include")

configure_file("${SM_EXTERN_DIR}/config.ogg.types.in.h"
               "${OGG_DIR}/include/ogg/config_types.h")
