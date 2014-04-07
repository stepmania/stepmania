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
include_directories(
	"."
	"../extern/ffmpeg/include"
	"../extern/glew-1.5.8/include"
	"../extern/jsoncpp/include"
	"../extern/libjpeg"
	"../extern/vorbis"
	"../extern/zlib"
)

if(WIN32)
	include_directories(
		${DIRECTX_INCLUDE_DIR}
	)
endif(WIN32)

# Add the main library.

list(APPEND SMDATA_GLOBAL_FILES_SRC
	"GameLoop.cpp"
	"global.cpp"
	"SpecialFiles.cpp"
	# StepMania will be refactored to not have its main.
)

list(APPEND SMDATA_GLOBAL_FILES_HPP
	"GameLoop.h"
	"global.h"
	"ProductInfo.h" # At some point, have some of this file be generated perhaps.
	"SpecialFiles.h"
	"StdString.h" # At some point, remove the need for this file. We don't support Visual Studio 6.
	# StepMania will be refactored to not have its main.
)

source_group("Global Files" FILES ${SMDATA_GLOBAL_FILES_SRC} ${SMDATA_GLOBAL_FILES_HPP})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include(SMMakeArch)
include(SMMakeRage)
include(SMMakeActor)

list(APPEND SMDATA_GLOBAL_SINGLETON_SRC
	"AnnouncerManager.cpp"
	"Bookkeeper.cpp"
	"CharacterManager.cpp"
	"CommandLineActions.cpp"
	"CryptManager.cpp"
	"ezsockets.cpp"
	"FontManager.cpp"
	"GameManager.cpp"
	"GameSoundManager.cpp"
	"GameState.cpp"
	"InputFilter.cpp"
	"InputMapper.cpp"
	"InputQueue.cpp"
	"LightsManager.cpp"
	"LuaManager.cpp"
	"MemoryCardManager.cpp"
	"MessageManager.cpp"
	"NetworkSyncManager.cpp"
	"NoteSkinManager.cpp"
	"PrefsManager.cpp"
	"ProfileManager.cpp"
	"ScreenManager.cpp"
	"SongManager.cpp"
	"StatsManager.cpp"
	"ThemeManager.cpp"
	"UnlockManager.cpp"
)

list(APPEND SMDATA_GLOBAL_SINGLETON_HPP
	"AnnouncerManager.h"
	"Bookkeeper.h"
	"CharacterManager.h"
	"CommandLineActions.h"
	"CryptManager.h"
	"ezsockets.h"
	"FontManager.h"
	"GameManager.h"
	"GameSoundManager.h"
	"GameState.h"
	"InputFilter.h"
	"InputMapper.h"
	"InputQueue.h"
	"LightsManager.h"
	"LuaManager.h"
	"MemoryCardManager.h"
	"MessageManager.h"
	"NetworkSyncManager.h"
	"NoteSkinManager.h"
	"PrefsManager.h"
	"ProfileManager.h"
	"ScreenManager.h"
	"SongManager.h"
	"StatsManager.h"
	"ThemeManager.h"
	"UnlockManager.h"
)

source_group("Global Singletons" FILES ${SMDATA_GLOBAL_SINGLETON_SRC} ${SMDATA_GLOBAL_SINGLETON_HPP})

source_group("Generated Files" FILES ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp)

list(APPEND SMDATA_ALL_FILES_SRC
	${SMDATA_GLOBAL_FILES_SRC}
	${SMDATA_GLOBAL_SINGLETON_SRC}
	${SMDATA_ALL_ARCH_SRC}
	${SMDATA_ALL_RAGE_SRC}
	${SMDATA_ALL_ACTORS_SRC}
	${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
)

list(APPEND SMDATA_ALL_FILES_HPP
	${SMDATA_GLOBAL_SINGLETON_HPP}
	${SMDATA_ALL_ARCH_HPP}
	${SMDATA_ALL_RAGE_HPP}
	${SMDATA_ALL_ACTORS_HPP}
	${SMDATA_GLOBAL_FILES_HPP}
)

if (WIN32)
	# TODO: Combine these two definitions into one.
	add_definitions(/D WINDOWS)
	add_definitions(/D _WINDOWS)
endif(WIN32)

add_library("StepMania-Data" ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})

target_link_libraries("StepMania-Data"
	"glew"
	"lua-5.1"
	"pcre"
	"png"
	"tomcrypt"
	"tommath"
)

if (WIN32)
	target_link_libraries("StepMania-Data" ${DIRECTX_LIBRARY_DIR})
endif(WIN32)

set_property(TARGET "StepMania-Data" PROPERTY FOLDER "Internal Libraries")
