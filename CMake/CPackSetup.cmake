if(WITH_FULL_RELEASE)
  set(NSIS_VERSION_FINAL "${SM_VERSION_TRADITIONAL}")
else()
  set(NSIS_VERSION_FINAL "${SM_VERSION_GIT}")
endif()

set(CPACK_PACKAGE_NAME "${SM_EXE_NAME}")
set(CPACK_PACKAGE_VENDOR "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")
set(CPACK_PACKAGE_VERSION_MAJOR "${SM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SM_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${SM_VERSION_TRADITIONAL}")
set(CPACK_NSIS_HELP_LINK "https://github.com/stepmania/stepmania/issues")
set(CPACK_NSIS_PACKAGE_NAME "${SM_EXE_NAME} ${NSIS_VERSION_FINAL}")
set(CPACK_NSIS_URL_INFO_ABOUT "http://www.stepmania.com/")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_RESOURCE_FILE_README "${SM_ROOT_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${SM_CMAKE_DIR}/license_install.txt")
set(CPACK_PACKAGE_EXECUTABLES "${SM_EXE_NAME}" "StepMania ${SM_VERSION_MAJOR}")
set(CPACK_NSIS_MUI_ICON "${SM_INSTALLER_DIR}/install.ico")
set(CPACK_NSIS_MUI_UNIICON "${SM_INSTALLER_DIR}/uninstall.ico")
set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")

# Custom items for nsis go here.
set(CPACK_SM_NSIS_REPOSITORY "https://github.com/stepmania/stepmania")
set(CPACK_SM_NSIS_ROOT_DIR "${SM_ROOT_DIR}")
set(CPACK_SM_NSIS_PRODUCT_ID
    "StepMania ${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}")
set(CPACK_SM_NSIS_PRODUCT_VERSION "${SM_VERSION_TRADITIONAL}.0")
set(CPACK_SM_NSIS_HEADER_BITMAP "${SM_INSTALLER_DIR}/header-sm5.bmp")
set(CPACK_SM_NSIS_WELCOME_BITMAP "${SM_INSTALLER_DIR}/welcome-sm5.bmp")
set(CPACK_SM_NSIS_GIT_VERSION "${SM_VERSION_GIT}")

if(WIN32)
  # The header and welcome bitmaps require backslashes.
  string(REGEX
         REPLACE "/"
                 "\\\\\\\\"
                 CPACK_SM_NSIS_HEADER_BITMAP
                 "${CPACK_SM_NSIS_HEADER_BITMAP}")
  string(REGEX
         REPLACE "/"
                 "\\\\\\\\"
                 CPACK_SM_NSIS_WELCOME_BITMAP
                 "${CPACK_SM_NSIS_WELCOME_BITMAP}")

  set(CPACK_PACKAGE_FILE_NAME "${SM_EXE_NAME}-${NSIS_VERSION_FINAL}-win32")
  # By setting these install keys manually, The default directory of "StepMania
  # major.minor.patch" is lost. This is currently done to maintain backwards
  # compatibility. However, removing these two will allow for multiple versions
  # of StepMania to be installed relatively cleanly.
  set(CPACK_PACKAGE_INSTALL_DIRECTORY
      "StepMania ${SM_VERSION_MAJOR}.${SM_VERSION_MINOR}")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "StepMania ${SM_VERSION_MAJOR}")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "Program")
  set(CPACK_NSIS_INSTALL_ROOT "C:\\\\Games")
endif()

include(CPack)
