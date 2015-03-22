# Copied from marsyas, which is also copied from fqterm.
# Further modifications are done.

IF(UNIX)
  IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(OSS_HDR_NAME "linux/soundcard.h")
  ELSE(CMAKE_SYSTEM_NAME MATCHES "Linux")
    IF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
      SET(OSS_HDR_NAME "sys/soundcard.h")
    ENDIF(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  ENDIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
ENDIF(UNIX)

FIND_PATH(OSS_INCLUDE_DIR "${OSS_HDR_NAME}"
  "/usr/include" "/usr/local/include"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSS DEFAULT_MSG OSS_INCLUDE_DIR)

MARK_AS_ADVANCED (
  OSS_FOUND
  OSS_INCLUDE_DIR
)
