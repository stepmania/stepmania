set(LUA_SRC
  "liblua-5.1.5/src/lapi.c"
  "liblua-5.1.5/src/lauxlib.c"
  "liblua-5.1.5/src/lbaselib.c"
  "liblua-5.1.5/src/lcode.c"
  "liblua-5.1.5/src/ldblib.c"
  "liblua-5.1.5/src/ldebug.c"
  "liblua-5.1.5/src/ldo.c"
  "liblua-5.1.5/src/ldump.c"
  "liblua-5.1.5/src/lfunc.c"
  "liblua-5.1.5/src/lgc.c"
  "liblua-5.1.5/src/linit.c"
  "liblua-5.1.5/src/liolib.c"
  "liblua-5.1.5/src/llex.c"
  "liblua-5.1.5/src/lmathlib.c"
  "liblua-5.1.5/src/lmem.c"
  "liblua-5.1.5/src/loadlib.c"
  "liblua-5.1.5/src/lobject.c"
  "liblua-5.1.5/src/lopcodes.c"
  "liblua-5.1.5/src/loslib.c"
  "liblua-5.1.5/src/lparser.c"
  "liblua-5.1.5/src/lstate.c"
  "liblua-5.1.5/src/lstring.c"
  "liblua-5.1.5/src/lstrlib.c"
  "liblua-5.1.5/src/ltable.c"
  "liblua-5.1.5/src/ltablib.c"
  "liblua-5.1.5/src/ltm.c"
  "liblua-5.1.5/src/lundump.c"
  "liblua-5.1.5/src/lvm.c"
  "liblua-5.1.5/src/lzio.c"
)

set(LUA_HPP
  "liblua-5.1.5/src/lapi.h"
  "liblua-5.1.5/include/lauxlib.h"
  "liblua-5.1.5/src/lcode.h"
  "liblua-5.1.5/src/ldebug.h"
  "liblua-5.1.5/src/ldo.h"
  "liblua-5.1.5/src/lfunc.h"
  "liblua-5.1.5/src/lgc.h"
  "liblua-5.1.5/src/llex.h"
  "liblua-5.1.5/src/llimits.h"
  "liblua-5.1.5/src/lmem.h"
  "liblua-5.1.5/src/lobject.h"
  "liblua-5.1.5/src/lopcodes.h"
  "liblua-5.1.5/src/lparser.h"
  "liblua-5.1.5/src/lstate.h"
  "liblua-5.1.5/src/lstring.h"
  "liblua-5.1.5/src/ltable.h"
  "liblua-5.1.5/src/ltm.h"
  "liblua-5.1.5/include/lua.h"
  "liblua-5.1.5/include/luaconf.h"
  "liblua-5.1.5/include/lualib.h"
  "liblua-5.1.5/src/lundump.h"
  "liblua-5.1.5/src/lvm.h"
  "liblua-5.1.5/src/lzio.h"
)

source_group("Source Files" FILES ${LUA_SRC})
source_group("Header Files" FILES ${LUA_HPP})

add_library("lua-5.1" ${LUA_SRC} ${LUA_HPP})

set_property(TARGET "lua-5.1" PROPERTY FOLDER "External Libraries")

# include_directories(src)

if(MSVC)
  sm_add_compile_definition("lua-5.1" _CRT_SECURE_NO_WARNINGS)
endif(MSVC)

disable_project_warnings("lua-5.1")

target_include_directories("lua-5.1" PUBLIC "liblua-5.1.5/include")
