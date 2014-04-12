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

if(WIN32 OR APPLE)
	include_directories("../extern/mad-0.15.1b")
endif(WIN32 OR APPLE)

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
	"StepMania.cpp"	# StepMania will be refactored to not have its main.
)

list(APPEND SMDATA_GLOBAL_FILES_HPP
	"GameLoop.h"
	"global.h"
	"ProductInfo.h" # At some point, have some of this file be generated perhaps.
	"SpecialFiles.h"
	"StdString.h" # At some point, remove the need for this file. We don't support Visual Studio 6.
	"StepMania.h" # StepMania will be refactored to not have its main.
)

source_group("Global Files" FILES ${SMDATA_GLOBAL_FILES_SRC} ${SMDATA_GLOBAL_FILES_HPP})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if(WIN32)
	add_definitions(/MP)
endif(WIN32)

include(SMMakeArch)
include(SMMakeRage)
include(SMMakeActor)
include(SMMakeScreen)
include(SMMakeData)

if(WIN32)
	include(SMMakeOSWin)
endif(WIN32)

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

list(APPEND SMDATA_FILE_TYPES_SRC
	"IniFile.cpp"
	"MsdFile.cpp"
	"XmlFile.cpp"
	"XmlFileUtil.cpp"
)
list(APPEND SMDATA_FILE_TYPES_HPP
	"IniFile.h"
	"MsdFile.h"
	"XmlFile.h"
	"XmlFileUtil.h"
)

source_group("File Types" FILES ${SMDATA_FILE_TYPES_SRC} ${SMDATA_FILE_TYPES_HPP})

source_group("Generated Files" FILES ${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp)

list(APPEND SMDATA_ALL_FILES_SRC
	${SMDATA_GLOBAL_FILES_SRC}
	${SMDATA_GLOBAL_SINGLETON_SRC}
	${SMDATA_ALL_ARCH_SRC}
	${SMDATA_ALL_RAGE_SRC}
	${SMDATA_ALL_ACTORS_SRC}
	${SMDATA_ALL_SCREENS_SRC}
	${SMDATA_ALL_DATA_SRC}
	${SMDATA_FILE_TYPES_SRC}
	${SMDATA_OS_SRC}
	${CMAKE_CURRENT_BINARY_DIR}/verdata.cpp
)

if (NOT APPLE)
	list(APPEND SMDATA_ALL_FILES_SRC "Main.cpp")
	source_group("" FILES "Main.cpp")
endif(NOT APPLE)

list(APPEND SMDATA_ALL_FILES_HPP
	${SMDATA_GLOBAL_SINGLETON_HPP}
	${SMDATA_ALL_ARCH_HPP}
	${SMDATA_ALL_RAGE_HPP}
	${SMDATA_ALL_ACTORS_HPP}
	${SMDATA_ALL_SCREENS_HPP}
	${SMDATA_ALL_DATA_HPP}
	${SMDATA_FILE_TYPES_HPP}
	${SMDATA_OS_HPP}
	${SMDATA_GLOBAL_FILES_HPP}
)

if (MSVC)
	# TODO: Combine these two definitions into one.
	add_definitions(/D WINDOWS)
	add_definitions(/D _WINDOWS)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
endif(MSVC)

add_executable("StepMania" ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})

list(APPEND SM_LINK_LIBRARIES
	"glew"
	"lua-5.1"
	"pcre"
	"png"
	"tomcrypt"
	"tommath"
	"jsoncpp"
)

if(MSVC)
	find_library(LIB_SWSCALE NAMES "swscale" 
		PATHS ${CMAKE_SOURCE_DIR}/extern/ffmpeg/lib NO_DEFAULT_PATH)
	get_filename_component(LIB_SWSCALE ${LIB_SWSCALE} NAME)
	list(APPEND SM_LINK_LIBRARIES ${LIB_SWSCALE})

	find_library(LIB_AVCODEC NAMES "avcodec" 
		PATHS ${CMAKE_SOURCE_DIR}/extern/ffmpeg/lib NO_DEFAULT_PATH)
	get_filename_component(LIB_AVCODEC ${LIB_AVCODEC} NAME)
	list(APPEND SM_LINK_LIBRARIES ${LIB_AVCODEC})

	find_library(LIB_AVFORMAT NAMES "avformat" 
		PATHS ${CMAKE_SOURCE_DIR}/extern/ffmpeg/lib NO_DEFAULT_PATH)
	get_filename_component(LIB_AVFORMAT ${LIB_AVFORMAT} NAME)
	list(APPEND SM_LINK_LIBRARIES ${LIB_AVFORMAT})

	find_library(LIB_AVUTIL NAMES "avutil" 
		PATHS ${CMAKE_SOURCE_DIR}/extern/ffmpeg/lib NO_DEFAULT_PATH)
	get_filename_component(LIB_AVUTIL ${LIB_AVUTIL} NAME)
	list(APPEND SM_LINK_LIBRARIES ${LIB_AVUTIL})

	list(APPEND SM_LINK_LIBRARIES "libmad.lib")

	list(APPEND SM_LINK_LIBRARIES
		"ogg_static.lib"
		"vorbis_static.lib"
		"vorbisfile_static.lib"

		"dbghelp.lib"
		"setupapi.lib"
		"hid.lib"
	)

endif(MSVC)

target_link_libraries("StepMania"
	${SM_LINK_LIBRARIES}
)

if(WIN32)
	my_add_link_flags("StepMania" "/LIBPATH:\"${DIRECTX_LIBRARY_DIR}\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/libpng/lib\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/ffmpeg/lib\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/libjpeg\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/zlib\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/mad-0.15.1b/msvc++/Release\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/extern/vorbis/win32\"")
	my_add_link_flags("StepMania" "/LIBPATH:\"${CMAKE_SOURCE_DIR}/archutils/Win32/ddk\"")
	my_add_link_flags("StepMania" "/ERRORREPORT:SEND")
	my_add_link_flags("StepMania" "/MAPINFO:EXPORTS")
	my_add_link_flags("StepMania" "/SAFESEH:NO")
	my_add_link_flags("StepMania" "/NOLOGO")
	my_add_link_flags("StepMania" "/MAP")
	my_add_link_flags("StepMania" "/SUBSYSTEM:WINDOWS")
endif(WIN32)

set_property(TARGET "StepMania" PROPERTY FOLDER "Executables")
