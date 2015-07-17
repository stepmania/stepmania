set(LUA_DIR "liblua-5.1.5")

set(LUA_SRC
  "${LUA_DIR}/src/lapi.c"
  "${LUA_DIR}/src/lauxlib.c"
  "${LUA_DIR}/src/lbaselib.c"
  "${LUA_DIR}/src/lcode.c"
  "${LUA_DIR}/src/ldblib.c"
  "${LUA_DIR}/src/ldebug.c"
  "${LUA_DIR}/src/ldo.c"
  "${LUA_DIR}/src/ldump.c"
  "${LUA_DIR}/src/lfunc.c"
  "${LUA_DIR}/src/lgc.c"
  "${LUA_DIR}/src/linit.c"
  "${LUA_DIR}/src/liolib.c"
  "${LUA_DIR}/src/llex.c"
  "${LUA_DIR}/src/lmathlib.c"
  "${LUA_DIR}/src/lmem.c"
  "${LUA_DIR}/src/loadlib.c"
  "${LUA_DIR}/src/lobject.c"
  "${LUA_DIR}/src/lopcodes.c"
  "${LUA_DIR}/src/loslib.c"
  "${LUA_DIR}/src/lparser.c"
  "${LUA_DIR}/src/lstate.c"
  "${LUA_DIR}/src/lstring.c"
  "${LUA_DIR}/src/lstrlib.c"
  "${LUA_DIR}/src/ltable.c"
  "${LUA_DIR}/src/ltablib.c"
  "${LUA_DIR}/src/ltm.c"
  "${LUA_DIR}/src/lundump.c"
  "${LUA_DIR}/src/lvm.c"
  "${LUA_DIR}/src/lzio.c"
)

set(LUA_HPP
  "${LUA_DIR}/src/lapi.h"
  "${LUA_DIR}/include/lauxlib.h"
  "${LUA_DIR}/src/lcode.h"
  "${LUA_DIR}/src/ldebug.h"
  "${LUA_DIR}/src/ldo.h"
  "${LUA_DIR}/src/lfunc.h"
  "${LUA_DIR}/src/lgc.h"
  "${LUA_DIR}/src/llex.h"
  "${LUA_DIR}/src/llimits.h"
  "${LUA_DIR}/src/lmem.h"
  "${LUA_DIR}/src/lobject.h"
  "${LUA_DIR}/src/lopcodes.h"
  "${LUA_DIR}/src/lparser.h"
  "${LUA_DIR}/src/lstate.h"
  "${LUA_DIR}/src/lstring.h"
  "${LUA_DIR}/src/ltable.h"
  "${LUA_DIR}/src/ltm.h"
  "${LUA_DIR}/include/lua.h"
  "${LUA_DIR}/include/luaconf.h"
  "${LUA_DIR}/include/lualib.h"
  "${LUA_DIR}/src/lundump.h"
  "${LUA_DIR}/src/lvm.h"
  "${LUA_DIR}/src/lzio.h"
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

target_include_directories("lua-5.1" PUBLIC "${LUA_DIR}/include")
