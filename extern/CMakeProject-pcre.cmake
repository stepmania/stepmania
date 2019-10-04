if(WITH_SYSTEM_PCRE)
  find_package(Pcre REQUIRED)
else()
  set(PCRE_SRC "pcre/get.c" "pcre/maketables.c" "pcre/pcre.c" "pcre/study.c")

  set(PCRE_HPP "pcre/internal.h" "pcre/pcre.h")

  source_group("" FILES ${PCRE_SRC})
  source_group("" FILES ${PCRE_HPP})

  add_library("pcre" ${PCRE_SRC} ${PCRE_HPP})

  set_property(TARGET "pcre" PROPERTY FOLDER "External Libraries")

  disable_project_warnings("pcre")
endif()
