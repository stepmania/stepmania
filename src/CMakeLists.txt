# Version Updater library.
add_subdirectory(version_updater)

# This part may move to a different file: it's the SM Data project.
add_custom_command(
  OUTPUT ${SM_SRC_DIR}/verstub.cpp
  COMMAND VersionUpdater ${SM_SRC_DIR}/verstub.cpp
  DEPENDS VersionUpdater
)

# Add dependencies needed here.
if(APPLE)

elseif(MSVC)

elseif(WIN32)

else() # Unix/Linux
  find_package(Threads)
  if (${Threads_FOUND})
    set(HAS_PTHREAD TRUE)
  else()
    set(HAS_PTHREAD FALSE)
  endif()

  include("${SM_CMAKE_DIR}/CMakeScout_FFMPEG.txt")
  if (${FFMPEG_FOUND})
    set(HAS_FFMPEG TRUE)
  else()
    set(HAS_FFMPEG FALSE)
  endif()

  find_package(OpenGL REQUIRED)
  if (NOT ${OPENGL_FOUND})
    message(FATAL_ERROR "OpenGL required to compile StepMania.")
  endif()

  find_package(GLEW REQUIRED)
  if (NOT ${GLEW_FOUND})
    message(FATAL_ERROR "GLEW required to compile StepMania.")
  endif()

  find_package(GTK2 2.0 REQUIRED gtk)
endif()

source_group("Generated Files" FILES ${SM_SRC_DIR}/verstub.cpp)

# Main project is below.

list(APPEND SMDATA_GLOBAL_FILES_SRC
  "GameLoop.cpp"
  "global.cpp"
  "SpecialFiles.cpp"
  "StepMania.cpp" # TODO: Refactor into separate main project.
)

list(APPEND SMDATA_GLOBAL_FILES_HPP
  "GameLoop.h"
  "global.h"
  "ProductInfo.h" # TODO: Have this be auto-generated.
  "SpecialFiles.h"
  "StdString.h" # TODO: Remove the need for this file, transition to std::string.
  "StepMania.h" # TODO: Refactor into separate main project.
)

source_group("Global Files" FILES ${SMDATA_GLOBAL_FILES_SRC} ${SMDATA_GLOBAL_FILES_HPP})

# Keep the module path local for easier grabbing.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

include(CMakeData-arch.txt)
include(CMakeData-rage.txt)
include(CMakeData-os.txt)
include(CMakeData-actor.txt)
include(CMakeData-screen.txt)
include(CMakeData-data.txt)

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

list(APPEND SMDATA_ALL_FILES_SRC
  ${SM_SRC_DIR}/verstub.cpp
  ${SMDATA_GLOBAL_FILES_SRC}
  ${SMDATA_GLOBAL_SINGLETON_SRC}
  ${SMDATA_ALL_ACTORS_SRC}
  ${SMDATA_ALL_ARCH_SRC}
  ${SMDATA_ALL_DATA_SRC}
  ${SMDATA_ALL_RAGE_SRC}
  ${SMDATA_ALL_SCREENS_SRC}
  ${SMDATA_OS_SRC}
  ${SMDATA_FILE_TYPES_SRC}
)
list(APPEND SMDATA_ALL_FILES_HPP
  ${SMDATA_GLOBAL_FILES_HPP}
  ${SMDATA_GLOBAL_SINGLETON_HPP}
  ${SMDATA_ALL_ACTORS_HPP}
  ${SMDATA_ALL_ARCH_HPP}
  ${SMDATA_ALL_DATA_HPP}
  ${SMDATA_ALL_RAGE_HPP}
  ${SMDATA_ALL_SCREENS_HPP}
  ${SMDATA_OS_HPP}
  ${SMDATA_FILE_TYPES_HPP}
)

if(NOT APPLE)
  list(APPEND SMDATA_ALL_FILES_SRC "Main.cpp")
  source_group("" FILES "Main.cpp")
