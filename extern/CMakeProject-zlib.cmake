if(WITH_SYSTEM_ZLIB)
  find_package(ZLIB REQUIRED)
else()
  list(APPEND ZLIB_SRC
              "zlib/adler32.c"
              "zlib/compress.c"
              "zlib/crc32.c"
              "zlib/deflate.c"
              "zlib/gzclose.c"
              "zlib/gzlib.c"
              "zlib/gzread.c"
              "zlib/gzwrite.c"
              "zlib/infback.c"
              "zlib/inffast.c"
              "zlib/inflate.c"
              "zlib/inftrees.c"
              "zlib/minigzip.c"
              "zlib/trees.c"
              "zlib/uncompr.c"
              "zlib/zutil.c")

  list(APPEND ZLIB_HPP
              "zlib/crc32.h"
              "zlib/deflate.h"
              "zlib/gzguts.h"
              "zlib/inffast.h"
              "zlib/inffixed.h"
              "zlib/inflate.h"
              "zlib/inftrees.h"
              "zlib/trees.h"
              "zlib/zconf.h"
              "zlib/zlib.h"
              "zlib/zutil.h")

  source_group("" FILES ${ZLIB_SRC} ${ZLIB_HPP})

  add_library("zlib" ${ZLIB_SRC} ${ZLIB_HPP})

  set_property(TARGET "zlib" PROPERTY FOLDER "External Libraries")

  disable_project_warnings("zlib")

  if(MSVC)
    sm_add_compile_definition("zlib" _MBCS)
  endif(MSVC)
endif()
