if(MSVC)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("pcre")
endif(MSVC)

set(PCRE_SRC
#	"chartables.c"
	"get.c"
	"maketables.c"
	"pcre.c"
	"study.c")

set(PCRE_HPP
	"internal.h"
	"pcre.h")

source_group("" FILES ${PCRE_SRC})
source_group("" FILES ${PCRE_HPP})

add_library("pcre" ${PCRE_SRC} ${PCRE_HPP})