endif()

# TODO: Make this actually be data and not an executable.
if(APPLE)
  set(MACOSX_BUNDLE_BUNDLE_NAME "StepMania")
  add_executable("StepMania" MACOSX_BUNDLE ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})
  set_target_properties("StepMania" PROPERTIES MACOSX_BUNDLE TRUE)
else()
  add_executable("StepMania" ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})
endif()

sm_add_compile_definition("StepMania" CMAKE_POWERED)

set_target_properties("StepMania" PROPERTIES OUTPUT_NAME_DEBUG "StepMania-debug")
set_target_properties("StepMania" PROPERTIES OUTPUT_NAME_MINSIZEREL "StepMania-min-size")
set_target_properties("StepMania" PROPERTIES OUTPUT_NAME_RELWITHDEBINFO "StepMania-release-symbols")

if(WIN32)
  sm_add_compile_definition("StepMania" WINDOWS)
  sm_add_compile_definition("StepMania" _WINDOWS) # TODO: Remove this potential duplicate.
  sm_add_compile_definition("StepMania" _CRT_SECURE_NO_WARNINGS)
  sm_add_compile_definition("StepMania" _WINSOCK_DEPRECATED_NO_WARNINGS)
  sm_add_compile_definition("StepMania" GLEW_STATIC)
  sm_add_compile_flag("StepMania" "/MP")
  
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SM_PROGRAM_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_PROGRAM_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_PROGRAM_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_PROGRAM_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_PROGRAM_DIR}")
elseif(APPLE)
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SM_ROOT_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_ROOT_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_ROOT_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_ROOT_DIR}")
  set_target_properties("StepMania" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_ROOT_DIR}")
  
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE
    "${SM_XCODE_DIR}/Info-StepMania.plist"
  )
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREFIX_HEADER "${SM_SRC_DIR}/ProductInfo.h")
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS
    "EXECUTABLE_NAME=\$(PRODUCT_NAME)"
  )

  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/archutils/Darwin/StepMania.pch")
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_LIBRARY_SEARCH_PATHS
    "${SM_XCODE_DIR}/ffmpeg/lib ${SM_XCODE_DIR}/Libraries"
  )
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-lbz2")
  sm_add_compile_definition("StepMania" _XOPEN_SOURCE)
  
  # Add the ability to copy the resource file.
  add_custom_command(TARGET "StepMania"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:StepMania>/../Resources"
  )
  add_custom_command(TARGET "StepMania"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SM_XCODE_DIR}/smicon.icns" "$<TARGET_FILE_DIR:StepMania>/../Resources/"
  )
  add_custom_command(TARGET "StepMania"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SM_XCODE_DIR}/Hardware.plist" "$<TARGET_FILE_DIR:StepMania>/../Resources/"
  )
else() # Linux
  if (${HAS_PTHREAD})
    sm_add_compile_definition("StepMania" HAVE_LIBPTHREAD)
  endif()
  if (${HAS_FFMPEG})
    sm_add_compile_definition("StepMania" HAVE_FFMPEG)
  endif()
  include(TestBigEndian)
  test_big_endian(BIGENDIAN)
  if (${BIGENDIAN})
    sm_add_compile_definition("StepMania" ENDIAN_BIG)
  else()
    sm_add_compile_definition("StepMania" ENDIAN_LITTLE)
  endif()
  sm_add_compile_definition("StepMania" UNIX)
  if("${CMAKE_SYSTEM}" MATCHES "Linux")
    sm_add_compile_definition("StepMania" LINUX)
  endif()
endif()

set_property(TARGET "StepMania" PROPERTY FOLDER "Internal Libraries")

list(APPEND SMDATA_LINK_LIB
  "lua-5.1"
  "tomcrypt"
  "tommath"
  "zlib"
  "jsoncpp"
  "mad"
  "png"
  "glew"
  "pcre"
  "jpeg"
)

