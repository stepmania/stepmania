set(CPACK_PACKAGE_NAME "${SM_EXE_NAME}")
set(CPACK_PACKAGE_VENDOR "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")
set(CPACK_PACKAGE_VERSION_MAJOR "${SM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SM_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${SM_VERSION_TRADITIONAL}")
include(CPack)
