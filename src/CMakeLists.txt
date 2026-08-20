if(MSVC)
  # Add the mapconv program of the past to allow for debugging information.
  include("CMakeProject-mapconv.cmake")
  if(WITH_TEXTURE_GENERATOR OR WITH_FULL_RELEASE)
    include("CMakeProject-texture.cmake")
  endif()
endif()

# Keep the module path local for easier grabbing.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Main project is below.

include(CMakeData-arch.cmake)
include(CMakeData-rage.cmake)
include(CMakeData-os.cmake)
include(CMakeData-actor.cmake)
include(CMakeData-screen.cmake)
include(CMakeData-data.cmake)
include(CMakeData-file-types.cmake)
include(CMakeData-globals.cmake)
include(CMakeData-singletons.cmake)

list(APPEND SMDATA_ALL_FILES_SRC
            ${SMDATA_GLOBAL_FILES_SRC}
            ${SMDATA_GLOBAL_SINGLETON_SRC}
            ${SMDATA_ALL_ACTORS_SRC}
            ${SMDATA_ALL_ARCH_SRC}
            ${SMDATA_ALL_DATA_SRC}
            ${SMDATA_ALL_RAGE_SRC}
            ${SMDATA_ALL_SCREENS_SRC}
            ${SMDATA_OS_SRC}
            ${SMDATA_FILE_TYPES_SRC})
list(APPEND SMDATA_ALL_FILES_HPP
            ${SMDATA_GLOBAL_FILES_HPP}
            ${SMDATA_GLOBAL_SINGLETON_HPP}
            ${SMDATA_ALL_ACTORS_HPP}
            ${SMDATA_ALL_ARCH_HPP}
            ${SMDATA_ALL_DATA_HPP}
            ${SMDATA_ALL_RAGE_HPP}
            ${SMDATA_ALL_SCREENS_HPP}
            ${SMDATA_OS_HPP}
            ${SMDATA_FILE_TYPES_HPP})

if(NOT APPLE)
  list(APPEND SMDATA_ALL_FILES_SRC "Main.cpp")
  source_group("" FILES "Main.cpp")
endif()

if(MSVC OR APPLE)
  set(SM_NAME_RELEASE "StepMania")
  set(SM_NAME_DEBUG "StepMania-debug")
  set(SM_NAME_MINSIZEREL "StepMania-min-size")
  set(SM_NAME_RELWITHDEBINFO "StepMania-release-symbols")
else()
  set(SM_NAME_RELEASE "stepmania")
  set(SM_NAME_DEBUG "stepmania-debug")
  set(SM_NAME_MINSIZEREL "stepmania-min-size")
  set(SM_NAME_RELWITHDEBINFO "stepmania-release-symbols")
endif()

if(APPLE)
  add_executable("${SM_EXE_NAME}"
                 MACOSX_BUNDLE
                 ${SMDATA_ALL_FILES_SRC}
                 ${SMDATA_ALL_FILES_HPP})
else()
  if(MSVC)
    foreach(sm_src_file ${SMDATA_ALL_FILES_SRC})
      get_filename_component(sm_src_raw "${sm_src_file}" NAME_WE)
      if(${sm_src_raw} MATCHES "global")
        set_source_files_properties("${sm_src_raw}.cpp"
                                    PROPERTIES
                                    COMPILE_FLAGS
                                    "/Ycglobal.h")
      elseif(NOT (${sm_src_raw} MATCHES "verstub"))
        set_source_files_properties("${sm_src_raw}.cpp"
                                    PROPERTIES
                                    COMPILE_FLAGS
                                    "/Yuglobal.h")
      endif()
    endforeach()
  endif()
  add_executable("${SM_EXE_NAME}" ${SMDATA_ALL_FILES_SRC}
                 ${SMDATA_ALL_FILES_HPP})
endif()

set_property(TARGET "${SM_EXE_NAME}" PROPERTY CXX_STANDARD 17)
set_property(TARGET "${SM_EXE_NAME}" PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET "${SM_EXE_NAME}" PROPERTY CXX_EXTENSIONS ON)

