# Include the macros and functions.

include(${CMAKE_CURRENT_LIST_DIR}/CMake/CMakeMacros.cmake)

# Make Xcode's 'Archive' build work
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/extern")

# Set up helper variables for future configuring.
set(SM_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/CMake")
set(SM_EXTERN_DIR "${CMAKE_CURRENT_LIST_DIR}/extern")
set(SM_INSTALLER_DIR "${CMAKE_CURRENT_LIST_DIR}/Installer")
set(SM_XCODE_DIR "${CMAKE_CURRENT_LIST_DIR}/Xcode")
set(SM_PROGRAM_DIR "${CMAKE_CURRENT_LIST_DIR}/Program")
set(SM_BUILD_DIR "${CMAKE_CURRENT_LIST_DIR}/Build")
set(SM_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(SM_DOC_DIR "${CMAKE_CURRENT_LIST_DIR}/Docs")
set(SM_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(SM_GENERATED_DIR "${PROJECT_BINARY_DIR}/generated")
set(SM_GENERATED_SRC_DIR "${SM_GENERATED_DIR}/src")

# TODO: Reconsile the OS dependent naming scheme.
set(SM_EXE_NAME "StepMania")

# Some OS specific helpers.
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(LINUX TRUE)
else()
  set(LINUX FALSE)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(MACOSX TRUE)
else()
  set(MACOSX FALSE)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "BSD")
  set(BSD TRUE)
else()
  set(BSD FALSE)
endif()

# Allow for finding our libraries in a standard location.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}"
            "${SM_CMAKE_DIR}/Modules/")

include("${SM_CMAKE_DIR}/DefineOptions.cmake")

include("${SM_CMAKE_DIR}/SMDefs.cmake")

# Put the predefined targets in separate groups.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Select static MSVC runtime instead of DLL
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Checks the standard include directories for c-style headers. We may use C++ in
# this project, but the check works better with plain C headers.
include(CheckIncludeFiles)
check_include_files(alloca.h HAVE_ALLOCA_H)
check_include_files(assert.h HAVE_ASSERT_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(float.h HAVE_FLOAT_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(math.h HAVE_MATH_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdarg.h HAVE_STDARG_H)
check_include_files(stddef.h HAVE_STDDEF_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(sys/param.h HAVE_SYS_PARAM_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_files(sys/wait.h HAVE_SYS_WAIT_H)

check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(machine/endian.h HAVE_MACHINE_ENDIAN_H)

if(HAVE_STDLIB_H AND HAVE_STDARG_H AND HAVE_STRING_H AND HAVE_FLOAT_H)
  set(STDC_HEADERS 1)
endif()

include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckCXXSymbolExists)

# Mostly Windows functions.
check_function_exists(_mkdir HAVE__MKDIR)
check_cxx_symbol_exists(_snprintf cstdio HAVE__SNPRINTF)
check_cxx_symbol_exists(stricmp cstring HAVE_STRICMP)
check_cxx_symbol_exists(_stricmp cstring HAVE__STRICMP)

# Mostly non-Windows functions.
check_function_exists(fcntl HAVE_FCNTL)
check_function_exists(fork HAVE_FORK)
check_function_exists(mkdir HAVE_MKDIR)
check_cxx_symbol_exists(snprintf cstdio HAVE_SNPRINTF)
check_cxx_symbol_exists(strcasecmp cstring HAVE_STRCASECMP)
check_function_exists(waitpid HAVE_WAITPID)

# Mostly universal symbols.
check_cxx_symbol_exists(strtof cstdlib HAVE_STRTOF)
check_symbol_exists(M_PI math.h HAVE_M_PI)
check_symbol_exists(posix_fadvise fcntl.h HAVE_POSIX_FADVISE)

# Checks to make it easier to work with 32-bit/64-bit builds if required.
include(CheckTypeSize)
check_type_size(pid_t SIZEOF_PID_T)
check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)


if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(SM_WIN32_ARCH "x64")
  else()
    set(SM_WIN32_ARCH "x86")
  endif()
endif()

check_compile_features("${SM_CMAKE_DIR}/TestCode"
                       "${SM_CMAKE_DIR}/TestCode/test_external.c"
                       "Checking for external name shortening requirements"
                       "not needed"
                       "needed"
                       SM_BUILT_LONG_NAME
                       TRUE)

if(NOT SM_BUILT_LONG_NAME)
  set(NEED_SHORT_EXTERNAL_NAMES 1)
endif()

check_compile_features("${SM_CMAKE_DIR}/TestCode"
                       "${SM_CMAKE_DIR}/TestCode/test_broken.c"
                       "Checking if incomplete types are broken."
                       "not broken"
                       "broken"
                       SM_BUILT_INCOMPLETE_TYPE
                       FALSE)

if(SM_BUILT_INCOMPLETE_TYPE)
  set(INCOMPLETE_TYPES_BROKEN 1)
endif()

# Dependencies go here.
include(ExternalProject)

find_package(nasm)
find_package(yasm)
find_package(Iconv)

find_package(Threads)
if(${Threads_FOUND})
  set(HAS_PTHREAD TRUE)
  list(APPEND CMAKE_REQUIRED_LIBRARIES pthread)
  check_symbol_exists(pthread_mutex_timedlock pthread.h
                      HAVE_PTHREAD_MUTEX_TIMEDLOCK)
  check_symbol_exists(pthread_cond_timedwait pthread.h
                      HAVE_PTHREAD_COND_TIMEDWAIT)
else()
  set(HAS_PTHREAD FALSE)
endif()

if(WIN32)
  # FFMPEG...it can be evil.
  find_library(LIB_SWSCALE
               NAMES "swscale"
               PATHS "${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}"
               NO_DEFAULT_PATH)
  get_filename_component(LIB_SWSCALE ${LIB_SWSCALE} NAME)

  find_library(LIB_AVCODEC
               NAMES "avcodec"
               PATHS "${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}"
               NO_DEFAULT_PATH)
  get_filename_component(LIB_AVCODEC ${LIB_AVCODEC} NAME)

  find_library(LIB_AVFORMAT
               NAMES "avformat"
               PATHS "${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}"
               NO_DEFAULT_PATH)
  get_filename_component(LIB_AVFORMAT ${LIB_AVFORMAT} NAME)

  find_library(LIB_AVUTIL
               NAMES "avutil"
               PATHS "${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}"
               NO_DEFAULT_PATH)
  get_filename_component(LIB_AVUTIL ${LIB_AVUTIL} NAME)

  list(APPEND SM_FFMPEG_WIN32_DLLS
    "avcodec-59.dll"
    "avformat-59.dll"
    "avutil-57.dll"
    "swscale-6.dll"
  )
  foreach(dll ${SM_FFMPEG_WIN32_DLLS})
    file(REMOVE "${SM_PROGRAM_DIR}/${dll}")
    file(COPY "${SM_EXTERN_DIR}/ffmpeg-w32/${SM_WIN32_ARCH}/${dll}" DESTINATION "${SM_PROGRAM_DIR}/")
  endforeach()
elseif(MACOSX)
  include("${SM_CMAKE_DIR}/SetupFfmpeg.cmake")

  set(WITH_CRASH_HANDLER TRUE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11")

  find_library(MAC_FRAME_ACCELERATE Accelerate ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_APPKIT AppKit ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_AUDIOTOOLBOX AudioToolbox
               ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_AUDIOUNIT AudioUnit ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_CARBON Carbon ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_COCOA Cocoa ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_COREAUDIO CoreAudio ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_COREFOUNDATION CoreFoundation
               ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_CORESERVICES CoreServices
               ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_FOUNDATION Foundation ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_IOKIT IOKit ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_OPENGL OpenGL ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_SYSTEM System ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)

  mark_as_advanced(MAC_FRAME_ACCELERATE
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
                   MAC_FRAME_SYSTEM)

  find_library(MAC_FRAME_COREMEDIA CoreMedia ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_COREVIDEO CoreVideo ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)
  find_library(MAC_FRAME_VIDEOTOOLBOX VideoToolbox ${CMAKE_SYSTEM_FRAMEWORK_PATH} REQUIRED)

  if(NOT YASM_FOUND AND NOT NASM_FOUND)
    message(FATAL_ERROR
      "Neither NASM nor YASM were found. Please install at least one of them."
    )
  endif()
elseif(LINUX OR BSD)
  if(WITH_GTK3)
    find_package("GTK3" 2.0)
    if(${GTK3_FOUND})
      set(HAS_GTK3 TRUE)
    else()
      set(HAS_GTK3 FALSE)
      message(
        "GTK3 was not found on your system. There will be no loading window.")
    endif()
  else()
    set(HAS_GTK3 FALSE)
  endif()

  set(HAS_X11 FALSE)
  if(WITH_X11)
    find_package(X11 REQUIRED)
    set(HAS_X11 TRUE)
  endif()

  set(HAS_XRANDR FALSE)
  if(WITH_XRANDR)
    find_package(Xrandr REQUIRED)
    set(HAS_XRANDR TRUE)
  endif()

  set(HAS_LIBXTST FALSE)
  if(WITH_LIBXTST)
    find_package(Xtst REQUIRED)
    set(HAS_LIBXTST TRUE)
  endif()

  set(HAS_XINERAMA FALSE)
  if(WITH_XINERAMA)
    find_package(Xinerama REQUIRED)
    set(HAS_XINERAMA TRUE)
  endif()

  set(HAS_PULSE FALSE)
  if(WITH_PULSEAUDIO)
    find_package(PulseAudio REQUIRED)
    set(HAS_PULSE TRUE)
  endif()

  set(HAS_ALSA FALSE)
  if(WITH_ALSA)
    find_package(ALSA REQUIRED)
    set(HAS_ALSA TRUE)
  endif()

  set(HAS_JACK FALSE)
  if(WITH_JACK)
    find_package(JACK REQUIRED)
    set(HAS_JACK TRUE)
  endif()

  set(HAS_OSS FALSE)
  if(WITH_OSS)
    find_package(OSS)
    set(HAS_OSS TRUE)
  endif()

  if( NOT (HAS_OSS OR HAS_JACK OR HAS_ALSA OR HAS_PULSE) )
    message(
      FATAL_ERROR
        "No sound libraries found (or selected). You will require at least one."
      )
  else()
    message(
      STATUS
        "-- At least one sound library was found. Do not worry if any were not found at this stage."
      )
  endif()

  if(NOT YASM_FOUND AND NOT NASM_FOUND)
    message(FATAL_ERROR
      "Neither NASM nor YASM were found. Please install at least one of them."
    )
  endif()

  include("${SM_CMAKE_DIR}/SetupFfmpeg.cmake")

  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL REQUIRED)

  find_package(Libusb)
  if(NOT LIBUSB_FOUND)
    message(FATAL_ERROR "libusb was not found.")
  endif()
endif(WIN32) # LINUX OR BSD, APPLE

configure_file("${SM_SRC_DIR}/config.in.hpp"
               "${SM_GENERATED_SRC_DIR}/config.hpp")
configure_file("${SM_SRC_DIR}/verstub.in.cpp"
               "${SM_GENERATED_SRC_DIR}/verstub.cpp")

# Define installer based items for cpack.
include("${CMAKE_CURRENT_LIST_DIR}/CMake/CPackSetup.cmake")
