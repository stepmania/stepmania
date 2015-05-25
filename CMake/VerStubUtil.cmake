include("${CMAKE_CURRENT_LIST_DIR}/SMDefs.cmake")

string(TIMESTAMP SM_TIMESTAMP_DATE "%Y%m%d")
string(TIMESTAMP SM_TIMESTAMP_TIME "%H:%M:%S" UTC)
configure_file(
    "${SM_SRC_DIR}/version_updater/verstub.cpp.in"
    "${SM_SRC_DIR}/generated/verstub.cpp"
)