# From the CMake wiki, get the DirectX version needed.
# This assumes default directories.

# Once loaded, the following will be defined:
#   DIRECTX_FOUND
#   DIRECTX_INCLUDE_DIR
#   DIRECTX_LIBRARY_DIR

set(DIRECTX_FOUND "NO")
set(DIRECTX_REQURIED "NO")

if(WIN32)
  set(DIRECTX_REQUIRED "YES")

  # Get the include directory
  find_path(DIRECTX_INCLUDE_DIR "DxErr.h"
    "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include"
    "C:/DSXDK/Include"
    doc "Where can DxErr.h be found?"
  )

  if(DIRECTX_INCLUDE_DIR)
    # TODO: Don't be limited to X86. Heck, don't need DirectX period.
    find_path(DIRECTX_LIBRARY_DIR "DxErr.lib"
      "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86"
      "C:/DSXDK/Include/Lib/x86"
      doc "Where can DxErr.lib be found?"
    )

    if (DIRECTX_LIBRARY_DIR)
      set(DIRECTX_FOUND "YES")
    endif(DIRECTX_LIBRARY_DIR)
  endif(DIRECTX_INCLUDE_DIR)

endif(WIN32)

if(NOT DIRECTX_FOUND)
  if(DIRECTX_REQUIRED)
    message(FATAL_ERROR "DirectX SDK June 2010 required. Install in a standard location.")
  endif(DIRECTX_REQUIRED)
endif(NOT DIRECTX_FOUND)
