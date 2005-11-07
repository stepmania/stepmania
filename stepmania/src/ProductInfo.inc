; Included by the NSIS installer script
; Don't forget to also change ProductInfo.h!

!define PRODUCT_NAME "StepMania"
!define PRODUCT_VER "4.0 CVS"
; !define PRODUCT_VER "4.0 alpha 1"
!define PRODUCT_NAME_VER "${PRODUCT_NAME} ${PRODUCT_VER}"

; String used for the install directory and registry locations
; Official releases = "StepMania"; intermediate releases = "StepMania CVS".
!define PRODUCT_ID "StepMania CVS"

!define PRODUCT_URL "http://www.stepmania.com"

;!define INSTALL_TYPE_EXTERNAL_PCKS
;!define INSTALL_TYPE_INTERNAL_PCKS
!define INSTALL_TYPE_NON_PCK_DATA
!define INSTALL_SMPACKAGE
;!define SHOW_AUTORUN
!define MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
;!define DIRECTX_81_REDIST_PRESENT
