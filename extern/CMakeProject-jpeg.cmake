if(WITH_SYSTEM_JPEG)
  find_package(JPEG REQUIRED)
  set(JPEG_LIBRARIES ${JPEG_LIBRARIES} PARENT_SCOPE)
else()
  set(JPEG_DIR "${SM_EXTERN_DIR}/libjpeg")

  configure_file("${SM_EXTERN_DIR}/config.jpeg.in.h" "${JPEG_DIR}/jconfig.h")

  if(NOT HAVE_PROTOTYPES)
    list(APPEND JPEG_SRC "${JPEG_DIR}/ansi2knr.c")
  endif()

  list(APPEND JPEG_SRC
              "${JPEG_DIR}/jaricom.c"
              "${JPEG_DIR}/jcapimin.c"
              "${JPEG_DIR}/jcapistd.c"
              "${JPEG_DIR}/jcarith.c"
              "${JPEG_DIR}/jccoefct.c"
              "${JPEG_DIR}/jccolor.c"
              "${JPEG_DIR}/jcdctmgr.c"
              "${JPEG_DIR}/jchuff.c"
              "${JPEG_DIR}/jcinit.c"
              "${JPEG_DIR}/jcmainct.c"
              "${JPEG_DIR}/jcmarker.c"
              "${JPEG_DIR}/jcmaster.c"
              "${JPEG_DIR}/jcomapi.c"
              "${JPEG_DIR}/jcparam.c"
              "${JPEG_DIR}/jcprepct.c"
              "${JPEG_DIR}/jcsample.c"
              "${JPEG_DIR}/jctrans.c"
              "${JPEG_DIR}/jdapimin.c"
              "${JPEG_DIR}/jdapistd.c"
              "${JPEG_DIR}/jdarith.c"
              "${JPEG_DIR}/jdatadst.c"
              "${JPEG_DIR}/jdcoefct.c"
              "${JPEG_DIR}/jdcolor.c"
              "${JPEG_DIR}/jddctmgr.c"
              "${JPEG_DIR}/jdhuff.c"
              "${JPEG_DIR}/jdinput.c"
              "${JPEG_DIR}/jdmainct.c"
              "${JPEG_DIR}/jdmarker.c"
              "${JPEG_DIR}/jdmaster.c"
              "${JPEG_DIR}/jdmerge.c"
              "${JPEG_DIR}/jdpostct.c"
              "${JPEG_DIR}/jdsample.c"
              "${JPEG_DIR}/jdtrans.c"
              "${JPEG_DIR}/jerror.c"
              "${JPEG_DIR}/jfdctflt.c"
              "${JPEG_DIR}/jfdctfst.c"
              "${JPEG_DIR}/jfdctint.c"
              "${JPEG_DIR}/jutils.c"
              "${JPEG_DIR}/jidctflt.c"
              "${JPEG_DIR}/jidctfst.c"
              "${JPEG_DIR}/jidctint.c"
              "${JPEG_DIR}/jmemmgr.c"
              "${JPEG_DIR}/jmemnobs.c"
              "${JPEG_DIR}/jquant1.c"
              "${JPEG_DIR}/jquant2.c")

  list(APPEND JPEG_HPP
              "${JPEG_DIR}/jconfig.h"
              "${JPEG_DIR}/jdct.h"
              "${JPEG_DIR}/jerror.h"
              "${JPEG_DIR}/jinclude.h"
              "${JPEG_DIR}/jmemsys.h"
              "${JPEG_DIR}/jmorecfg.h"
              "${JPEG_DIR}/jpegint.h"
              "${JPEG_DIR}/jpeglib.h"
              "${JPEG_DIR}/jversion.h")

  source_group("Source Files" FILES ${JPEG_SRC})
  source_group("Header Files" FILES ${JPEG_HPP})

  add_library("jpeg" ${JPEG_SRC} ${JPEG_HPP})

  disable_project_warnings("jpeg")

  set_property(TARGET "jpeg" PROPERTY FOLDER "External Libraries")

  if(MSVC)
    sm_add_compile_definition("jpeg" _CRT_SECURE_NO_WARNINGS)
  endif(MSVC)

  target_include_directories("jpeg" PUBLIC "${JPEG_DIR}")
endif()
