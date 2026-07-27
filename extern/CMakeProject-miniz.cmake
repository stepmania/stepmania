set(MINIZ_SRC "miniz/miniz.c")
set(MINIZ_HPP "miniz/miniz.h")

source_group("" FILES ${MINIZ_SRC} ${MINIZ_HPP})

add_library("miniz" STATIC ${MINIZ_SRC} ${MINIZ_HPP})

set_property(TARGET "miniz" PROPERTY FOLDER "External Libraries")

disable_project_warnings("miniz")

if(MSVC)
  target_compile_definitions("miniz" PRIVATE _MBCS)
endif(MSVC)

target_include_directories("miniz" PUBLIC "miniz")
