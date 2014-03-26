# include_directories(src)

if(MSVC)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("lua-5.1")
endif(MSVC)

set(LUA_SRC 
	"src/lapi.c"
	"src/lauxlib.c"
	"src/lbaselib.c"
	"src/lcode.c"
	"src/ldblib.c"
	"src/ldebug.c"
	"src/ldo.c"
	"src/ldump.c"
	"src/lfunc.c"
	"src/lgc.c"
	"src/linit.c"
	"src/liolib.c"
	"src/llex.c"
	"src/lmathlib.c"
	"src/lmem.c"
	"src/loadlib.c"
	"src/lobject.c"
	"src/lopcodes.c"
	"src/loslib.c"
	"src/lparser.c"
	"src/lstate.c"
	"src/lstring.c"
	"src/lstrlib.c"
	"src/ltable.c"
	"src/ltablib.c"
	"src/ltm.c"
	"src/lua.c"
	"src/lundump.c"
	"src/lvm.c"
	"src/lzio.c")

set(LUA_HPP
	"src/lapi.h"
	"src/lauxlib.h"
	"src/lcode.h"
	"src/ldebug.h"
	"src/lfunc.h"
	"src/lgc.h"
	"src/llex.h"
	"src/llimits.h"
	"src/lmem.h"
	"src/lobject.h"
	"src/lopcodes.h"
	"src/lparser.h"
	"src/lstate.h"
	"src/lstring.h"
	"src/ltable.h"
	"src/ltm.h"
	"src/lua.h"
	"src/luaconf.h"
	"src/lualib.h"
	"src/lundump.h"
	"src/lvm.h"
	"src/lzio.h")

source_group("" FILES ${LUA_SRC})
source_group("" FILES ${LUA_HPP})

add_library("lua-5.1" ${LUA_SRC} ${LUA_HPP})

set_property(TARGET "lua-5.1" PROPERTY FOLDER "External Libraries")