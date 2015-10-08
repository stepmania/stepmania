# Stick to traditional approaches here. The following are defined.

# DL_FOUND - The system has the dl library.
# DL_INCLUDE_DIR - The dl include directory.
# DL_LIBRARIES - The library file to link to.

if (DL_INCLUDE_DIR AND DL_LIBRARIES)
  # Already in cache, so don't repeat the finding procedures.
  set(DL_FIND_QUIETLY TRUE)
endif()

find_path(DL_INCLUDE_DIR dlfcn.h
  PATHS /usr/local/include /usr/include
)

find_library(DL_LIBRARIES dl
  PATHS /usr/local/lib /usr/lib /lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DL DEFAULT_MSG DL_LIBRARIES DL_INCLUDE_DIR)

mark_as_advanced(DL_INCLUDE_DIR DL_LIBRARIES)

