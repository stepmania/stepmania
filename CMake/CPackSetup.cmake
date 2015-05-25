set(CPACK_PACKAGE_NAME "${SM_EXE_NAME}")
set(CPACK_PACKAGE_VENDOR "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")
set(CPACK_PACKAGE_VERSION_MAJOR "${SM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SM_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${SM_VERSION_TRADITIONAL}")
set(CPACK_NSIS_HELP_LINK "https://github.com/stepmania/stepmania/issues")
set(CPACK_NSIS_URL_INFO_ABOUT "http://www.stepmania.com/")
set(CPACK_RESOURCE_FILE_README "${SM_ROOT_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${SM_CMAKE_DIR}/license_install.txt")
set(CPACK_PACKAGE_EXECUTABLES
  "${SM_EXE_NAME}" "StepMania ${SM_VERSION_MAJOR}" 
)

if(WIN32)

  # By setting these install keys manually,
  # The default directory of "StepMania major.minor.patch" is lost.
  # This is currently done to maintain backwards compatibility.
  # However, removing these two will allow for multiple versions of StepMania
  # to be installed relatively cleanly.
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "StepMania ${SM_VERSION_MAJOR}")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "StepMania ${SM_VERSION_MAJOR}")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "Program")
  set(CPACK_NSIS_INSTALL_ROOT "C:\\\\Games")
endif()

include(CPack)
