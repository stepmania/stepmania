# - Find libusb for portable USB support
# This module will find libusb as published by
#  http://libusb.sf.net and
#  http://libusb-win32.sf.net
# 
# It will use PkgConfig if present and supported, else search
# it on its own. If the LibUSB_ROOT_DIR environment variable
# is defined, it will be used as base path.
# The following standard variables get defined:
#  LibUSB_FOUND:        true if LibUSB was found
#  LibUSB_INCLUDE_DIRS: the directory that contains the include file
#  LibUSB_LIBRARIES:    the library
if( WIN32 )
  return()
endif()

include ( CheckLibraryExists )
include ( CheckIncludeFile )

find_package ( PkgConfig )
if ( PKG_CONFIG_FOUND )
  pkg_check_modules ( PKGCONFIG_LIBUSB libusb )
endif ( PKG_CONFIG_FOUND )

if ( PKGCONFIG_LIBUSB_FOUND )
  set ( LibUSB_FOUND ${PKGCONFIG_LIBUSB_FOUND} )
  set ( LibUSB_INCLUDE_DIRS ${PKGCONFIG_LIBUSB_INCLUDE_DIRS} )
  foreach ( i ${PKGCONFIG_LIBUSB_LIBRARIES} )
    find_library ( ${i}_LIBRARY
      NAMES ${i}
      PATHS ${PKGCONFIG_LIBUSB_LIBRARY_DIRS}
    )
    if ( ${i}_LIBRARY )
      list ( APPEND LibUSB_LIBRARIES ${${i}_LIBRARY} )
    endif ( ${i}_LIBRARY )
    mark_as_advanced ( ${i}_LIBRARY )
  endforeach ( i )

else ( PKGCONFIG_LIBUSB_FOUND )
  find_path ( LibUSB_INCLUDE_DIRS
    NAMES
      usb.h
    PATHS
      $ENV{ProgramFiles}/LibUSB-Win32
      $ENV{LibUSB_ROOT_DIR}
    PATH_SUFFIXES
      include
  )
  mark_as_advanced ( LibUSB_INCLUDE_DIRS )
#  message ( STATUS "LibUSB include dir: ${LibUSB_INCLUDE_DIRS}" )

  if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )
    # LibUSB-Win32 binary distribution contains several libs.
    # Use the lib that got compiled with the same compiler.
    if ( MSVC )
      if ( WIN32 )
        set ( LibUSB_LIBRARY_PATH_SUFFIX lib/msvc )
      else ( WIN32 )
        set ( LibUSB_LIBRARY_PATH_SUFFIX lib/msvc_x64 )
      endif ( WIN32 )          
    elseif ( BORLAND )
      set ( LibUSB_LIBRARY_PATH_SUFFIX lib/bcc )
    elseif ( CMAKE_COMPILER_IS_GNUCC )
      set ( LibUSB_LIBRARY_PATH_SUFFIX lib/gcc )
    endif ( MSVC )
  endif ( ${CMAKE_SYSTEM_NAME} STREQUAL "Windows" )

  find_library ( usb_LIBRARY
    NAMES
      libusb usb
    PATHS
      $ENV{ProgramFiles}/LibUSB-Win32
      $ENV{LibUSB_ROOT_DIR}
    PATH_SUFFIXES
      ${LibUSB_LIBRARY_PATH_SUFFIX}
  )
  mark_as_advanced ( usb_LIBRARY )
  if ( usb_LIBRARY )
    set ( LibUSB_LIBRARIES ${usb_LIBRARY} )
  endif ( usb_LIBRARY )

  if ( LibUSB_INCLUDE_DIRS AND LibUSB_LIBRARIES )
    set ( LibUSB_FOUND true )
  endif ( LibUSB_INCLUDE_DIRS AND LibUSB_LIBRARIES )
endif ( PKGCONFIG_LIBUSB_FOUND )

if ( LibUSB_FOUND )
  set ( CMAKE_REQUIRED_INCLUDES "${LibUSB_INCLUDE_DIRS}" )
  check_include_file ( usb.h LibUSB_FOUND )
#    message ( STATUS "LibUSB: usb.h is usable: ${LibUSB_FOUND}" )
endif ( LibUSB_FOUND )
if ( LibUSB_FOUND )
  check_library_exists ( "${LibUSB_LIBRARIES}" usb_open "" LibUSB_FOUND )
#    message ( STATUS "LibUSB: library is usable: ${LibUSB_FOUND}" )
endif ( LibUSB_FOUND )

if ( NOT LibUSB_FOUND )
  if ( NOT LibUSB_FIND_QUIETLY )
    message ( STATUS "LibUSB not found, try setting LibUSB_ROOT_DIR environment variable." )
  endif ( NOT LibUSB_FIND_QUIETLY )
  if ( LibUSB_FIND_REQUIRED )
    message ( FATAL_ERROR "" )
  endif ( LibUSB_FIND_REQUIRED )
endif ( NOT LibUSB_FOUND )
#message ( STATUS "LibUSB: ${LibUSB_FOUND}" )