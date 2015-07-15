set(PNG_SRC
  "libpng-1.6.17/src/png.c"
  "libpng-1.6.17/src/pngerror.c"
  "libpng-1.6.17/src/pngget.c"
  "libpng-1.6.17/src/pngmem.c"
  "libpng-1.6.17/src/pngpread.c"
  "libpng-1.6.17/src/pngread.c"
  "libpng-1.6.17/src/pngrio.c"
  "libpng-1.6.17/src/pngrtran.c"
  "libpng-1.6.17/src/pngrutil.c"
  "libpng-1.6.17/src/pngset.c"
  "libpng-1.6.17/src/pngtrans.c"
  "libpng-1.6.17/src/pngwio.c"
  "libpng-1.6.17/src/pngwrite.c"
  "libpng-1.6.17/src/pngwtran.c"
  "libpng-1.6.17/src/pngwutil.c"
)

set(PNG_HPP
  "libpng-1.6.17/include/png.h"
  "libpng-1.6.17/include/pngconf.h"
  "libpng-1.6.17/src/pngdebug.h"
  "libpng-1.6.17/src/pnginfo.h"
  "libpng-1.6.17/include/pnglibconf.h"
  "libpng-1.6.17/src/pngpriv.h"
  "libpng-1.6.17/src/pngstruct.h"
)

source_group("" FILES ${PNG_SRC})
source_group("" FILES ${PNG_HPP})

add_library("png" ${PNG_SRC} ${PNG_HPP})

set_property(TARGET "png" PROPERTY FOLDER "External Libraries")

disable_project_warnings("png")

if(MSVC)
  sm_add_compile_definition("png" _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories("png" PUBLIC "libpng-1.6.17/include")
target_include_directories("png" PUBLIC "zlib")
