list(APPEND JPEG_SRC
  "libjpeg/jaricom.c"
  "libjpeg/jcapimin.c"
  "libjpeg/jcapistd.c"
  "libjpeg/jcarith.c"
  "libjpeg/jccoefct.c"
  "libjpeg/jccolor.c"
  "libjpeg/jcdctmgr.c"
  "libjpeg/jchuff.c"
  "libjpeg/jcinit.c"
  "libjpeg/jcmainct.c"
  "libjpeg/jcmarker.c"
  "libjpeg/jcmaster.c"
  "libjpeg/jcomapi.c"
  "libjpeg/jcparam.c"
  "libjpeg/jcprepct.c"
  "libjpeg/jcsample.c"
  "libjpeg/jctrans.c"
  "libjpeg/jdapimin.c"
  "libjpeg/jdapistd.c"
  "libjpeg/jdarith.c"
  "libjpeg/jdatadst.c"
  "libjpeg/jdatasrc.c"
  "libjpeg/jdcoefct.c"
  "libjpeg/jdcolor.c"
  "libjpeg/jddctmgr.c"
  "libjpeg/jdhuff.c"
  "libjpeg/jdinput.c"
  "libjpeg/jdmainct.c"
  "libjpeg/jdmarker.c"
  "libjpeg/jdmaster.c"
  "libjpeg/jdmerge.c"
  "libjpeg/jdpostct.c"
  "libjpeg/jdsample.c"
  "libjpeg/jdtrans.c"
  "libjpeg/jerror.c"
  "libjpeg/jfdctflt.c"
  "libjpeg/jfdctfst.c"
  "libjpeg/jfdctint.c"
  "libjpeg/jidctflt.c"
  "libjpeg/jidctfst.c"
  "libjpeg/jidctint.c"
  "libjpeg/jmemmgr.c"
  "libjpeg/jmemnobs.c"
  "libjpeg/jquant1.c"
  "libjpeg/jquant2.c"
  "libjpeg/jutils.c"
)

if(APPLE)
  list(APPEND JPEG_SRC "jmemmac.c")
endif()

list(APPEND JPEG_HPP
  "libjpeg/jconfig.h"
  "libjpeg/jdct.h"
  "libjpeg/jerror.h"
  "libjpeg/jinclude.h"
  "libjpeg/jmemsys.h"
  "libjpeg/jmorecfg.h"
  "libjpeg/jpegint.h"
  "libjpeg/jpeglib.h"
  "libjpeg/jversion.h"
)

source_group("" FILES ${JPEG_SRC} ${JPEG_HPP})

add_library("jpeg" ${JPEG_SRC} ${JPEG_HPP})

disable_project_warnings("jpeg")

set_property(TARGET "jpeg" PROPERTY FOLDER "External Libraries")

target_include_directories("jpeg" PUBLIC "libjpeg")

if(MSVC)
  sm_add_compile_definition("jpeg" _CRT_SECURE_NO_WARNINGS)
endif(MSVC)