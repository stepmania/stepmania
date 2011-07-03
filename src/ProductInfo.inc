; Included by the NSIS installer script
; Don't forget to also change ProductInfo.h!

!define PRODUCT_FAMILY "StepMania"

; see ProductInfo.h for use descriptions
!define PRODUCT_ID "StepMania"
!define PRODUCT_VER "v5.0 Preview 2"
!define PRODUCT_DISPLAY "${PRODUCT_ID} ${PRODUCT_VER}"
!define PRODUCT_BITMAP "sm5"

!define PRODUCT_URL "http://www.stepmania.com/"
!define UPDATES_URL "http://www.stepmania.com/"

;!define INSTALL_EXTERNAL_PCKS
;!define INSTALL_INTERNAL_PCKS
!define INSTALL_PROGRAM_LIBRARIES
!define INSTALL_EXECUTABLES
!define INSTALL_NON_PCK_FILES
!define ASSOCIATE_SMZIP
!define ASSOCIATE_SMURL
;!define SHOW_AUTORUN
!define MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
!define MAKE_OPEN_SETTINGS_FOLDER_SHORTCUT
;!define DIRECTX_81_REDIST_PRESENT
;!define MAKE_UPDATES_SHORTCUT
!define CRC_CHECK
!define COMPRESS
