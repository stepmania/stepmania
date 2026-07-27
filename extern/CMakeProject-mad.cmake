set(MAD_SRC "libmad/bit.c"
            "libmad/decoder.c"
            "libmad/fixed.c"
            "libmad/frame.c"
            "libmad/huffman.c"
            "libmad/layer12.c"
            "libmad/layer3.c"
            "libmad/stream.c"
            "libmad/synth.c"
            "libmad/timer.c"
            "libmad/version.c")

set(MAD_HPP "libmad/bit.h"
            "libmad/config.h"
            "libmad/decoder.h"
            "libmad/fixed.h"
            "libmad/frame.h"
            "libmad/global.h"
            "libmad/huffman.h"
            "libmad/layer12.h"
            "libmad/layer3.h"
            "libmad/mad.h"
            "libmad/stream.h"
            "libmad/synth.h"
            "libmad/timer.h"
            "libmad/version.h")

set(MAD_DAT "libmad/D.dat"
            "libmad/imdct_s.dat"
            "libmad/qc_table.dat"
            "libmad/rq_table.dat"
            "libmad/sf_table.dat")

source_group("Source Files" FILES ${MAD_SRC})
source_group("Header Files" FILES ${MAD_HPP})
source_group("Data Files" FILES ${MAD_DAT})

add_library("mad" STATIC ${MAD_SRC} ${MAD_HPP} ${MAD_DAT})

set_property(TARGET "mad" PROPERTY FOLDER "External Libraries")

disable_project_warnings("mad")

if(ENDIAN_BIG)
  set(WORDS_BIGENDIAN 1)
endif()

target_compile_definitions("mad" PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions("mad" PRIVATE HAVE_CONFIG_H)

if(MSVC)
  target_compile_definitions("mad" PRIVATE _CRT_SECURE_NO_WARNINGS)
  # TODO: Remove the need for this check since it's tied to 32-bit builds from
  # first glance.
  target_compile_definitions("mad" PRIVATE ASO_ZEROCHECK)
  target_compile_definitions("mad" PRIVATE $<$<CONFIG:Debug>:FPM_DEFAULT>)
  if(SM_WIN32_ARCH MATCHES "x64")
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:Release>:FPM_64BIT>)
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:MinSizeRel>:FPM_64BIT>)
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:RelWithDebInfo>:FPM_64BIT>)
  else()
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:Release>:FPM_INTEL>)
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:MinSizeRel>:FPM_INTEL>)
    target_compile_definitions("mad" PRIVATE $<$<CONFIG:RelWithDebInfo>:FPM_INTEL>)
  endif()
  # TODO: Provide a proper define for inline.
  target_compile_definitions("mad" PRIVATE inline=__inline)
elseif(APPLE OR UNIX)
  target_compile_definitions("mad" PRIVATE FPM_64BIT=1)
endif(MSVC)

target_include_directories("mad" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/libmad")
target_include_directories("mad" PUBLIC "libmad")

configure_file("config.mad.in.h" "libmad/config.h")