set_target_properties("${SM_EXE_NAME}"
                      PROPERTIES OUTPUT_NAME "${SM_NAME_RELEASE}"
                                 RELEASE_OUTPUT_NAME "${SM_NAME_RELEASE}"
                                 DEBUG_OUTPUT_NAME "${SM_NAME_DEBUG}"
                                 MINSIZEREL_OUTPUT_NAME "${SM_NAME_MINSIZEREL}"
                                 RELWITHDEBINFO_OUTPUT_NAME "${SM_NAME_RELWITHDEBINFO}")

if(WITH_LTO)
  include(CheckIPOSupported)
  check_ipo_supported()
  set_property(TARGET "${SM_EXE_NAME}" PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(WITH_PORTABLE_TOMCRYPT)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE LTC_NO_ASM)
endif()
if(WITH_NO_ROLC_TOMCRYPT)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE LTC_NO_ROLC)
endif()

# Compilation flags per project here.
target_compile_definitions("${SM_EXE_NAME}" PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions("${SM_EXE_NAME}" PRIVATE $<$<CONFIG:Release>:RELEASE>)
target_compile_definitions("${SM_EXE_NAME}" PRIVATE $<$<CONFIG:MinSizeRel>:MINSIZEREL>)
target_compile_definitions("${SM_EXE_NAME}" PRIVATE
                           $<$<CONFIG:RelWithDebInfo>:RELWITHDEBINFO>)

# For Apple Clang, CMAKE_<LANG>_COMPILER_ID will be set to AppleClang instead of Clang.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()

target_compile_options("${SM_EXE_NAME}" PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:
  -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-unknown-pragmas -Werror=type-limits>
  $<$<CXX_COMPILER_ID:Clang>:
    -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-unknown-pragmas -Wno-undefined-var-template>
  $<$<CXX_COMPILER_ID:AppleClang>:
    -Wall -Wextra -Wno-unused -Wno-unused-parameter -Wno-unknown-pragmas -Wno-undefined-var-template -Wno-deprecated-declarations -Wno-unused-command-line-argument>
  $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /wd4100 /wd4189 /wd4244 /wd4267 /wd4702>
)

set(SM_COMPILE_FLAGS "")

if(WITH_SSE2)
  if(MSVC)
    if(SM_WIN32_ARCH MATCHES "x86")
      set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} /arch:SSE2")
    endif()
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86")
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} -msse2")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} -mtune=cortex-a72")
  else()
    message("Unrecognized host processor type")
  endif()
endif()

if(MSVC)
  # TODO: Find a way to do this cleanly for non MSVC users.
  set(SM_COMPILE_FLAGS "${SM_COMPILE_FLAGS} /MP2 /utf-8")
endif()

set_target_properties("${SM_EXE_NAME}"
                      PROPERTIES COMPILE_FLAGS "${SM_COMPILE_FLAGS}")

set_target_properties("${SM_EXE_NAME}"
                      PROPERTIES OUTPUT_NAME_DEBUG "${SM_NAME_DEBUG}"
                                 OUTPUT_NAME_MINSIZEREL "${SM_NAME_MINSIZEREL}"
                                 OUTPUT_NAME_RELWITHDEBINFO "${SM_NAME_RELWITHDEBINFO}")

