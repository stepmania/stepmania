set(GLEW_SRC "glew-1.12.0/src/glew.c")

set(GLEW_HPP "glew-1.12.0/include/GL/glew.h")

if(WIN32)
  list(APPEND GLEW_HPP "glew-1.12.0/include/GL/wglew.h")
elseif(LINUX)
  list(APPEND GLEW_HPP "glew-1.12.0/include/GL/glxew.h")
endif(WIN32)

source_group("" FILES ${GLEW_SRC} ${GLEW_HPP})

add_library("glew" ${GLEW_SRC} ${GLEW_HPP})

set_property(TARGET "glew" PROPERTY FOLDER "External Libraries")

target_include_directories("glew" PUBLIC "glew-1.12.0/include")

sm_add_compile_definition("glew" GLEW_STATIC)

if(MSVC)
  sm_add_compile_definition("glew" _MBCS)
endif(MSVC)
