# Version Updater library.
add_subdirectory(version_updater)

# This part may move to a different file: it's the SM Data project.
add_custom_command(
  OUTPUT ${SM_SRC_DIR}/verstub.cpp
  COMMAND VersionUpdater ${SM_SRC_DIR}/verstub.cpp
  DEPENDS VersionUpdater
)

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

include(CMakeData-arch.cmake)
include(CMakeData-rage.cmake)
include(CMakeData-os.cmake)
include(CMakeData-actor.cmake)
include(CMakeData-screen.cmake)
include(CMakeData-data.cmake)
include(CMakeData-gtk.cmake)

list(APPEND SMDATA_GLOBAL_SINGLETON_SRC
  "AnnouncerManager.cpp"
  "Bookkeeper.cpp"
  "CharacterManager.cpp"
  "CommandLineActions.cpp"
  "CryptManager.cpp"
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

if(WITH_NETWORKING)
  list(APPEND SMDATA_GLOBAL_SINGLETON_SRC
    "ezsockets.cpp"
  )
  list(APPEND SMDATA_GLOBAL_SINGLETON_HPP
    "ezsockets.h"
  )
endif()

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

# TODO: Reconsile the OS dependent naming scheme.
if (WIN32 OR APPLE)
  set(SM_EXE_NAME "StepMania")
else()
  set(SM_EXE_NAME "stepmania")
endif()

# TODO: Make this actually be data and not an executable.
if(APPLE)
  set(MACOSX_BUNDLE_BUNDLE_NAME ${SM_EXE_NAME})
  add_executable("${SM_EXE_NAME}" MACOSX_BUNDLE ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})
  set_target_properties("${SM_EXE_NAME}" PROPERTIES MACOSX_BUNDLE TRUE)
else()
  add_executable("${SM_EXE_NAME}" ${SMDATA_ALL_FILES_SRC} ${SMDATA_ALL_FILES_HPP})
endif()

sm_add_compile_definition("${SM_EXE_NAME}" CMAKE_POWERED)

if (NOT WITH_NETWORKING)
  sm_add_compile_definition("${SM_EXE_NAME}" WITHOUT_NETWORKING)
endif()

set(SM_COMPILE_FLAGS "")
# Compilation flags per project here.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  sm_add_compile_definition("${SM_EXE_NAME}" "DEBUG=1")
endif()

if(WITH_SSE2)
  if(MSVC)
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} /arch:SSE2")
  else()
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} -msse2")
  endif()
endif()

if(WITH_LTO)
  if(MSVC)
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} /GL")
  else()
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} -flto")
  endif()
endif()

if(MSVC)
  # TODO: Find a way to do this cleanly for non MSVC users.
  set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} /MP2")
endif()

set_target_properties("${SM_EXE_NAME}" PROPERTIES COMPILE_FLAGS ${SM_COMPILE_FLAGS})

set_target_properties("${SM_EXE_NAME}" PROPERTIES OUTPUT_NAME_DEBUG "StepMania-debug")
set_target_properties("${SM_EXE_NAME}" PROPERTIES OUTPUT_NAME_MINSIZEREL "StepMania-min-size")
set_target_properties("${SM_EXE_NAME}" PROPERTIES OUTPUT_NAME_RELWITHDEBINFO "StepMania-release-symbols")

if(WIN32)
  sm_add_compile_definition("${SM_EXE_NAME}" WINDOWS)
  sm_add_compile_definition("${SM_EXE_NAME}" _WINDOWS) # TODO: Remove this potential duplicate.
  sm_add_compile_definition("${SM_EXE_NAME}" _CRT_SECURE_NO_WARNINGS)
  sm_add_compile_definition("${SM_EXE_NAME}" _WINSOCK_DEPRECATED_NO_WARNINGS)
  sm_add_compile_definition("${SM_EXE_NAME}" GLEW_STATIC)
  
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SM_PROGRAM_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_PROGRAM_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_PROGRAM_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_PROGRAM_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_PROGRAM_DIR}")
elseif(APPLE)
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_ROOT_DIR}")
  
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_FILE
    "${SM_XCODE_DIR}/Info-StepMania.plist"
  )
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREFIX_HEADER "${SM_SRC_DIR}/ProductInfo.h")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS "YES")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_INFOPLIST_PREPROCESSOR_DEFINITIONS
    "EXECUTABLE_NAME=\$(PRODUCT_NAME)"
  )

  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/archutils/Darwin/StepMania.pch")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_GCC_ENABLE_CPP_EXCEPTIONS "NO")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_LIBRARY_SEARCH_PATHS
    "${SM_XCODE_DIR}/ffmpeg/lib ${SM_XCODE_DIR}/Libraries"
  )
  set_target_properties("${SM_EXE_NAME}" PROPERTIES XCODE_ATTRIBUTE_OTHER_LDFLAGS "-lbz2")
  sm_add_compile_definition("${SM_EXE_NAME}" _XOPEN_SOURCE)
  
  # Add the ability to copy the resource file.
  add_custom_command(TARGET "${SM_EXE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:StepMania>/../Resources"
  )
  add_custom_command(TARGET "${SM_EXE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SM_XCODE_DIR}/smicon.icns" "$<TARGET_FILE_DIR:StepMania>/../Resources/"
  )
  add_custom_command(TARGET "${SM_EXE_NAME}"
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${SM_XCODE_DIR}/Hardware.plist" "$<TARGET_FILE_DIR:StepMania>/../Resources/"
  )
