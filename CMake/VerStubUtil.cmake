include("${CMAKE_CURRENT_LIST_DIR}/SMDefs.cmake")

# SM_TIMESTAMP_DATE and SM_TIMESTAMP_TIME, needed for the stub, are
# prepared/generated as we want them in SMDefs
configure_file("${SM_SRC_DIR}/version_updater/verstub.cpp.in"
               "${SM_SRC_DIR}/generated/verstub.cpp")
