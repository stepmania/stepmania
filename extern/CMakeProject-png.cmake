if(WITH_SYSTEM_PNG)
  find_package(PNG REQUIRED)
  set(PNG_LIBRARIES ${PNG_LIBRARIES} PARENT_SCOPE)
else()
  set(PNG_SRC
      "libpng/png.c"
      "libpng/pngerror.c"
      "libpng/pngget.c"
      "libpng/pngmem.c"
      "libpng/pngpread.c"
      "libpng/pngread.c"
      "libpng/pngrio.c"
      "libpng/pngrtran.c"
      "libpng/pngrutil.c"
      "libpng/pngset.c"
      "libpng/pngtest.c"
      "libpng/pngtrans.c"
      "libpng/pngwio.c"
      "libpng/pngwrite.c"
      "libpng/pngwtran.c"
      "libpng/pngwutil.c")

  configure_file("libpng/scripts/pnglibconf.h.prebuilt"
                 "libpng/pnglibconf.h"
                 COPYONLY)

  set(PNG_HPP
      "libpng/png.h"
      "libpng/pngconf.h"
      "libpng/pngdebug.h"
      "libpng/pnginfo.h"
      "libpng/pnglibconf.h"
      "libpng/pngpriv.h"
      "libpng/pngstruct.h")

  source_group("" FILES ${PNG_SRC})
  source_group("" FILES ${PNG_HPP})

  add_library("png" STATIC ${PNG_SRC} ${PNG_HPP})

  set_property(TARGET "png" PROPERTY FOLDER "External Libraries")

  disable_project_warnings("png")

  if(MSVC)
    sm_add_compile_definition("png" _CRT_SECURE_NO_WARNINGS)
  endif()

  target_include_directories("png" PUBLIC
    "zlib"
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/libpng>
    $<INSTALL_INTERFACE:libpng>
  )
endif()
