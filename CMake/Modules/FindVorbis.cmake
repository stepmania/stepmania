# Borrowed from code.openhub.net

# Base Io build system Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libvorbis.

find_path(VORBIS_INCLUDE_DIR vorbis/codec.h)

set(VORBIS_NAMES ${VORBIS_NAMES} vorbis libvorbis)
find_library(VORBIS_LIBRARY NAMES ${VORBIS_NAMES} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBIS
                                  DEFAULT_MSG
                                  VORBIS_LIBRARY
                                  VORBIS_INCLUDE_DIR)

mark_as_advanced(VORBIS_LIBRARY VORBIS_INCLUDE_DIR)
