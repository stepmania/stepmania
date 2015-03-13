set(GLEW_SRC "glew-1.5.8/src/glew.c")

source_group("" FILES ${GLEW_SRC})

add_library("glew" ${GLEW_SRC})

set_property(TARGET "glew" PROPERTY FOLDER "External Libraries")

target_include_directories("glew" PUBLIC "glew-1.5.8/include")

sm_add_compile_definition("glew" GLEW_STATIC)

if(MSVC)
  sm_add_compile_definition("glew" _MBCS)
endif(MSVC)

