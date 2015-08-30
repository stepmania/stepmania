set(PCRE_DIR "pcre-8.37")

set(PCRE_SRC
  "${PCRE_DIR}/src/pcre_chartables.c"
  "${PCRE_DIR}/src/pcre_compile.c"
  "${PCRE_DIR}/src/pcre_exec.c"
  "${PCRE_DIR}/src/pcre_fullinfo.c"
  "${PCRE_DIR}/src/pcre_globals.c"
  "${PCRE_DIR}/src/pcre_newline.c"
  "${PCRE_DIR}/src/pcre_tables.c"
)

set(PCRE_HPP
  "${PCRE_DIR}/include/pcre.h"
  "${PCRE_DIR}/src/pcre_internal.h"
  "${PCRE_DIR}/src/ucp.h"
)

source_group("Source Files" FILES ${PCRE_SRC})
source_group("Header Files" FILES ${PCRE_HPP})

add_library("pcre" ${PCRE_SRC} ${PCRE_HPP})

set_property(TARGET "pcre" PROPERTY FOLDER "External Libraries")

disable_project_warnings("pcre")

sm_add_compile_definition("pcre" PCRE_STATIC)
sm_add_compile_definition("pcre" NEWLINE=10)
sm_add_compile_definition("pcre" LINK_SIZE=2)
sm_add_compile_definition("pcre" PARENS_NEST_LIMIT=250)
sm_add_compile_definition("pcre" MATCH_LIMIT=10000000)
sm_add_compile_definition("pcre" MATCH_LIMIT_RECURSION=MATCH_LIMIT)
sm_add_compile_definition("pcre" MAX_NAME_SIZE=32)
sm_add_compile_definition("pcre" MAX_NAME_COUNT=10000)

target_include_directories("pcre" PUBLIC "${PCRE_DIR}/include")
