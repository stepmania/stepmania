# From the CMake wiki, get the DirectX version needed.
# This assumes default directories.

# Once loaded, the following are defined:
#  DIRECTX_FOUND
#  DIRECTX_INCLUDE_DIR
#  DIRECTX_LIBRARIES

if(NOT WIN32)
  return()
endif()

set(DIRECTX_INCLUDE_SEARCH_PATHS
  # TODO: Do not be limited to x86 in the future.
  "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
  "C:/DXSDK/Include"
)

set(DIRECTX_LIBRARY_SEARCH_PATHS
  # TODO: Do not be limited to x86 in the future.
  "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"
  "C:/DXSDK/Include/Lib/x86"
)

find_path(DIRECTX_INCLUDE_DIR
  NAMES "DxErr.h"
  PATHS ${DIRECTX_INCLUDE_SEARCH_PATHS}
  DOC "Where can DxErr.h be found?"
)

find_library(DIRECTX_LIBRARIES
  NAMES "DxErr.lib" "d3dx9.lib"
  PATHS ${DIRECTX_LIBRARY_SEARCH_PATHS}
  DOC "Where can the DX libraries be found?"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DIRECTX DEFAULT_MSG DIRECTX_INCLUDE_DIR DIRECTX_LIBRARIES)

if(DIRECTX_FOUND)
  mark_as_advanced(DIRECTX_INCLUDE_DIR DIRECTX_LIBRARIES)
endif()
