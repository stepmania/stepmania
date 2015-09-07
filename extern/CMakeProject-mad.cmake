set(MAD_DIR "libmad-0.15.1b")

check_include_file(assert.h HAVE_ASSERT_H)
check_include_file(dlfcn.h HAVE_DLFCN_H)
check_include_file(errno.h HAVE_ERRNO_H)
check_function_exists(fcntl HAVE_FCNTL)
check_function_exists(fork HAVE_FORK)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_function_exists(pipe HAVE_PIPE)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
check_function_exists(waitpid HAVE_WAITPID)
check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

configure_file("${MAD_DIR}/config.h.in" "${SM_EXTERN_DIR}/${MAD_DIR}/config.h")

set(MAD_SRC
  "${MAD_DIR}/bit.c"
  "${MAD_DIR}/decoder.c"
  "${MAD_DIR}/fixed.c"
  "${MAD_DIR}/frame.c"
  "${MAD_DIR}/huffman.c"
  "${MAD_DIR}/layer12.c"
  "${MAD_DIR}/layer3.c"
  "${MAD_DIR}/stream.c"
  "${MAD_DIR}/synth.c"
  "${MAD_DIR}/timer.c"
  "${MAD_DIR}/version.c"
)

set(MAD_HPP
  "${MAD_DIR}/bit.h"
  "${MAD_DIR}/decoder.h"
  "${MAD_DIR}/fixed.h"
  "${MAD_DIR}/frame.h"
  "${MAD_DIR}/global.h"
  "${MAD_DIR}/huffman.h"
  "${MAD_DIR}/layer12.h"
  "${MAD_DIR}/layer3.h"
  "${MAD_DIR}/stream.h"
  "${MAD_DIR}/synth.h"
  "${MAD_DIR}/timer.h"
  "${MAD_DIR}/version.h"
)

set(MAD_DAT
  "${MAD_DIR}/D.dat"
  "${MAD_DIR}/imdct_s.dat"
  "${MAD_DIR}/qc_table.dat"
  "${MAD_DIR}/rq_table.dat"
  "${MAD_DIR}/sf_table.dat"
)

source_group("Source Files" FILES ${MAD_SRC})
source_group("Header Files" FILES ${MAD_HPP})
source_group("Data Files" FILES ${MAD_DAT})

add_library("mad" ${MAD_SRC} ${MAD_HPP} ${MAD_DAT})

set_property(TARGET "mad" PROPERTY FOLDER "External Libraries")

disable_project_warnings("mad")

sm_add_compile_definition("mad" $<$<CONFIG:Debug>:DEBUG>)
sm_add_compile_definition("mad" HAVE_CONFIG_H)

if(MSVC)
  sm_add_compile_definition("mad" _CRT_SECURE_NO_WARNINGS)
  sm_add_compile_definition("mad" ASO_ZEROCHECK)
  sm_add_compile_definition("mad" $<$<CONFIG:Debug>:FPM_DEFAULT>)
  sm_add_compile_definition("mad" $<$<CONFIG:Release>:FPM_INTEL>)
  sm_add_compile_definition("mad" $<$<CONFIG:MinSizeRel>:FPM_INTEL>)
  sm_add_compile_definition("mad" $<$<CONFIG:RelWithDebInfo>:FPM_INTEL>)
  sm_add_compile_definition("mad" inline=__inline)
elseif(APPLE)
  sm_add_compile_definition("mad" FPM_64BIT=1)
endif(MSVC)

target_include_directories("mad" PUBLIC "${MAD_DIR}")
