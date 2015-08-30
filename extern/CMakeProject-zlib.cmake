set(ZLIB_DIR "zlib-1.2.8")

set(ZLIB_SRC
  "${ZLIB_DIR}/src/adler32.c"
  "${ZLIB_DIR}/src/compress.c"
  "${ZLIB_DIR}/src/crc32.c"
  "${ZLIB_DIR}/src/deflate.c"
  "${ZLIB_DIR}/src/gzclose.c"
  "${ZLIB_DIR}/src/gzlib.c"
  "${ZLIB_DIR}/src/gzread.c"
  "${ZLIB_DIR}/src/gzwrite.c"
  "${ZLIB_DIR}/src/infback.c"
  "${ZLIB_DIR}/src/inffast.c"
  "${ZLIB_DIR}/src/inflate.c"
  "${ZLIB_DIR}/src/inftrees.c"
  "${ZLIB_DIR}/src/trees.c"
  "${ZLIB_DIR}/src/uncompr.c"
  "${ZLIB_DIR}/src/zutil.c"
)

set(ZLIB_HPP
  "${ZLIB_DIR}/src/crc32.h"
  "${ZLIB_DIR}/src/deflate.h"
  "${ZLIB_DIR}/src/gzguts.h"
  "${ZLIB_DIR}/src/inffast.h"
  "${ZLIB_DIR}/src/inffixed.h"
  "${ZLIB_DIR}/src/inflate.h"
  "${ZLIB_DIR}/src/inftrees.h"
  "${ZLIB_DIR}/src/trees.h"
  "${ZLIB_DIR}/include/zconf.h"
  "${ZLIB_DIR}/include/zlib.h"
  "${ZLIB_DIR}/src/zutil.h"
)

source_group("Source Files" FILES ${ZLIB_SRC})
source_group("Header Files" FILES ${ZLIB_HPP})

add_library("zlib" ${ZLIB_SRC} ${ZLIB_HPP})

set_property(TARGET "zlib" PROPERTY FOLDER "External Libraries")

disable_project_warnings("zlib")

if(MSVC)
  sm_add_compile_definition("zlib" _MBCS)
endif(MSVC)

target_include_directories("zlib" PUBLIC "${ZLIB_DIR}/include")
