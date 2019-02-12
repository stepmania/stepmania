if(WITH_SYSTEM_PNG)
  find_package(PNG REQUIRED)
  set(PNG_LIBRARIES ${PNG_LIBRARIES} PARENT_SCOPE)
else()
  set(PNG_SRC
      "libpng/include/png.c"
      "libpng/include/pngerror.c"
      "libpng/include/pngget.c"
      "libpng/include/pngmem.c"
      "libpng/include/pngpread.c"
      "libpng/include/pngread.c"
      "libpng/include/pngrio.c"
      "libpng/include/pngrtran.c"
      "libpng/include/pngrutil.c"
      "libpng/include/pngset.c"
      "libpng/include/pngtest.c"
      "libpng/include/pngtrans.c"
      "libpng/include/pngwio.c"
      "libpng/include/pngwrite.c"
      "libpng/include/pngwtran.c"
      "libpng/include/pngwutil.c")

  set(PNG_HPP
      "libpng/include/png.h"
      "libpng/include/pngconf.h"
      "libpng/include/pngdebug.h"
      "libpng/include/pnginfo.h"
      "libpng/include/pnglibconf.h"
      "libpng/include/pngpriv.h"
      "libpng/include/pngstruct.h")

  source_group("" FILES ${PNG_SRC})
  source_group("" FILES ${PNG_HPP})

  add_library("png" STATIC ${PNG_SRC} ${PNG_HPP})

  set_property(TARGET "png" PROPERTY FOLDER "External Libraries")

  disable_project_warnings("png")

  if(MSVC)
    sm_add_compile_definition("png" _CRT_SECURE_NO_WARNINGS)
  endif()

  target_include_directories("png" PUBLIC "zlib")
endif()
