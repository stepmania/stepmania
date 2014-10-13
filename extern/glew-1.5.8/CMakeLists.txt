include_directories(include)

if(MSVC)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("glew")
	add_definitions(/D GLEW_STATIC)
	add_definitions(/D _MBCS)
endif(MSVC)

set(GLEW_SRC "src/glew.c")

source_group("" FILES ${GLEW_SRC})

add_library("glew" ${GLEW_SRC})

set_property(TARGET glew PROPERTY FOLDER "External Libraries")