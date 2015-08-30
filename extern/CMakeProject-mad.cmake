set(MAD_DIR "libmad-0.15.1b")

set(MAD_SRC
  "${MAD_DIR}/src/bit.c"
  "${MAD_DIR}/src/decoder.c"
  "${MAD_DIR}/src/fixed.c"
  "${MAD_DIR}/src/frame.c"
  "${MAD_DIR}/src/huffman.c"
  "${MAD_DIR}/src/layer12.c"
  "${MAD_DIR}/src/layer3.c"
  "${MAD_DIR}/src/stream.c"
  "${MAD_DIR}/src/synth.c"
  "${MAD_DIR}/src/timer.c"
  "${MAD_DIR}/src/version.c"
)

set(MAD_HPP
  "${MAD_DIR}/src/bit.h"
  "${MAD_DIR}/src/decoder.h"
  "${MAD_DIR}/src/fixed.h"
  "${MAD_DIR}/src/frame.h"
  "${MAD_DIR}/src/global.h"
  "${MAD_DIR}/src/huffman.h"
  "${MAD_DIR}/src/layer12.h"
  "${MAD_DIR}/src/layer3.h"
  "${MAD_DIR}/src/stream.h"
  "${MAD_DIR}/src/synth.h"
  "${MAD_DIR}/src/timer.h"
  "${MAD_DIR}/src/version.h"
)

set(MAD_DAT
  "${MAD_DIR}/src/D.dat"
  "${MAD_DIR}/src/imdct_s.dat"
  "${MAD_DIR}/src/qc_table.dat"
  "${MAD_DIR}/src/rq_table.dat"
  "${MAD_DIR}/src/sf_table.dat"
)

source_group("Source Files" FILES ${MAD_SRC})
source_group("Header Files" FILES ${MAD_HPP})
source_group("Data Files" FILES ${MAD_DAT})

add_library("mad" ${MAD_SRC} ${MAD_HPP} ${MAD_DAT})

set_property(TARGET "mad" PROPERTY FOLDER "External Libraries")

disable_project_warnings("mad")

if(MSVC)
  sm_add_compile_definition("mad" _CRT_SECURE_NO_WARNINGS)
  sm_add_compile_definition("mad" ASO_ZEROCHECK)
  sm_add_compile_definition("mad" $<$<CONFIG:Debug>:FPM_DEFAULT>)
  sm_add_compile_definition("mad" $<$<CONFIG:Release>:FPM_INTEL>)
  sm_add_compile_definition("mad" $<$<CONFIG:MinSizeRel>:FPM_INTEL>)
  sm_add_compile_definition("mad" $<$<CONFIG:RelWithDebInfo>:FPM_INTEL>)
  sm_add_compile_definition("mad" HAVE_ASSERT_H=1)
  sm_add_compile_definition("mad" HAVE_ERRNO_H=1)
  sm_add_compile_definition("mad" HAVE_FCNTL_H=1)
  sm_add_compile_definition("mad" HAVE_INTTYPES_H=1)
  sm_add_compile_definition("mad" HAVE_LIMITS_H=1)
  sm_add_compile_definition("mad" HAVE_MEMORY_H=1)
  sm_add_compile_definition("mad" HAVE_STDINT_H=1)
  sm_add_compile_definition("mad" HAVE_STDLIB_H=1)
  sm_add_compile_definition("mad" HAVE_STRINGS_H=1)
  sm_add_compile_definition("mad" HAVE_STRING_H=1)
  sm_add_compile_definition("mad" HAVE_SYS_STAT_H=1)
  sm_add_compile_definition("mad" HAVE_SYS_TYPES_H=1)
  sm_add_compile_definition("mad" PACKAGE="libmad")
  sm_add_compile_definition("mad" PACKAGE_BUGREPORT="support@underbit.com")
  sm_add_compile_definition("mad" PACKAGE_NAME="MPEG Audio Decoder")
  sm_add_compile_definition("mad" PACKAGE_STRING="MPEG Audio Decoder 0.15.1b")
  sm_add_compile_definition("mad" PACKAGE_TARNAME="libmad")
  sm_add_compile_definition("mad" PACKAGE_VERSION="0.15.1b")
  sm_add_compile_definition("mad" SIZEOF_INT=4)
  sm_add_compile_definition("mad" SIZEOF_LONG=4)
  sm_add_compile_definition("mad" SIZEOF_LONG_LONG=8)
  sm_add_compile_definition("mad" STDC_HEADERS=1)
  sm_add_compile_definition("mad" VERSION="0.15.1b")
  sm_add_compile_definition("mad" inline=__inline)
elseif(APPLE)
  sm_add_compile_definition("mad" HAVE_ASSERT_H=1)
  sm_add_compile_definition("mad" HAVE_DLFCN_H=1)
  sm_add_compile_definition("mad" HAVE_ERRNO_H=1)
  sm_add_compile_definition("mad" HAVE_FORK=1)
  sm_add_compile_definition("mad" HAVE_INTTYPES_H=1)
  sm_add_compile_definition("mad" HAVE_LIMITS_H=1)
  sm_add_compile_definition("mad" HAVE_MEMORY_H=1)
  sm_add_compile_definition("mad" HAVE_WAITPID=1)
  sm_add_compile_definition("mad" SIZEOF_LONG=4)
  sm_add_compile_definition("mad" FPM_64BIT=1)
endif(MSVC)

target_include_directories("mad" PUBLIC "${MAD_DIR}/include")
