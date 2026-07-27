set(PCRE_SRC "pcre/pcre_byte_order.c"
             "pcre/pcre_chartables.c"
             "pcre/pcre_compile.c"
             "pcre/pcre_config.c"
             "pcre/pcre_dfa_exec.c"
             "pcre/pcre_exec.c"
             "pcre/pcre_fullinfo.c"
             "pcre/pcre_get.c"
             "pcre/pcre_globals.c"
             "pcre/pcre_jit_compile.c"
             "pcre/pcre_maketables.c"
             "pcre/pcre_newline.c"
             "pcre/pcre_ord2utf8.c"
             "pcre/pcre_refcount.c"
             "pcre/pcre_string_utils.c"
             "pcre/pcre_study.c"
             "pcre/pcre_tables.c"
             "pcre/pcre_ucd.c"
             "pcre/pcre_valid_utf8.c"
             "pcre/pcre_version.c"
             "pcre/pcre_xclass.c")

set(PCRE_HPP "pcre/pcre_internal.h"
             "pcre/pcre.h"
             "pcre/ucp.h")

configure_file("config.pcre.in.h" "pcre/config.h" COPYONLY)
configure_file("pcre/pcre.h.generic" "pcre/pcre.h" COPYONLY)
configure_file("pcre/pcre_chartables.c.dist" "pcre/pcre_chartables.c" COPYONLY)

source_group("" FILES ${PCRE_SRC} ${PCRE_HPP})

add_library("pcre" STATIC ${PCRE_SRC} ${PCRE_HPP})

set_property(TARGET "pcre" PROPERTY FOLDER "External Libraries")

disable_project_warnings("pcre")

target_compile_definitions("pcre" PRIVATE HAVE_CONFIG_H)
target_compile_definitions("pcre" PUBLIC PCRE_STATIC)

target_include_directories("pcre" PUBLIC
  "pcre"
  "${CMAKE_CURRENT_BINARY_DIR}/pcre"
)
