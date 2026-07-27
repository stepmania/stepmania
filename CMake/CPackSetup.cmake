set(CPACK_PACKAGE_NAME "${SM_EXE_NAME}")
set(CPACK_PACKAGE_VENDOR "StepMania")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Rhythm Game Simulator")
set(CPACK_PACKAGE_VERSION_MAJOR "${SM_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${SM_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${SM_VERSION_PATCH}")
if(WITH_FULL_RELEASE)
  set(CPACK_PACKAGE_VERSION "${SM_VERSION_TRADITIONAL}")
else()
  set(CPACK_PACKAGE_VERSION "${SM_VERSION_GIT}")
endif()
set(CPACK_PACKAGE_EXECUTABLES "${SM_EXE_NAME}" "StepMania")
set(CPACK_RESOURCE_FILE_README "${SM_ROOT_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${SM_CMAKE_DIR}/license_install.txt")

if(WIN32)
  set(CPACK_GENERATOR NSIS)
  set(CPACK_SYSTEM_NAME "Windows")

  # By setting these install keys manually, The default directory of "StepMania
  # major.minor.patch" is lost. This is currently done to maintain backwards
  # compatibility. However, removing these two will allow for multiple versions
  # of StepMania to be installed relatively cleanly.
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "StepMania")
  set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "StepMania")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "Program")
  set(CPACK_NSIS_INSTALL_ROOT "C:\\\\Games")

  set(CPACK_NSIS_HELP_LINK "https://github.com/stepmania/stepmania/issues")
  set(CPACK_NSIS_PACKAGE_NAME "${SM_EXE_NAME} ${CPACK_PACKAGE_VERSION}")
  set(CPACK_NSIS_URL_INFO_ABOUT "https://www.stepmania.com/")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MUI_ICON "${SM_INSTALLER_DIR}/install.ico")
  set(CPACK_NSIS_MUI_UNIICON "${SM_INSTALLER_DIR}/uninstall.ico")
  set(CPACK_NSIS_MUI_HEADERIMAGE "${SM_INSTALLER_DIR}/header.bmp")
  set(CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP "${SM_INSTALLER_DIR}/welcome.bmp")
  set(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
  set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${SM_EXE_NAME}.exe")
  set(CPACK_NSIS_BRANDING_TEXT " ")

  # The header and welcome bitmaps require backslashes.
  string(REGEX
         REPLACE "/"
                 "\\\\\\\\"
                 CPACK_NSIS_MUI_HEADERIMAGE
                 "${CPACK_NSIS_MUI_HEADERIMAGE}")
  string(REGEX
         REPLACE "/"
                 "\\\\\\\\"
                 CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP
                 "${CPACK_NSIS_MUI_WELCOMEFINISHPAGE_BITMAP}")

  # Custom items for nsis go here.
  set(CPACK_SM_NSIS_PRODUCT_ID "StepMania")
  set(CPACK_SM_NSIS_PRODUCT_VERSION "${SM_VERSION_TRADITIONAL}.0")
  set(CPACK_SM_NSIS_GIT_VERSION "${SM_VERSION_GIT}")
elseif(MACOSX)
  set(CPACK_GENERATOR DragNDrop)
  set(CPACK_DMG_VOLUME_NAME "${CPACK_PACKAGE_NAME} ${CPACK_PACKAGE_VERSION}")
  set(CPACK_DMG_FORMAT ULMO)  # lzma-compressed image

  if(CMAKE_OSX_ARCHITECTURES STREQUAL "arm64")
    set(CPACK_SYSTEM_NAME "macOS-M1")
  elseif(CMAKE_OSX_ARCHITECTURES STREQUAL "x86_64")
    set(CPACK_SYSTEM_NAME "macOS-Intel")
  else()
    message(FATAL_ERROR
      "Unsupported macOS architecture: ${CMAKE_OSX_ARCHITECTURES}, set CMAKE_OSX_ARCHITECTURES to either arm64 or x86_64"
    )
  endif()
else()
  set(CPACK_GENERATOR TGZ)
  set(CPACK_SYSTEM_NAME "Linux")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
if(NOT WITH_CLUB_FANTASTIC)
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-no-songs")
endif()

include(CPack)
