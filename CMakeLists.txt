cmake_minimum_required(VERSION 2.8)

# Include the macros and functions.
include(CMake/CMakeMacros.txt)

# Set up version numbers according to the new scheme.
set(SM_VERSION_MAJOR 5)
set(SM_VERSION_MINOR 0)
set(SM_VERSION_PATCH 6)

# Set up helper variables for future configuring.
set(SM_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(SM_EXTERN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")
set(SM_BUNDLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bundle")
set(SM_XCODE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Xcode")
set(SM_PROGRAM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Program")
set(SM_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Build")
set(SM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SM_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

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

# Finally set up the project.
project(StepMania)

# Put the predefined targets in separate groups.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Prep the endian test.
set(ENDIANNESS "ENDIAN_LITTLE")
if(APPLE)

elseif(WIN32)

else()
  include(TestBigEndian)
  test_big_endian(BIGENDIAN)
  if (${BIGENDIAN})
    set(ENDIANNESS "ENDIAN_BIG")
  endif()
endif()

# The external libraries need to be included.
add_subdirectory(extern)

# The internal libraries and eventual executable to be used.
add_subdirectory(src)

