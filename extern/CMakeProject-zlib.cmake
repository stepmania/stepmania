set(ZLIB_SRC "zlib/adler32.c"
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
             "zlib/trees.c"
             "zlib/uncompr.c"
             "zlib/zutil.c")

set(ZLIB_HPP "zlib/crc32.h"
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

add_library("zlib" STATIC ${ZLIB_SRC} ${ZLIB_HPP})

set_property(TARGET "zlib" PROPERTY FOLDER "External Libraries")

# Fixes clang build failures
# See stepmania/stepmania#107 for details
# TODO(teejusb/natano): Remove these two lines once these issues have been fixed upstream.
set_property(TARGET "zlib" PROPERTY C_STANDARD 90)
set_property(TARGET "zlib" PROPERTY C_STANDARD_REQUIRED ON)

disable_project_warnings("zlib")

if(MSVC)
  target_compile_definitions("zlib" PRIVATE _MBCS)
endif(MSVC)

target_include_directories("zlib" PUBLIC "zlib")
