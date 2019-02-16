# Find the Video Acceleration library.

# The following will be set:

# VA_INCLUDE_DIR VA_LIBRARY VA_FOUND

find_path(VA_INCLUDE_DIR NAMES va/va.h)

set(VA_NAMES "${VA_NAMES}" "va" "libva")
find_library(VA_LIBRARY NAMES ${VA_NAMES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(VA DEFAULT_MSG VA_LIBRARY VA_INCLUDE_DIR)

mark_as_advanced(VA_LIBRARY VA_INCLUDE_DIR)
