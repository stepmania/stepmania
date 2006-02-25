; Included by the NSIS installer script
; Don't forget to also change ProductInfo.h!

!define PRODUCT_FAMILY "StepMania"
; String used for the install directory and registry locations
; Official releases = "StepMania"; intermediate releases = "StepMania CVS".
!define PRODUCT_ID "StepMania CVS"
!define PRODUCT_VER "4.0"
; !define PRODUCT_VER "4.0 alpha 1"
!define PRODUCT_ID_VER "${PRODUCT_ID} ${PRODUCT_VER}"


!define PRODUCT_URL "http://www.stepmania.com"
!define UPDATES_URL "http://www.stepmania.com/wiki/Downloads"

;!define INSTALL_TYPE_EXTERNAL_PCKS
;!define INSTALL_TYPE_INTERNAL_PCKS
!define INSTALL_TYPE_NON_PCK_DATA
!define ASSOCIATE_SMZIP
;!define SHOW_AUTORUN
!define MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
;!define DIRECTX_81_REDIST_PRESENT
;!define MAKE_UPDATES_SHORTCUT
