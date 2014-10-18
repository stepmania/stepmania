source_group("" FILES Main.cpp)

add_executable("VersionUpdater" Main.cpp)

set_property(TARGET "VersionUpdater" PROPERTY FOLDER "Internal Libraries")

if(MSVC)
  sm_add_compile_definition("VersionUpdater" _CRT_SECURE_NO_WARNINGS)
endif()

