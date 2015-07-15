list(APPEND OGG_SRC
  "libogg-1.3.2/src/bitwise.c"
  "libogg-1.3.2/src/framing.c"
)

list(APPEND OGG_HPP
  "libogg-1.3.2/include/ogg/ogg.h"
  "libogg-1.3.2/include/ogg/os_types.h"
)

source_group("" FILES ${OGG_SRC} ${OGG_HPP})

add_library("ogg" ${OGG_SRC} ${OGG_HPP})

set_property(TARGET "ogg" PROPERTY FOLDER "External Libraries")

disable_project_warnings("ogg")

target_include_directories("ogg" PUBLIC "libogg-1.3.2/include")
