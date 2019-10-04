# Copied from marsyas, which is also copied from fqterm. Further modifications
# are done.

if(UNIX)
  if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(OSS_HDR_NAME "linux/soundcard.h")
  else(CMAKE_SYSTEM_NAME MATCHES "Linux")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
      set(OSS_HDR_NAME "sys/soundcard.h")
    endif(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  endif(CMAKE_SYSTEM_NAME MATCHES "Linux")
endif(UNIX)

find_path(OSS_INCLUDE_DIR "${OSS_HDR_NAME}" "/usr/include" "/usr/local/include")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSS DEFAULT_MSG OSS_INCLUDE_DIR)

mark_as_advanced(OSS_FOUND OSS_INCLUDE_DIR)