if(WIN32)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE WINDOWS)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE _WINDOWS) # TODO: Remove this potential duplicate.
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)

  set_target_properties("${SM_EXE_NAME}"
                        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                   "${SM_PROGRAM_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_RELEASE
                                   "${SM_PROGRAM_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_DEBUG
                                   "${SM_PROGRAM_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                                   "${SM_PROGRAM_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                                   "${SM_PROGRAM_DIR}")

  if(MSVC)
    # Allow for getting a virtualdub stack trace.
    add_custom_command(
      TARGET "${SM_EXE_NAME}" POST_BUILD
      COMMAND
        "mapconv"
        "${SM_PROGRAM_DIR}/$<$<CONFIG:DEBUG>:${SM_NAME_DEBUG}>$<$<CONFIG:MINSIZEREL>:${SM_NAME_MINSIZEREL}>$<$<CONFIG:RELWITHDEBINFO>:${SM_NAME_RELWITHDEBINFO}>$<$<CONFIG:RELEASE>:${SM_NAME_RELEASE}>.map"
        "${SM_PROGRAM_DIR}/$<$<CONFIG:DEBUG>:${SM_NAME_DEBUG}>$<$<CONFIG:MINSIZEREL>:${SM_NAME_MINSIZEREL}>$<$<CONFIG:RELWITHDEBINFO>:${SM_NAME_RELWITHDEBINFO}>$<$<CONFIG:RELEASE>:${SM_NAME_RELEASE}>.vdi"
      COMMENT "Generating file to allow for easier stack traces.")
  endif()
elseif(APPLE)
  # TODO should we set CRASH_HANDLER and other things like LINUX does below? Why is that logic specfic to LINUX?
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE BACKTRACE_METHOD_X86_DARWIN)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE BACKTRACE_LOOKUP_METHOD_TEXT="dladdr")
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE BACKTRACE_LOOKUP_METHOD_DLADDR)
  target_compile_definitions("${SM_EXE_NAME}" PRIVATE MACOSX)
  set_target_properties(
    "${SM_EXE_NAME}"
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY
      "${SM_ROOT_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_RELEASE
      "${SM_ROOT_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_DEBUG
      "${SM_ROOT_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
      "${SM_ROOT_DIR}"
      RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
      "${SM_ROOT_DIR}"
      MACOSX_BUNDLE_INFO_PLIST
      "${SM_XCODE_DIR}/Info.plist.in")

  set(APPLE_BUNDLE_RESOURCES
    "${SM_ROOT_DIR}/Announcers"
    "${SM_ROOT_DIR}/BackgroundEffects"
    "${SM_ROOT_DIR}/BackgroundTransitions"
    "${SM_ROOT_DIR}/BGAnimations"
    "${SM_ROOT_DIR}/Characters"
    "${SM_ROOT_DIR}/Courses"
    "${SM_ROOT_DIR}/Data"
    "${SM_ROOT_DIR}/NoteSkins"
    "${SM_ROOT_DIR}/Scripts"
    "${SM_ROOT_DIR}/Songs"
    "${SM_ROOT_DIR}/Themes"
  )

  target_sources("${SM_EXE_NAME}" PUBLIC ${APPLE_BUNDLE_RESOURCES})
  set_source_files_properties(${APPLE_BUNDLE_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  target_compile_definitions("${SM_EXE_NAME}" PRIVATE _XOPEN_SOURCE)

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SM_APP_RELEASE_NAME "${SM_NAME_DEBUG}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    set(SM_APP_RELEASE_NAME "${SM_NAME_MINSIZEREL}")
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(SM_APP_RELEASE_NAME "${SM_NAME_RELWITHDEBINFO}")
  else()
    set(SM_APP_RELEASE_NAME "${SM_NAME_RELEASE}")
  endif()

  # Add the ability to copy the resource file.
  add_custom_command(TARGET "${SM_EXE_NAME}" POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E make_directory
                             "$<TARGET_FILE_DIR:StepMania>/../Resources"
                     COMMAND ${CMAKE_COMMAND} -E copy
                             "${SM_XCODE_DIR}/smicon.icns"
                             "$<TARGET_FILE_DIR:StepMania>/../Resources/")
else() # Linux
  set_target_properties("${SM_EXE_NAME}"
                        PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                   "${SM_ROOT_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_RELEASE
                                   "${SM_ROOT_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_DEBUG
                                   "${SM_ROOT_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
                                   "${SM_ROOT_DIR}"
                                   RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
                                   "${SM_ROOT_DIR}")

  if(${WITH_CRASH_HANDLER})
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE CRASH_HANDLER)
    if(LINUX)
      # used only in the ArchHooks_Unix.cpp file
      if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86" OR CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        target_compile_definitions("${SM_EXE_NAME}" PRIVATE BACKTRACE_METHOD_X86_LINUX)
        target_compile_definitions("${SM_EXE_NAME}" PRIVATE
                                   BACKTRACE_METHOD_TEXT="x86 custom backtrace")
      else()
        target_compile_definitions("${SM_EXE_NAME}" PRIVATE
                                   BACKTRACE_METHOD_TEXT="no backtrace")
      endif()
      target_compile_definitions("${SM_EXE_NAME}" PRIVATE
                                 BACKTRACE_LOOKUP_METHOD_TEXT="dladdr")
      target_compile_definitions("${SM_EXE_NAME}" PRIVATE
                                 BACKTRACE_LOOKUP_METHOD_DLADDR)
    endif()
  endif()
  if(${HAS_PTHREAD})
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE HAVE_LIBPTHREAD)

    message("Host processor is ${CMAKE_SYSTEM_PROCESSOR}")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
      message("Host processor is 64bit X86")
      target_compile_definitions("${SM_EXE_NAME}" PRIVATE CPU_X86_64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86"
           OR CMAKE_SYSTEM_PROCESSOR MATCHES "i686")
      message("Host processor is 32bit X86")
      target_compile_definitions("${SM_EXE_NAME}" PRIVATE CPU_X86)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
      message("Host processor is 64bit ARM")
      target_compile_definitions("${SM_EXE_NAME}" PRIVATE CPU_AARCH64)
    else()
      message("Unrecognized host processor type")
    endif()
  endif()
  if(${HAS_XRANDR})
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE HAVE_XRANDR)
  endif()
  if(${HAS_LIBXTST})
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE HAVE_LIBXTST)
  endif()
  if(${HAS_XINERAMA})
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE HAVE_XINERAMA)
  endif()

  if(BSD)
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE BSD)
  endif()

  if(GTK3_FOUND)
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE HAVE_GTK)
  endif()

  target_compile_definitions("${SM_EXE_NAME}" PRIVATE UNIX)
  if("${CMAKE_SYSTEM}" MATCHES "Linux")
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE LINUX)
  endif()
