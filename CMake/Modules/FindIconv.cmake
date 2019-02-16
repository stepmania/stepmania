# Borrowed from https://github.com/onyx-
# intl/cmake_modules/blob/master/FindIconv.cmake

# Find Iconv on the system. When this is done, the following are defined:

# ICONV_FOUND - The system has Iconv. ICONV_INCLUDE_DIR - The Iconv include
# directory. ICONV_LIBRARIES - The library file to link to.

if(ICONV_INCLUDE_DIR AND ICONV_LIBRARIES)
  # Already in cache, so don't repeat the finding procedures.
  set(ICONV_FIND_QUIETLY TRUE)
endif()

find_path(ICONV_INCLUDE_DIR iconv.h)
set(ICONV_NAMES ${ICONV_NAMES} iconv libiconv libiconv-2 c)
find_library(ICONV_LIBRARIES NAMES ${ICONV_NAMES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ICONV
                                  DEFAULT_MSG
                                  ICONV_LIBRARIES
                                  ICONV_INCLUDE_DIR)

mark_as_advanced(ICONV_INCLUDE_DIR ICONV_LIBRARIES)
