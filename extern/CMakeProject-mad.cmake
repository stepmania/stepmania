list(APPEND MAD_SRC
  "mad-0.15.1b/bit.c"
  "mad-0.15.1b/decoder.c"
  "mad-0.15.1b/fixed.c"
  "mad-0.15.1b/frame.c"
  "mad-0.15.1b/huffman.c"
  "mad-0.15.1b/layer12.c"
  "mad-0.15.1b/layer3.c"
  "mad-0.15.1b/stream.c"
  "mad-0.15.1b/synth.c"
  "mad-0.15.1b/timer.c"
  "mad-0.15.1b/version.c"
)

list(APPEND MAD_HPP
  "mad-0.15.1b/bit.h"
  "mad-0.15.1b/decoder.h"
  "mad-0.15.1b/fixed.h"
  "mad-0.15.1b/frame.h"
  "mad-0.15.1b/global.h"
  "mad-0.15.1b/huffman.h"
  "mad-0.15.1b/layer12.h"
  "mad-0.15.1b/layer3.h"
  "mad-0.15.1b/mad.h"
  "mad-0.15.1b/stream.h"
  "mad-0.15.1b/synth.h"
  "mad-0.15.1b/timer.h"
  "mad-0.15.1b/version.h"
)

if(ANDROID)
  #list(APPEND MAD_HPP "mad-0.15.1b/mad_android/config.h")
endif(ANDROID)

source_group("" FILES ${MAD_SRC} ${MAD_HPP})

add_library("mad" ${MAD_SRC} ${MAD_HPP})

set_property(TARGET "mad" PROPERTY FOLDER "External Libraries")

disable_project_warnings("mad")

if(MSVC)
  sm_add_compile_definition("mad" _CRT_SECURE_NO_WARNINGS)
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
elseif(ANDROID)
  sm_add_compile_definition("mad" HAVE_ASSERT_H=1)
  sm_add_compile_definition("mad" HAVE_ERRNO_H=1)
  sm_add_compile_definition("mad" HAVE_FCNTL=1)
  sm_add_compile_definition("mad" HAVE_FCNTL_H=1)
  sm_add_compile_definition("mad" HAVE_FORK=1)
  sm_add_compile_definition("mad" HAVE_INTTYPES_H=1)
  sm_add_compile_definition("mad" HAVE_LIMITS_H=1)
  sm_add_compile_definition("mad" HAVE_PIPE=1)
  sm_add_compile_definition("mad" HAVE_STDLIB_H=1)
  sm_add_compile_definition("mad" HAVE_STRING_H=1)
  sm_add_compile_definition("mad" HAVE_SYS_TYPES_H=1)
  sm_add_compile_definition("mad" HAVE_SYS_WAIT_H=1)
  sm_add_compile_definition("mad" HAVE_UNISTD_H=1)
  sm_add_compile_definition("mad" HAVE_WAITPID=1)
  sm_add_compile_definition("mad" OPT_SPEED=1)
  sm_add_compile_definition("mad" SIZEOF_INT=4)
  sm_add_compile_definition("mad" STDC_HEADERS=1)
  sm_add_compile_definition("mad" FPM_ARM=1)
endif(MSVC)