if (WIN32)

  # FFMPEG...it can be evil.
  find_library(LIB_SWSCALE NAMES "swscale"
    PATHS "${SM_EXTERN_DIR}/ffmpeg/lib" NO_DEFAULT_PATH
  )
  get_filename_component(LIB_SWSCALE ${LIB_SWSCALE} NAME)
  
  find_library(LIB_AVCODEC NAMES "avcodec"
    PATHS "${SM_EXTERN_DIR}/ffmpeg/lib" NO_DEFAULT_PATH
  )
  get_filename_component(LIB_AVCODEC ${LIB_AVCODEC} NAME)
  
  find_library(LIB_AVFORMAT NAMES "avformat"
    PATHS "${SM_EXTERN_DIR}/ffmpeg/lib" NO_DEFAULT_PATH
  )
  get_filename_component(LIB_AVFORMAT ${LIB_AVFORMAT} NAME)
  
  find_library(LIB_AVUTIL NAMES "avutil"
    PATHS "${SM_EXTERN_DIR}/ffmpeg/lib" NO_DEFAULT_PATH
  )
  get_filename_component(LIB_AVUTIL ${LIB_AVUTIL} NAME)

  set(SMDATA_LINK_LIB
    "lua-5.1"
    "tomcrypt"
    "tommath"
    "zlib"
    "jsoncpp"
    "png"
    "glew"
    "pcre"
	# The misc libraries are here.
	"${LIB_SWSCALE}"
	"${LIB_AVCODEC}"
	"${LIB_AVFORMAT}"
	"${LIB_AVUTIL}"
	"libmad.lib"
	"ogg_static.lib"
	"vorbis_static.lib"
	"vorbisfile_static.lib"
	"dbghelp.lib"
	"setupapi.lib"
	"hid.lib"
  )
  
  sm_add_link_flag("StepMania" "/LIBPATH:\"${DIRECTX_LIBRARY_DIR}\"")
  sm_add_link_flag("StepMania" "/LIBPATH:\"${SM_EXTERN_DIR}/ffmpeg/lib\"")
  sm_add_link_flag("StepMania" "/LIBPATH:\"${SM_EXTERN_DIR}/libjpeg\"")
  sm_add_link_flag("StepMania" "/LIBPATH:\"${SM_EXTERN_DIR}/mad-0.15.1b/msvc++/Release\"")
  sm_add_link_flag("StepMania" "/LIBPATH:\"${SM_EXTERN_DIR}/vorbis/win32\"")
  sm_add_link_flag("StepMania" "/LIBPATH:\"${SM_SRC_DIR}/archutils/Win32/ddk\"")
  sm_add_link_flag("StepMania" "/ERRORREPORT:SEND")
  sm_add_link_flag("StepMania" "/MAPINFO:EXPORTS")
  sm_add_link_flag("StepMania" "/SAFESEH:NO")
  sm_add_link_flag("StepMania" "/NOLOGO")
  sm_add_link_flag("StepMania" "/MAP")
  
