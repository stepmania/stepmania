; Included by the NSIS installer script
; Don't forget to also change ProductInfo.h!

; a friendly string to refer to the product
!define PRODUCT_FAMILY "StepMania"

; PRODUCT_ID must NOT be "StepMania" unless you want people to uninstall 3.9
; when they install StepMania 5. (not recommended)
!define PRODUCT_ID "StepMania 5.1"
; TODO: This needs to be updated with the git rev hash
!define PRODUCT_VER "5.1-UNKNOWN"
!define PRODUCT_DISPLAY "${PRODUCT_FAMILY} ${PRODUCT_VER}"
!define PRODUCT_BITMAP "sm5"

!define PRODUCT_URL "https://www.stepmania.com/"
!define UPDATES_URL "https://www.stepmania.com/download/"

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
