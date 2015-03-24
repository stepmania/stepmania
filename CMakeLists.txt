cmake_minimum_required(VERSION 2.8.12)

project(StepMania)

# Include the macros and functions.
include(CMake/CMakeMacros.cmake)

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
else()
  set(BSD FALSE)
endif()

# Some eventual distribution stuff.
set(CPACK_PACKAGE_NAME "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")

# Prep options that are needed for each platform.
option(WITH_NETWORKING "Build with networking support." ON)

# This option is not yet working, but will likely default to ON in the future.
option(WITH_LTO "Build with Link Time Optimization (LTO)/Whole Program Optimization." OFF)

option(WITH_SSE2 "Build with SSE2 Optimizations." ON)

# This option may go away in the future: if it does, JPEG will always be required.
option(WITH_JPEG "Build with JPEG Image Support." ON)

# Turn this on to set this to a specific release mode.
option(WITH_FULL_RELEASE "Build as a proper, full release." OFF)

# Turn this option off to not use the GPL exclusive components.
option(WITH_GPL_LIBS "Build with GPL libraries." ON)

if(WIN32)
  option(WITH_MINIMAID "Build with Mimimaid Lights Support." OFF)
else()
  if(LINUX)
    option(WITH_FFMPEG "Build with FFMPEG." ON)
    # Builder beware: later versions of ffmpeg may break!
    option(WITH_SYSTEM_FFMPEG "Build with the system's FFMPEG." OFF)
    option(WITH_TTY "Build with Linux TTY Input Support." OFF)
    option(WITH_PROFILING "Build with Profiling Support." OFF)
    option(WITH_GLES2 "Build with OpenGL ES 2.0 Support." ON)
    option(WITH_GTK2 "Build with GTK2 Support." ON)
    option(WITH_OGG "Build with OGG/Vorbis Support." ON)
    option(WITH_MP3 "Build with MP3 Support." ON)
    option(WITH_PARALLEL_PORT "Build with Parallel Lights I/O Support." OFF)
    option(WITH_CRASH_HANDLER "Build with Crash Handler Support." ON)
  endif()
endif()

# Set up version numbers according to the new scheme.
set(SM_VERSION_MAJOR 5)
set(SM_VERSION_MINOR 0)
set(SM_VERSION_PATCH 6)
set(SM_VERSION_TRADITIONAL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")

if(WITH_FULL_RELEASE EQUAL ON)
  set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}.${SM_VERSION_PATCH}")
else()
  # Using the deprecated version for now to keep cmake 2.8 users happy.
  exec_program(git "${SM_ROOT_DIR}"
    ARGS "rev-parse" "--short" "HEAD"
    OUTPUT_VARIABLE SM_VERSION_GIT_HASH
    RETURN_VALUE ret
  )

  if(NOT (ret STREQUAL "0"))
    set(SM_VERSION_GIT_HASH "UNKNOWN")
    set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-${SM_VERSION_GIT_HASH}")
  else()
    set(SM_VERSION_FULL "${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}-git-${SM_VERSION_GIT_HASH}")
  endif()
endif()

# Allow for finding our libraries in a standard location.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/")

# Put the predefined targets in separate groups.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Dependencies go here.
set(ENDIANNESS "ENDIAN_LITTLE")
if(WIN32)
  set(HAS_OGG TRUE)
  set(HAS_MP3 TRUE)
  set(SYSTEM_PCRE_FOUND FALSE)
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
  set(HAS_OGG TRUE)
  set(HAS_MP3 TRUE)
  set(SYSTEM_PCRE_FOUND FALSE)
  set(WITH_CRASH_HANDLER TRUE)
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

  if(NOT WITH_GPL_LIBS)
    message("Disabling GPL exclusive libraries: no MP3 support.")
    set(WITH_MP3 OFF)
  endif()

  if(WITH_GTK2)
    find_package("GTK2" 2.0)
    if (${GTK2_FOUND})
      set(HAS_GTK2 TRUE)
    else()
      set(HAS_GTK2 FALSE)
      message("GTK2 was not found on your system. There will be no loading window.")
    endif()
  else()
    set(HAS_GTK2 FALSE)
  endif()

  find_package(X11)

  find_package(Pcre)
  set(SYSTEM_PCRE_FOUND ${PCRE_FOUND})

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

  if (WITH_OGG)
    find_package(Ogg)
    find_package(Vorbis)
    find_package(VorbisFile)

    if(NOT (OGG_FOUND AND VORBIS_FOUND AND VORBISFILE_FOUND) )
      message(FATAL_ERROR "Not all vorbis libraries were found. If you wish to skip vorbis support, set WITH_OGG to OFF when configuring.")
    else()
      set(HAS_OGG TRUE)
    endif()
  endif()

  if (WITH_MP3)
    find_package(Mad)
    if(NOT LIBMAD_FOUND)
      message(FATAL_ERROR "Libmad library not found. If you wish to skip mp3 support, set WITH_MP3 to OFF when configuring.")
    else()
      set(HAS_MP3 TRUE)
    endif()
  endif()

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
  else()
    message(STATUS "-- At least one sound library was found. Do not worry if any were not found at this stage.")
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
    if (WITH_SYSTEM_FFMPEG)
      find_package("FFMPEG")
      if(NOT FFMPEG_FOUND)
        message(FATAL_ERROR "System ffmpeg not found! Either install the libraries or remove the argument, then try again.")
      else()

        message(STATUS "-- Warning! Your version of ffmpeg may be too high! If you want to use the system ffmpeg, clear your cmake cache and do not include the system ffmpeg argument.")
        set(HAS_FFMPEG TRUE)
      endif()
    else()
      list(APPEND FFMPEG_CONFIGURE
        "${SM_EXTERN_DIR}/ffmpeg-linux-2.1.3/configure"
        "--disable-programs"
        "--disable-doc"
        "--disable-avdevice"
        "--disable-swresample"
        "--disable-postproc"
        "--disable-avfilter"
        "--disable-shared"
        "--enable-static"
      )
      if(WITH_GPL_LIBS)
        list(APPEND FFMPEG_CONFIGURE
          "--enable-gpl"
        )
      endif()
      # --shlibdir=$our_installdir/stepmania-$VERSION
      externalproject_add("ffmpeg"
        # DOWNLOAD_DIR "${SM_EXTERN_DIR}/ffmpeg-linux"
        # PREFIX "${SM_EXTERN_DIR}/ffmpeg-linux"
        # GIT_REPOSITORY "git://source.ffmpeg.org/ffmpeg.git"
        # GIT_TAG "n2.1.3"
        DOWNLOAD_COMMAND git clone "--branch" "n2.1.3" "--depth" "1" "git://source.ffmpeg.org/ffmpeg.git" "${SM_EXTERN_DIR}/ffmpeg-linux-2.1.3"
        CONFIGURE_COMMAND ${FFMPEG_CONFIGURE}
        BUILD_COMMAND "make"
        UPDATE_COMMAND ""
        INSTALL_COMMAND ""
        TEST_COMMAND ""
      )
      set(HAS_FFMPEG TRUE)
    endif()
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

  test_big_endian(BIGENDIAN)
  if (${BIGENDIAN})
    set(ENDIANNESS "ENDIAN_BIG")
  endif()

endif()


# The external libraries need to be included.
add_subdirectory(extern)

# The internal libraries and eventual executable to be used.
add_subdirectory(src)

