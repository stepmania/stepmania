; Included by the NSIS installer script
; Don't forget to also change ProductInfo.h!

; a friendly string to refer to the product
!define PRODUCT_FAMILY "StepMania"

; PRODUCT_ID must NOT be "StepMania" unless you want people to uninstall 3.9
; when they install StepMania 5. (not recommended)
!define PRODUCT_ID "StepMania 5"
!define PRODUCT_VER "v5.0 beta 1a"
!define PRODUCT_DISPLAY "${PRODUCT_FAMILY} ${PRODUCT_VER}"
!define PRODUCT_BITMAP "sm5"

!define PRODUCT_URL "http://www.stepmania.com/"
!define UPDATES_URL "http://code.google.com/p/sm-ssc/downloads/list"

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
