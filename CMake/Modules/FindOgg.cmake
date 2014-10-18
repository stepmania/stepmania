# Borrowed from code.openhub.net

# Base Io build system
# Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libogg.

FIND_PATH(OGG_INCLUDE_DIR ogg/ogg.h)

SET(OGG_NAMES ${OGG_NAMES} ogg libogg)
FIND_LIBRARY(OGG_LIBRARY NAMES ${OGG_NAMES} PATH)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OGG DEFAULT_MSG OGG_LIBRARY OGG_INCLUDE_DIR)

mark_as_advanced(OGG_LIBRARY OGG_INCLUDE_DIR)

