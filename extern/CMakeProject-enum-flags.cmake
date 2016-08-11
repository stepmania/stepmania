set(ENUM_FLAGS_DIR "${SM_EXTERN_DIR}/enum-flags")

if (NOT IS_DIRECTORY ${ENUM_FLAGS_DIR})
  message(ERROR "Submodule for enum-flags missing. Run git submodule init && git submodule update first.")
  return()
endif()

list(APPEND ENUM_FLAGS_HPP
  "${ENUM_FLAGS_DIR}/enum_flags.h"
)

source_group("" FILES ${ENUM_FLAGS_HPP})

add_library("enum-flags" INTERFACE)
target_sources("enum-flags" INTERFACE ${ENUM_FLAGS_HPP})

#set_property(TARGET "enum-flags" PROPERTY FOLDER "External Libraries")