else() # Linux
  # TODO: Have this compile definition be used all over. Currently fixed for Windows and Mac, but in headers.
  sm_add_compile_definition("${SM_EXE_NAME}" "${ENDIANNESS}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_ROOT_DIR}")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_ROOT_DIR}")

  if (${HAS_PTHREAD})
    sm_add_compile_definition("${SM_EXE_NAME}" HAVE_LIBPTHREAD)
  endif()
  if (${HAS_FFMPEG})
    sm_add_compile_definition("${SM_EXE_NAME}" HAVE_FFMPEG)
  endif()
  if (BSD)
    sm_add_compile_definition("${SM_EXE_NAME}" BSD)
  endif()

  if (GTK2_FOUND)
    sm_add_compile_definition("${SM_EXE_NAME}" HAVE_GTK)
  endif()

  sm_add_compile_definition("${SM_EXE_NAME}" UNIX)
  if("${CMAKE_SYSTEM}" MATCHES "Linux")
    sm_add_compile_definition("${SM_EXE_NAME}" LINUX)
  endif()
endif()

set_property(TARGET "${SM_EXE_NAME}" PROPERTY FOLDER "Internal Libraries")

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
  list(APPEND SMDATA_LINK_LIB
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
  # Mad is not built in Windows.
  list(REMOVE_ITEM SMDATA_LINK_LIB "mad")
  
  get_filename_component(DIRECTX_LIBRARY_DIR "${DIRECTX_LIBRARIES}" DIRECTORY)
  
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${DIRECTX_LIBRARY_DIR}\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_EXTERN_DIR}/ffmpeg/lib\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_EXTERN_DIR}/libjpeg\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_EXTERN_DIR}/mad-0.15.1b/msvc++/Release\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_EXTERN_DIR}/vorbis/win32\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_SRC_DIR}/archutils/Win32/ddk\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/ERRORREPORT:SEND")
  sm_add_link_flag("${SM_EXE_NAME}" "/MAPINFO:EXPORTS")
  sm_add_link_flag("${SM_EXE_NAME}" "/SAFESEH:NO")
  sm_add_link_flag("${SM_EXE_NAME}" "/NOLOGO")
  sm_add_link_flag("${SM_EXE_NAME}" "/MAP")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:wininet.lib")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:msimg32.lib")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:libci.lib")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties("${SM_EXE_NAME}" PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
