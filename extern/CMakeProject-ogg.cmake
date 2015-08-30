set(OGG_DIR "libogg-1.3.2")

set(OGG_SRC
  "${OGG_DIR}/src/bitwise.c"
  "${OGG_DIR}/src/framing.c"
)

set(OGG_HPP
  "${OGG_DIR}/include/ogg/ogg.h"
  "${OGG_DIR}/include/ogg/os_types.h"
)

source_group("" FILES ${OGG_SRC} ${OGG_HPP})

add_library("ogg" ${OGG_SRC} ${OGG_HPP})

set_property(TARGET "ogg" PROPERTY FOLDER "External Libraries")

disable_project_warnings("ogg")

target_include_directories("ogg" PUBLIC "${OGG_DIR}/include")
