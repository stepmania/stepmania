# Find pcre using standard tools.

# The following will be set:

# PCRE_INCLUDE_DIR PCRE_LIBRARY PCRE_FOUND

find_path(PCRE_INCLUDE_DIR NAMES pcre.h)
find_library(PCRE_LIBRARY NAMES pcre)

# Properly pass QUIETLY and REQUIRED.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE
                                  DEFAULT_MSG
                                  PCRE_INCLUDE_DIR
                                  PCRE_LIBRARY)

mark_as_advanced(PCRE_INCLUDE_DIR PCRE_LIBRARY)
