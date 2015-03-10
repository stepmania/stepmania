# Hopefully find the Open Sound System, or OSS.
# Anyone looking for Open Source Software is in the wrong file, but the right repository. ;)

find_path(OSS_INCLUDE_DIR "sys/soundcard.h")
set(OSS_LIBRARIES TRUE)
mark_as_advanced(OSS_INCLUDE_DIR)

# Pass QUIETLY and REQUIRED as normal. Will set OSS_FOUND to TRUE if everything works.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OSS DEFAULT_MSG OSS_LIBRARIES OSS_INCLUDE_DIR)
