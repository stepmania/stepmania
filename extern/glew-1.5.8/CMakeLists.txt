include_directories(include)

add_definitions(/D GLEW_STATIC)

if(MSVC)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("glew")
endif(MSVC)

set(GLEW_SRC "src/glew.c")

source_group("" FILES ${GLEW_SRC})

add_library("glew" ${GLEW_SRC})