endif()

set_property(TARGET "${SM_EXE_NAME}" PROPERTY FOLDER "Internal Libraries")

list(APPEND SMDATA_LINK_LIB "lua-5.1" "miniz")

if(WITH_MINIMAID)
  include(../extern/CMakeProject-mmmagic.cmake)
endif()

list(APPEND SMDATA_LINK_LIB "mad"
                            "pcre"
                            "glew"
                            "tomcrypt"
                            "tommath"
                            "jpeg"
                            "zlib"
                            "png"
                            "jsoncpp"
                            "vorbisfile"
                            "vorbis"
                            "ogg"
                            "ixwebsocket")

if(WIN32)
  list(APPEND SMDATA_LINK_LIB
              # The misc libraries are here.
              "${LIB_SWSCALE}"
              "${LIB_AVCODEC}"
              "${LIB_AVFORMAT}"
              "${LIB_AVUTIL}")

  list(APPEND SMDATA_LINK_LIB
              "dbghelp.lib"
              "setupapi.lib"
              "hid.lib")

  sm_add_link_flag("${SM_EXE_NAME}" "/LIBPATH:\"${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}\"")
  sm_add_link_flag("${SM_EXE_NAME}"
                   "/LIBPATH:\"$(WindowsSdkDir)Lib\\um\\${SM_WIN32_ARCH}\"")
  sm_add_link_flag("${SM_EXE_NAME}" "/ERRORREPORT:SEND")
  sm_add_link_flag("${SM_EXE_NAME}" "/MAPINFO:EXPORTS")
  sm_add_link_flag("${SM_EXE_NAME}" "/SAFESEH:NO")
  sm_add_link_flag("${SM_EXE_NAME}" "/NOLOGO")
  sm_add_link_flag("${SM_EXE_NAME}" "/MAP")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:wininet.lib")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:msimg32.lib")
  sm_add_link_flag("${SM_EXE_NAME}" "/NODEFAULTLIB:libci.lib")
  set_target_properties("${SM_EXE_NAME}"
                        PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:msvcrt.lib")
  set_target_properties("${SM_EXE_NAME}"
                        PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  set_target_properties("${SM_EXE_NAME}"
                        PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")

elseif(APPLE)
  list(INSERT SMDATA_LINK_LIB
              0
              ${MAC_FRAME_ACCELERATE}
              ${MAC_FRAME_CARBON}
              ${MAC_FRAME_COCOA}
              ${MAC_FRAME_IOKIT}
              ${MAC_FRAME_OPENGL}
              ${MAC_FRAME_COREFOUNDATION}
              ${MAC_FRAME_AUDIOTOOLBOX}
              ${MAC_FRAME_AUDIOUNIT}
              ${MAC_FRAME_COREAUDIO}
              ${MAC_FRAME_CORESERVICES}
              ${MAC_FRAME_SYSTEM}
              "${ICONV_LIBRARIES}"
              "${SM_FFMPEG_LIB}/libavformat.a"
              "${SM_FFMPEG_LIB}/libavcodec.a"
              "${SM_FFMPEG_LIB}/libswscale.a"
              "${SM_FFMPEG_LIB}/libavutil.a"
              "${MAC_FRAME_COREMEDIA}"
              "${MAC_FRAME_COREVIDEO}"
              "${MAC_FRAME_VIDEOTOOLBOX}")
else() # Unix / Linux TODO: Remember to find and locate the zip archive files.
  list(APPEND SMDATA_LINK_LIB
              "${SM_FFMPEG_LIB}/libavformat.a"
              "${SM_FFMPEG_LIB}/libavcodec.a"
              "${SM_FFMPEG_LIB}/libswscale.a"
              "${SM_FFMPEG_LIB}/libavutil.a")

  list(REMOVE_ITEM SMDATA_LINK_LIB "zlib")
  list(APPEND SMDATA_LINK_LIB "zlib"
                              ${CMAKE_DL_LIBS}
                              "${OPENGL_LIBRARY}")

  if(GTK3_FOUND)
    list(APPEND SMDATA_LINK_LIB "${GTK3_LIBRARIES}")
  endif()

  list(APPEND SMDATA_LINK_LIB "${CMAKE_THREAD_LIBS_INIT}")

  if(HAS_ALSA)
    list(APPEND SMDATA_LINK_LIB ${ALSA_LIBRARIES})
  endif()

  if(HAS_JACK)
    list(APPEND SMDATA_LINK_LIB ${JACK_LIBRARIES})
  endif()

  if(HAS_OSS)
    # No mention of OSS libraries.
    target_compile_definitions("${SM_EXE_NAME}" PRIVATE "HAVE_OSS_VERSION=1")
  endif()

  if(HAS_PULSE)
    list(APPEND SMDATA_LINK_LIB ${PULSEAUDIO_LIBRARY})
  endif()

  if(X11_FOUND)
    list(APPEND SMDATA_LINK_LIB ${X11_LIBRARIES})
  endif()

  if(LIBXTST_FOUND)
    list(APPEND SMDATA_LINK_LIB ${LIBXTST_LIBRARY})
  endif()

  list(APPEND SMDATA_LINK_LIB ${LIBUSB_LIBRARIES})

  list(APPEND SMDATA_LINK_LIB ${XRANDR_LIBRARIES} ${XINERAMA_LIBRARIES})

  list(REMOVE_DUPLICATES SMDATA_LINK_LIB)
endif()

target_link_libraries("${SM_EXE_NAME}" ${SMDATA_LINK_LIB})

list(APPEND SM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
            "${SM_GENERATED_SRC_DIR}")

if(APPLE)
  list(APPEND SM_INCLUDE_DIRS "archutils/Unix")

  add_dependencies("${SM_EXE_NAME}" "ffmpeg")
  list(APPEND SM_INCLUDE_DIRS "${SM_FFMPEG_INCLUDE}")
else()
  if(MSVC)
    list(APPEND SM_INCLUDE_DIRS "${SM_EXTERN_DIR}/ffmpeg-w32/include")
  else()
    add_dependencies("${SM_EXE_NAME}" "ffmpeg")
    list(APPEND SM_INCLUDE_DIRS "${SM_FFMPEG_INCLUDE}")

    if(X11_FOUND)
      list(APPEND SM_INCLUDE_DIRS "${X11_INCLUDE_DIR}")
    endif()

    list(APPEND SM_INCLUDE_DIRS "${LIBUSB_INCLUDE_DIRS}")
  endif()
endif()

target_include_directories("${SM_EXE_NAME}" PUBLIC ${SM_INCLUDE_DIRS})

if(WIN32)
  set(SM_INSTALL_DESTINATION ".")
elseif(APPLE)
  set(SM_INSTALL_DESTINATION "StepMania")
else()
  set(SM_INSTALL_DESTINATION "stepmania")
endif()

if(WIN32)
  set(SM_FULL_INSTALLATION_PATH_LIST "${SM_INSTALL_DESTINATION}" "Program")
  list(JOIN SM_FULL_INSTALLATION_PATH_LIST "/" SM_FULL_INSTALLATION_PATH)
  # Hardcoding the values for now since the foreach loop is not working as
  # intended.
  install(TARGETS "${SM_EXE_NAME}" DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  if(WITH_FULL_RELEASE OR WITH_TEXTURE_GENERATOR)
    install(FILES "${SM_PROGRAM_DIR}/Texture Font Generator.exe"
            DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  endif()
  install(FILES "${SM_PROGRAM_DIR}/avcodec-59.dll"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  install(FILES "${SM_PROGRAM_DIR}/avformat-59.dll"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  install(FILES "${SM_PROGRAM_DIR}/avutil-57.dll"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  install(FILES "${SM_PROGRAM_DIR}/parallel_lights_io.dll"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  install(FILES "${SM_PROGRAM_DIR}/swscale-6.dll"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
  install(FILES "${SM_PROGRAM_DIR}/StepMania.vdi"
          DESTINATION "${SM_FULL_INSTALLATION_PATH}")
else()
  install(TARGETS "${SM_EXE_NAME}" DESTINATION "${SM_INSTALL_DESTINATION}")
  if(LINUX)
    install(FILES "${SM_ROOT_DIR}/stepmania.desktop"
            DESTINATION "${SM_INSTALL_DESTINATION}")
    install(PROGRAMS "${SM_ROOT_DIR}/Installer/setup.sh" DESTINATION ".")
  endif()
endif()
install(DIRECTORY "${SM_ROOT_DIR}/Announcers"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/BGAnimations"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Themes"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Characters"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Scripts"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Courses"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/BackgroundEffects"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Data"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/BackgroundTransitions"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/Docs"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(DIRECTORY "${SM_ROOT_DIR}/NoteSkins"
        DESTINATION "${SM_INSTALL_DESTINATION}")
install(FILES "${SM_ROOT_DIR}/Songs/instructions.txt"
        DESTINATION "${SM_INSTALL_DESTINATION}/Songs")

if(WITH_CLUB_FANTASTIC)
  # Fetch ClubFantastic packs
  include(FetchContent)
  FetchContent_Declare(
    ClubFantastic
    URL https://download.clubfantastic.dance/club_fantastic_season2_songs_9ms.zip
    URL_HASH SHA256=845b62da6bb2994fc2ba7871b35da02f6eb36ea1ae78c2b9a59a6ec046946787
  )
  FetchContent_MakeAvailable(ClubFantastic)
  FetchContent_GetProperties(ClubFantastic SOURCE_DIR CLUB_FANTASTIC_DIR)

  install(DIRECTORY "${CLUB_FANTASTIC_DIR}/Club Fantastic Season 1"
          DESTINATION "${SM_INSTALL_DESTINATION}/Songs")
  install(DIRECTORY "${CLUB_FANTASTIC_DIR}/Club Fantastic Season 2"
          DESTINATION "${SM_INSTALL_DESTINATION}/Songs")
endif()
