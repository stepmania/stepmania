# Borrowed from code.openhub.net

# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libvorbis.

FIND_PATH(VORBIS_INCLUDE_DIR vorbis/codec.h)

SET(VORBIS_NAMES ${VORBIS_NAMES} vorbis libvorbis)
FIND_LIBRARY(VORBIS_LIBRARY NAMES ${VORBIS_NAMES} PATH)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VORBIS DEFAULT_MSG VORBIS_LIBRARY VORBIS_INCLUDE_DIR)

mark_as_advanced(VORBIS_LIBRARY VORBIS_INCLUDE_DIR)

