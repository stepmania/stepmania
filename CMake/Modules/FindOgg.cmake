# Borrowed from code.openhub.net

# Base Io build system Written by Jeremy Tregunna <jeremy.tregunna@me.com>
#
# Find libogg.

find_path(OGG_INCLUDE_DIR ogg/ogg.h)

set(OGG_NAMES ${OGG_NAMES} ogg libogg)
find_library(OGG_LIBRARY NAMES ${OGG_NAMES} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OGG DEFAULT_MSG OGG_LIBRARY OGG_INCLUDE_DIR)

mark_as_advanced(OGG_LIBRARY OGG_INCLUDE_DIR)
