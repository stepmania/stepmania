if (NOT MSVC)
  return()
endif()

set(MAPCONV_DIR "${SM_SRC_DIR}/archutils/Win32")

list(APPEND MAPCONV_SRC
  "${MAPCONV_DIR}/mapconv.cpp"
)

source_group("" FILES ${MAPCONV_SRC})

add_executable("mapconv" ${MAPCONV_SRC})

set_property(TARGET "mapconv" PROPERTY FOLDER "Internal Libraries")

disable_project_warnings("mapconv")

set_target_properties("mapconv" PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${SM_PROGRAM_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SM_PROGRAM_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SM_PROGRAM_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${SM_PROGRAM_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${SM_PROGRAM_DIR}"
)

set_target_properties("mapconv" PROPERTIES
  OUTPUT_NAME "mapconv"
  RELEASE_OUTPUT_NAME "mapconv"
  DEBUG_OUTPUT_NAME "mapconv"
  MINSIZEREL_OUTPUT_NAME "mapconv"
  RELWITHDEBINFO_OUTPUT_NAME "mapconv"
)