list(APPEND JPEG_SRC
  "libjpeg/src/jaricom.c"
  "libjpeg/src/jcapimin.c"
  "libjpeg/src/jcapistd.c"
  "libjpeg/src/jcarith.c"
  "libjpeg/src/jccoefct.c"
  "libjpeg/src/jccolor.c"
  "libjpeg/src/jcdctmgr.c"
  "libjpeg/src/jchuff.c"
  "libjpeg/src/jcinit.c"
  "libjpeg/src/jcmainct.c"
  "libjpeg/src/jcmarker.c"
  "libjpeg/src/jcmaster.c"
  "libjpeg/src/jcomapi.c"
  "libjpeg/src/jcparam.c"
  "libjpeg/src/jcprepct.c"
  "libjpeg/src/jcsample.c"
  "libjpeg/src/jctrans.c"
  "libjpeg/src/jdapimin.c"
  "libjpeg/src/jdapistd.c"
  "libjpeg/src/jdarith.c"
  "libjpeg/src/jdatadst.c"
  "libjpeg/src/jdatasrc.c"
  "libjpeg/src/jdcoefct.c"
  "libjpeg/src/jdcolor.c"
  "libjpeg/src/jddctmgr.c"
  "libjpeg/src/jdhuff.c"
  "libjpeg/src/jdinput.c"
  "libjpeg/src/jdmainct.c"
  "libjpeg/src/jdmarker.c"
  "libjpeg/src/jdmaster.c"
  "libjpeg/src/jdmerge.c"
  "libjpeg/src/jdpostct.c"
  "libjpeg/src/jdsample.c"
  "libjpeg/src/jdtrans.c"
  "libjpeg/src/jerror.c"
  "libjpeg/src/jfdctflt.c"
  "libjpeg/src/jfdctfst.c"
  "libjpeg/src/jfdctint.c"
  "libjpeg/src/jidctflt.c"
  "libjpeg/src/jidctfst.c"
  "libjpeg/src/jidctint.c"
  "libjpeg/src/jmemmgr.c"
  "libjpeg/src/jmemnobs.c"
  "libjpeg/src/jquant1.c"
  "libjpeg/src/jquant2.c"
  "libjpeg/src/jutils.c"
)

if(APPLE)
  list(APPEND JPEG_SRC "jmemmac.c")
endif()

list(APPEND JPEG_HPP
  "libjpeg/include/jconfig.h"
  "libjpeg/src/jdct.h"
  "libjpeg/include/jerror.h"
  "libjpeg/src/jinclude.h"
  "libjpeg/src/jmemsys.h"
  "libjpeg/include/jmorecfg.h"
  "libjpeg/include/jpegint.h"
  "libjpeg/include/jpeglib.h"
  "libjpeg/src/jversion.h"
)

source_group("" FILES ${JPEG_SRC} ${JPEG_HPP})

add_library("jpeg" ${JPEG_SRC} ${JPEG_HPP})

disable_project_warnings("jpeg")

set_property(TARGET "jpeg" PROPERTY FOLDER "External Libraries")

if(MSVC)
  sm_add_compile_definition("jpeg" _CRT_SECURE_NO_WARNINGS)
endif(MSVC)

target_include_directories("jpeg" PUBLIC "libjpeg/include")