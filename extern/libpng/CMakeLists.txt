include_directories("../zlib")

if(MSVC)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("png")
endif(MSVC)

set(PNG_SRC 
	"include/png.c"
	"include/pngerror.c"
	"include/pngget.c"
	"include/pngmem.c"
	"include/pngpread.c"
	"include/pngread.c"
	"include/pngrio.c"
	"include/pngrtran.c"
	"include/pngrutil.c"
	"include/pngset.c"
	"include/pngtest.c"
	"include/pngtrans.c"
	"include/pngwio.c"
	"include/pngwrite.c"
	"include/pngwtran.c"
	"include/pngwutil.c")

set(PNG_HPP
	"include/png.h"
	"include/pngconf.h"
	"include/pngdebug.h"
	"include/pnginfo.h"
	"include/pnglibconf.h"
	"include/pngpriv.h"
	"include/pngstruct.h")

source_group("" FILES ${PNG_SRC})
source_group("" FILES ${PNG_HPP})

add_library("png" ${PNG_SRC} ${PNG_HPP})

set_property(TARGET png PROPERTY FOLDER "External Libraries")