elseif(APPLE)
  find_library(MAC_FRAME_ACCELERATE Accelerate ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_APPKIT AppKit ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_AUDIOTOOLBOX AudioToolbox ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_AUDIOUNIT AudioUnit ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_CARBON Carbon ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_COCOA Cocoa ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_COREAUDIO CoreAudio ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_COREFOUNDATION CoreFoundation ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_CORESERVICES CoreServices ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_FOUNDATION Foundation ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_IOKIT IOKit ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_OPENGL OpenGL ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  find_library(MAC_FRAME_QUICKTIME QuickTime ${CMAKE_SYSTEM_FRAMEWORK_PATH})
  
  set(MAC_VECLIB_DIR "${MAC_FRAME_ACCELERATE}/Versions/A/Frameworks")
  set_target_properties("StepMania" PROPERTIES XCODE_ATTRIBUTE_FRAMEWORK_SEARCH_PATHS
    "${MAC_VECLIB_DIR}"
  )
  
  find_library(MAC_FRAME_VECLIB vecLib "${MAC_VECLIB_DIR}")
  
  mark_as_advanced(
    MAC_FRAME_ACCELERATE
    MAC_FRAME_APPKIT
    MAC_FRAME_AUDIOTOOLBOX
    MAC_FRAME_AUDIOUNIT
    MAC_FRAME_CARBON
    MAC_FRAME_COCOA
    MAC_FRAME_COREAUDIO
    MAC_FRAME_COREFOUNDATION
    MAC_FRAME_CORESERVICES
    MAC_FRAME_FOUNDATION
    MAC_FRAME_IOKIT
    MAC_FRAME_OPENGL
    MAC_FRAME_QUICKTIME
    MAC_FRAME_VECLIB
  )
  
# The following were removed from SMDATA_LINK_LIB to "match" StepMania's pbxproj.
#    ${MAC_FRAME_APPKIT}
#    ${MAC_FRAME_FOUNDATION}
#    "ffmpeg"
  
  
  # TODO: Adjust the target when StepMania-Data returns.
  # TODO: Find a cleaner way besides just setting to wipe the slate clean.
  set(SMDATA_LINK_LIB
    ${MAC_FRAME_ACCELERATE}
    ${MAC_FRAME_CARBON}
    ${MAC_FRAME_COCOA}
    ${MAC_FRAME_IOKIT}
    ${MAC_FRAME_QUICKTIME}
    ${MAC_FRAME_OPENGL}
    ${MAC_FRAME_COREFOUNDATION}
    ${MAC_FRAME_AUDIOTOOLBOX}
    ${MAC_FRAME_AUDIOUNIT}
    ${MAC_FRAME_COREAUDIO}
    ${MAC_FRAME_CORESERVICES}
    ${MAC_FRAME_VECLIB}
    "ogg"
    "theora"
    "vorbis"
    "lua-5.1"
    "tomcrypt"
    "tommath"
    "zlib"
    "jsoncpp"
    "mad"
    "png"
    "glew"
    "pcre"
    "jpeg"
    "avcodec"
    "avdevice"
    "avfilter"
    "avformat"
    "avutil"
    "swresample"
    "swscale"
  )
else() # Unix / Linux
  list(APPEND SMDATA_LINK_LIB
    ${OPENGL_LIBRARY}
    ${GTK2_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
  )
  message(STATUS "${CMAKE_THREAD_LIBS_INIT}")
endif()

target_link_libraries("StepMania" ${SMDATA_LINK_LIB})

list(APPEND SM_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  "${SM_EXTERN_DIR}/vorbis"
)
if(NOT APPLE)
  list(APPEND SM_INCLUDE_DIRS
    "${SM_EXTERN_DIR}/glew-1.5.8/include"
    "${SM_EXTERN_DIR}/jsoncpp/include"
    "${SM_EXTERN_DIR}/libjpeg"
    "${SM_EXTERN_DIR}/zlib"
  )
  if(MSVC)
    list(APPEND SM_INCLUDE_DIRS
      "${SM_EXTERN_DIR}/ffmpeg/include"
    )
  else()
    list(APPEND SM_INCLUDE_DIRS
      "${SM_BUNDLE_DIR}/ffmpeg"
    )
  endif()
else()
  list(APPEND SM_INCLUDE_DIRS
    "${SM_XCODE_DIR}/ffmpeg/include"
    "archutils/Unix"
  )
endif()

if(WIN32)
  list(APPEND SM_INCLUDE_DIRS
    ${DIRECTX_INCLUDE_DIR}
  )
else()
  list(APPEND SM_INCLUDE_DIRS
    "${SM_EXTERN_DIR}/libpng/include"
  )
endif()

target_include_directories("StepMania" PUBLIC ${SM_INCLUDE_DIRS})
