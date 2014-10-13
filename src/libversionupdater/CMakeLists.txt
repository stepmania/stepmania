# First part is for the version incrementer tool.

if(MSVC)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("VersionUpdater")
endif(MSVC)

source_group("" FILES versionupdater.cpp)

add_executable("VersionUpdater" versionupdater.cpp)

set_property(TARGET "VersionUpdater" PROPERTY FOLDER "Internal Libraries")