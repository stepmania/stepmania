set(LUA_SRC
    "lua-5.1/src/lapi.c"
    "lua-5.1/src/lauxlib.c"
    "lua-5.1/src/lbaselib.c"
    "lua-5.1/src/lcode.c"
    "lua-5.1/src/ldblib.c"
    "lua-5.1/src/ldebug.c"
    "lua-5.1/src/ldo.c"
    "lua-5.1/src/ldump.c"
    "lua-5.1/src/lfunc.c"
    "lua-5.1/src/lgc.c"
    "lua-5.1/src/linit.c"
    "lua-5.1/src/liolib.c"
    "lua-5.1/src/llex.c"
    "lua-5.1/src/lmathlib.c"
    "lua-5.1/src/lmem.c"
    "lua-5.1/src/loadlib.c"
    "lua-5.1/src/lobject.c"
    "lua-5.1/src/lopcodes.c"
    "lua-5.1/src/loslib.c"
    "lua-5.1/src/lparser.c"
    "lua-5.1/src/lstate.c"
    "lua-5.1/src/lstring.c"
    "lua-5.1/src/lstrlib.c"
    "lua-5.1/src/ltable.c"
    "lua-5.1/src/ltablib.c"
    "lua-5.1/src/ltm.c"
    "lua-5.1/src/lundump.c"
    "lua-5.1/src/lvm.c"
    "lua-5.1/src/lzio.c")

set(LUA_HPP
    "lua-5.1/src/lapi.h"
    "lua-5.1/src/lauxlib.h"
    "lua-5.1/src/lcode.h"
    "lua-5.1/src/ldebug.h"
    "lua-5.1/src/lfunc.h"
    "lua-5.1/src/lgc.h"
    "lua-5.1/src/llex.h"
    "lua-5.1/src/llimits.h"
    "lua-5.1/src/lmem.h"
    "lua-5.1/src/lobject.h"
    "lua-5.1/src/lopcodes.h"
    "lua-5.1/src/lparser.h"
    "lua-5.1/src/lstate.h"
    "lua-5.1/src/lstring.h"
    "lua-5.1/src/ltable.h"
    "lua-5.1/src/ltm.h"
    "lua-5.1/src/lua.h"
    "lua-5.1/src/luaconf.h"
    "lua-5.1/src/lualib.h"
    "lua-5.1/src/lundump.h"
    "lua-5.1/src/lvm.h"
    "lua-5.1/src/lzio.h")

source_group("" FILES ${LUA_SRC})
source_group("" FILES ${LUA_HPP})

add_library("lua-5.1" STATIC ${LUA_SRC} ${LUA_HPP})

set_property(TARGET "lua-5.1" PROPERTY FOLDER "External Libraries")

# include_directories(src)

if(MSVC)
  sm_add_compile_definition("lua-5.1" _CRT_SECURE_NO_WARNINGS)
  set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
endif(MSVC)

disable_project_warnings("lua-5.1")
