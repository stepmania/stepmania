
find_path(LIBXTST_INCLUDE_DIR NAMES X11/extensions/XTest.h
          PATHS /opt/X11/include
          PATH_SUFFIXES X11/extensions
          DOC "The libXtst include directory"
)

find_library(LIBXTST_LIBRARY NAMES Xtst
          PATHS /opt/X11/lib
          DOC "The libXtst library"
)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBXTST DEFAULT_MSG LIBXTST_LIBRARY LIBXTST_INCLUDE_DIR)

mark_as_advanced(LIBXTST_INCLUDE_DIR LIBXTST_LIBRARY)