elseif(APPLE)
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
  # TODO: Remember to find and locate the zip archive files.
  list(APPEND SMDATA_LINK_LIB
    "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg-build/libavformat/libavformat.a"
    "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg-build/libavcodec/libavcodec.a"
    "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg-build/libswscale/libswscale.a"
    "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg-build/libavutil/libavutil.a"
    "${DL_LIBRARY}"
  )

  if(HAS_OGG)
    list(APPEND SMDATA_LINK_LIB
      "${VORBISFILE_LIBRARY}"
      "${VORBIS_LIBRARY}"
      "${OGG_LIBRARY}"
    )
  endif()

  if(HAS_MP3)
    list(APPEND SMDATA_LINK_LIB
      "${LIBMAD_LIBRARY}"
    )
  endif()

  list(APPEND SMDATA_LINK_LIB
    "${OPENGL_LIBRARY}"
  )

  if(GTK2_FOUND)
    list(APPEND SMDATA_LINK_LIB
      "${GTK2_LIBRARIES}"
    )
  endif()

  list(APPEND SMDATA_LINK_LIB
    "${BZIP2_LIBRARIES}"
    "${ZLIB_LIBRARIES}"
    "${CMAKE_THREAD_LIBS_INIT}"
  )

  if (HAS_ALSA)
    list(APPEND SMDATA_LINK_LIB ${ALSA_LIBRARIES})
  endif()

  if (HAS_JACK)
    list(APPEND SMDATA_LINK_LIB ${JACK_LIBRARIES})
  endif()

  if (HAS_OSS)
    # No mention of OSS libraries.
    sm_add_compile_definition("${SM_EXE_NAME}" "HAVE_OSS_VERSION=1")
  endif()

  if (HAS_PULSE)
    list(APPEND SMDATA_LINK_LIB ${PULSEAUDIO_LIBRARY})
    # PACKAGE_NAME and PACKAGE_VERSION are only used in this scenario. Why is not clear.
    # TODO: Remove this silliness.
    sm_add_compile_definition("${SM_EXE_NAME}" PACKAGE_NAME="StepMania")
    set(PACKAGE_VERSION "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}")
    sm_add_compile_definition("${SM_EXE_NAME}" PACKAGE_VERSION="${PACKAGE_VERSION}")
  endif()

  if(X11_FOUND)
    list(APPEND SMDATA_LINK_LIB ${X11_LIBRARIES})
  endif()

  list(APPEND SMDATA_LINK_LIB
    ${XRANDR_LIBRARIES}
  )
  list(REMOVE_ITEM SMDATA_LINK_LIB "zlib")
endif()

target_link_libraries("${SM_EXE_NAME}" ${SMDATA_LINK_LIB})

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
    if (HAS_FFMPEG)
      add_dependencies("${SM_EXE_NAME}" "ffmpeg")
      list(APPEND SM_INCLUDE_DIRS
        "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg"
        "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg-build"
      )
    endif()
    if (HAS_GTK2)
      list(APPEND SM_INCLUDE_DIRS "${GTK2_INCLUDE_DIRS}")
    endif()
    if (X11_FOUND)
      list(APPEND SM_INCLUDE_DIRS "${X11_INCLUDE_DIR}")
    endif()
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

target_include_directories("${SM_EXE_NAME}" PUBLIC ${SM_INCLUDE_DIRS})

if(WIN32)
  # TODO: Look forward to 64-bit builds. Also, Find a way to not need to hardcode this.
  set(SM_INSTALL_DESTINATION "C:/Program Files (x86)")
elseif(APPLE)
  # TODO: Confirm if anything special is needed. Most Mac apps are just portable .app folders.
  set(SM_INSTALL_DESTINATION "")
else()
  # TODO: Allow for a more flexible system. Good luck convincing old users of that.
  set(SM_INSTALL_DESTINATION "/opt/stepmania-5.0")
endif()

if(NOT APPLE)
  install(TARGETS "${SM_EXE_NAME}" DESTINATION "${SM_INSTALL_DESTINATION}")
  if (UNIX OR LINUX)
    install(FILES "${SM_ROOT_DIR}/GtkModule.so" LIBRARY DESTINATION "${SM_INSTALL_DESTINATION}" OPTIONAL)
  endif()
  install(DIRECTORY "${SM_ROOT_DIR}/Announcers" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/BGAnimations" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Themes" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Characters" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Scripts" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Courses" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/BackgroundEffects" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Data" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/BackgroundTransitions" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/Docs" DESTINATION "${SM_INSTALL_DESTINATION}")
  install(DIRECTORY "${SM_ROOT_DIR}/NoteSkins" DESTINATION "${SM_INSTALL_DESTINATION}")
  if (NOT SM_INSTALL_DESTINATION)
    install(DIRECTORY "${SM_ROOT_DIR}/Songs/StepMania 5" DESTINATION "Songs")
    install(FILES "${SM_ROOT_DIR}/Songs/instructions.txt" DESTINATION "Songs")
  else()
    install(DIRECTORY "${SM_ROOT_DIR}/Songs/StepMania 5" DESTINATION "${SM_INSTALL_DESTINATION}/Songs")
    install(FILES "${SM_ROOT_DIR}/Songs/instructions.txt" DESTINATION "${SM_INSTALL_DESTINATION}/Songs")
  endif()
endif()

