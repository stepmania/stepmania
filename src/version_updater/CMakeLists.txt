source_group("" FILES Main.cpp)

configure_file(
  "version.hpp.in"
  "${PROJECT_BINARY_DIR}/version.hpp"
)

include_directories(${PROJECT_BINARY_DIR})

add_executable("VersionUpdater" Main.cpp "${PROJECT_BINARY_DIR}/version.hpp")

set_property(TARGET "VersionUpdater" PROPERTY FOLDER "Internal Libraries")

if(MSVC)
  sm_add_compile_definition("VersionUpdater" _CRT_SECURE_NO_WARNINGS)
endif()

