set(PNG_DIR "libpng-1.6.17")

set(PNG_SRC
  "${PNG_DIR}/src/png.c"
  "${PNG_DIR}/src/pngerror.c"
  "${PNG_DIR}/src/pngget.c"
  "${PNG_DIR}/src/pngmem.c"
  "${PNG_DIR}/src/pngpread.c"
  "${PNG_DIR}/src/pngread.c"
  "${PNG_DIR}/src/pngrio.c"
  "${PNG_DIR}/src/pngrtran.c"
  "${PNG_DIR}/src/pngrutil.c"
  "${PNG_DIR}/src/pngset.c"
  "${PNG_DIR}/src/pngtrans.c"
  "${PNG_DIR}/src/pngwio.c"
  "${PNG_DIR}/src/pngwrite.c"
  "${PNG_DIR}/src/pngwtran.c"
  "${PNG_DIR}/src/pngwutil.c"
)

set(PNG_HPP
  "${PNG_DIR}/include/png.h"
  "${PNG_DIR}/include/pngconf.h"
  "${PNG_DIR}/src/pngdebug.h"
  "${PNG_DIR}/src/pnginfo.h"
  "${PNG_DIR}/include/pnglibconf.h"
  "${PNG_DIR}/src/pngpriv.h"
  "${PNG_DIR}/src/pngstruct.h"
)

source_group("Source Files" FILES ${PNG_SRC})
source_group("Header Files" FILES ${PNG_HPP})

add_library("png" ${PNG_SRC} ${PNG_HPP})

set_property(TARGET "png" PROPERTY FOLDER "External Libraries")

disable_project_warnings("png")

if(MSVC)
  sm_add_compile_definition("png" _CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories("png" PUBLIC "${PNG_DIR}/include")

if(APPLE OR MSVC)
  target_include_directories("png" PRIVATE "${ZLIB_DIR}/include")
endif()
