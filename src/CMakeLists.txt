# TODO: Move libtom* to extern
add_subdirectory(libtomcrypt)
add_subdirectory(libtommath)


# First part is for the version incrementer tool.
add_subdirectory(libversionupdater)

# Run the executable above.
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
	COMMAND VersionUpdater ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
	DEPENDS VersionUpdater
)

# Include this folder itself.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Add the main library.

source_group("Generated Files" FILES ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp)

add_library("StepMania-Data" ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp)

set_property(TARGET "StepMania-Data" PROPERTY FOLDER "Internal Libraries")