cmake_minimum_required(VERSION 2.8)

project(StepMania)

# Include the macros and functions.
include(CMake/CMakeMacros.txt)

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
    set(OPENBSD FALSE)
    set(NETBSD FALSE)
  elseif (CMAKE_SYSTEM_NAME MATCHES "OpenBSD")
    set(FREEBSD FALSE)
    set(OPENBSD TRUE)
    set(NETBSD FALSE)
  elseif (CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    set(FREEBSD FALSE)
    set(OPENBSD FALSE)
    set(NETBSD TRUE)
  else()
    set(FREEBSD FALSE)
    set(OPENBSD FALSE)
    set(NETBSD FALSE)
  endif()
else()
  set(BSD FALSE)
  set(FREEBSD FALSE)
  set(OPENBSD FALSE)
  set(NETBSD FALSE)
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

# Set up common C/C++ flags.
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG=1")
  set(CMAKE_C_FLAGS_DEBUG "-g -DDEBUG=1")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG=1")
  set(CMAKE_C_FLAGS_DEBUG "-DDEBUG=1")
endif()

# Allow for finding DirectX.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

if(APPLE)
  # Apple Archs needs to be 32-bit for now.
  # When SDL2 is introduced, this may change.
  set(CMAKE_OSX_ARCHITECTURES "i386")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.6")
elseif(WIN32)
  include(CMake/CMakeScout_DirectX.txt)
endif()

# Put the predefined targets in separate groups.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Prep the endian test.
set(ENDIANNESS "ENDIAN_LITTLE")
if(LINUX)
  include(TestBigEndian)
  test_big_endian(BIGENDIAN)
  if (${BIGENDIAN})
    set(ENDIANNESS "ENDIAN_BIG")
  endif()
endif()

# Prep options that are needed for each platform.
option(WITH_NETWORKING "Build with networking support" ON)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO "Build with Link Time Optimization (LTO)/Whole Program Optimization" OFF)

option(WITH_SSE2 "Build with SSE2 Optimizations" ON)

# This option may go away in the future: if it does, JPEG will always be required.
option(WITH_JPEG "Build with JPEG Image Support" ON)

if(WIN32)
  option(WITH_MINIMAID "Build with Mimimaid Lights Support" OFF)
elseif(LINUX)
  option(WITH_FFMPEG "Build with FFMPEG" ON)
  option(WITH_TTY "Build with Linux TTY Input Support" OFF)
  option(WITH_PROFILING "Build with Profiling Support" OFF)
  option(WITH_GLES2 "Build with OpenGL ES 2.0 Support" ON)
  option(WITH_GTK2 "Build with GTK2 Support" ON)
endif()

# The external libraries need to be included.
add_subdirectory(extern)

# The internal libraries and eventual executable to be used.
add_subdirectory(src)

