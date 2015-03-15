cmake_minimum_required(VERSION 2.8.12)

project(StepMania)

# Include the macros and functions.
include(CMake/CMakeMacros.cmake)

# Set up version numbers according to the new scheme.
set(SM_VERSION_MAJOR 5)
set(SM_VERSION_MINOR 0)
set(SM_VERSION_PATCH 6)
set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")

# Set up helper variables for future configuring.
set(SM_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(SM_EXTERN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")
set(SM_BUNDLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bundle")
set(SM_XCODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Xcode")
set(SM_PROGRAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Program")
set(SM_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Build")
set(SM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Some OS specific helpers.
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
else()
  set(LINUX FALSE)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(MACOSX TRUE)
else()
  set(MACOSX FALSE)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "BSD")
  set(BSD TRUE)
  if (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
    set(FREEBSD TRUE)
  endif()
  if (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    set(OPENBSD TRUE)
  endif()
  if (CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    set(NETBSD TRUE)
  endif()
else()
  set(BSD FALSE)
endif()

# Some eventual distribution stuff.
set(CPACK_PACKAGE_NAME "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")

# Using the deprecated version for now to keep cmake 2.8 users happy.
exec_program(git "${SM_ROOT_DIR}"
  ARGS "rev-parse" "--short" "HEAD"
  OUTPUT_VARIABLE SM_VERSION_GIT_HASH
  RETURN_VALUE ret
)

if(NOT (ret STREQUAL "0"))
  SET(SM_VERSION_GIT_HASH "UNKNOWN")
endif()

# Allow for finding our libraries in a standard location.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/")

# Put the predefined targets in separate groups.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Prep options that are needed for each platform.
option(WITH_NETWORKING "Build with networking support" ON)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO "Build with Link Time Optimization (LTO)/Whole Program Optimization" OFF)

option(WITH_SSE2 "Build with SSE2 Optimizations" ON)

# This option may go away in the future: if it does, JPEG will always be required.
option(WITH_JPEG "Build with JPEG Image Support" ON)

if(WIN32)
  option(WITH_MINIMAID "Build with Mimimaid Lights Support" OFF)
else()
  if(LINUX)
    option(WITH_FFMPEG "Build with FFMPEG" ON)
    option(WITH_TTY "Build with Linux TTY Input Support" OFF)
    option(WITH_PROFILING "Build with Profiling Support" OFF)
    option(WITH_GLES2 "Build with OpenGL ES 2.0 Support" ON)
    option(WITH_GTK2 "Build with GTK2 Support" ON)
  endif()
endif()

# Dependencies go here.
set(ENDIANNESS "ENDIAN_LITTLE")
if(WIN32)
  find_package(DirectX REQUIRED)
  
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
elseif(MACOSX)
  # Apple Archs needs to be 32-bit for now.
  # When SDL2 is introduced, this may change.
  set(CMAKE_OSX_ARCHITECTURES "i386")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
  
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
  )
elseif(LINUX)
  include(TestBigEndian)
  include(ExternalProject)

  if(WITH_GTK2)
    find_package("GTK2")
    if (${GTK2_FOUND})
      set(HAS_GTK2 TRUE)
    else()
      set(HAS_GTK2 FALSE)
      message("GTK2 was not found on your system. There will be no loading window.")
    endif()
  else()
    set(HAS_GTK2 FALSE)
  endif()

  find_package("BZip2")
  if (NOT(${BZIP2_FOUND}))
    message(FATAL_ERROR "Bzip2 support required.")
  endif()

  find_package("ZLIB")
  if (NOT(${ZLIB_FOUND}))
    message(FATAL_ERROR "zlib support required.")
  endif()

  find_library(DL_LIBRARY dl)
  if(${LIBDL_FOUND})
    set(HAS_LIBDL TRUE)
  else()
    set(HAS_LIBDL FALSE)
  endif()

  find_package(Xrandr)
  if (${XRANDR_FOUND})
    set(HAS_XRANDR TRUE)
  else()
    set(HAX_XRANDR FALSE)
  endif()

  include("${SM_CMAKE_DIR}/CMakeScout_Ogg.cmake")
  find_package(Vorbis)
  find_package(VorbisFile)

  find_package(PulseAudio)
  if (PULSEAUDIO_FOUND)
    set(HAS_PULSE TRUE)
  else()
    set(HAS_PULSE FALSE)
  endif()

  find_package(ALSA)
  if (ALSA_FOUND)
    set(HAS_ALSA TRUE)
  else()
    set(HAS_ALSA FALSE)
  endif()

  find_package(JACK)
  if (JACK_FOUND)
    set(HAS_JACK TRUE)
  else()
    set(HAS_JACK FALSE)
  endif()

  find_package(OSS)
  if (OSS_FOUND)
    set(HAS_OSS TRUE)
  else()
    set(HAS_OSS FALSE)
  endif()

  if(NOT OSS_FOUND AND NOT JACK_FOUND AND NOT ALSA_FOUND AND NOT PULSE_FOUND)
    message(FATAL_ERROR "No sound libraries found. You will require at least one.")
  endif()

  find_package(Threads)
  if (${Threads_FOUND})
    set(HAS_PTHREAD TRUE)
  else()
    set(HAS_PTHREAD FALSE)
  endif()

  find_package(yasm)
  if (NOT YASM_FOUND)
    message("YASM was not found. Please install if you wish for ffmpeg support.")
    set(WITH_FFMPEG OFF)
  endif()

  if(WITH_FFMPEG)
    # NEVER use the system ffmpeg. Not worth the hassle.
    # --shlibdir=$our_installdir/stepmania-$VERSION
    externalproject_add("ffmpeg"
      DOWNLOAD_DIR "${SM_EXTERN_DIR}/ffmpeg-linux"
      PREFIX "${SM_EXTERN_DIR}/ffmpeg-linux"
      GIT_REPOSITORY "git://source.ffmpeg.org/ffmpeg.git"
      GIT_TAG "n2.1.3"
      CONFIGURE_COMMAND "${SM_EXTERN_DIR}/ffmpeg-linux/src/ffmpeg/configure" "--enable-gpl" "--disable-programs" "--disable-doc" "--disable-avdevice" "--disable-swresample" "--disable-postproc" "--disable-avfilter" "--disable-shared" "--enable-static"
      BUILD_COMMAND "make"
      UPDATE_COMMAND ""
      INSTALL_COMMAND ""
      TEST_COMMAND ""
    )
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

  test_big_endian(BIGENDIAN)
  if (${BIGENDIAN})
    set(ENDIANNESS "ENDIAN_BIG")
  endif()

endif()


# The external libraries need to be included.
add_subdirectory(extern)

# The internal libraries and eventual executable to be used.
add_subdirectory(src)

