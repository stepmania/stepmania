# Inspired by Jeremy Tregunna's cmake work <jeremy.tregunna@me.com>
#
# Find libvorbisfile.

find_path(VORBISFILE_INCLUDE_DIR vorbis/vorbisfile.h)

set(VORBISFILE_NAMES ${VORBISFILE_NAMES} vorbisfile libvorbisfile)
find_library(VORBISFILE_LIBRARY NAMES ${VORBISFILE_NAMES} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VORBISFILE
                                  DEFAULT_MSG
                                  VORBISFILE_LIBRARY
                                  VORBISFILE_INCLUDE_DIR)

mark_as_advanced(VORBISFILE_LIBRARY VORBISFILE_INCLUDE_DIR)
