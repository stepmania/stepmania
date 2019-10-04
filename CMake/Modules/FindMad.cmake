find_path(LIBMAD_INCLUDE_DIR mad.h)

find_library(LIBMAD_LIBRARY mad)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBMAD
                                  DEFAULT_MSG
                                  LIBMAD_LIBRARY
                                  LIBMAD_INCLUDE_DIR)

mark_as_advanced(LIBMAD_LIBRARY LIBMAD_INCLUDE_DIR)
