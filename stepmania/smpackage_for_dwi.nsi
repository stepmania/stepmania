; NSIS Install Script
; created by 
;     Chris
; I use the following command to create the installer:
; D:\Program Files\NSIS>makensis.exe /v3 /cd "m:\Dev Projects\CVS\stepmania\stepmania.nsi"
;
; NOTE: this .NSI script is designed for NSIS v1.8+

; Don't change this.
!define PRODUCT_NAME "SMPackage for DWI"

; If this is changed, different versions of SM can be installed
; in parallel.  Normal releases should be StepMania; CVS releases
; should be StepMania CVS.
!define PRODUCT_ID "SMPackage"

Name "${PRODUCT_NAME}"
OutFile "smpackage_for_dwi.exe"

; Some default compiler settings (uncomment and change at will):
SetCompress auto ; (can be off or force)
SetDatablockOptimize on ; (can be off)
CRCCheck on ; (can be off)
AutoCloseWindow true ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
SetDateSave on ; (can be on to have files restored to their orginal date)
InstallDir "C:\DWI"
;InstallDir "$PROGRAMFILES\${PRODUCT_ID}"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" ""
DirShow show ; (make this hide to not let the user change it)
DirText " " "IMPORTANT!  Select your DWI directory:"


;
; .oninit is called before window is shown
;
Function .onInit

; force uninstall of previous version using NSIS
IfFileExists "$INSTDIR\uninst.exe" prompt_uninstall_nsis old_nsis_not_installed
prompt_uninstall_nsis:
MessageBox MB_YESNO|MB_ICONINFORMATION "The previous version of StepMania must be uninstalled before continuing.$\nDo you wish to continue?" IDYES do_uninstall_nsis
Abort
do_uninstall_nsis:
Exec "$INSTDIR\uninst.exe"
old_nsis_not_installed:

FunctionEnd


;
;  The default install section
;
Section ""

; write out uninstaller
SetOutPath "$INSTDIR"
SetOverwrite on
WriteUninstaller "$INSTDIR\uninst.exe"

; add registry entries
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "${PRODUCT_ID} (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninst.exe"'

WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command" "" '"$INSTDIR\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "StepMania package"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\smpackage.exe,0"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip" "" "smzipfile"

; Begin copying files
SetOverwrite ifnewer

SetOutPath "$INSTDIR"
File "smpackage.exe"

; Create Start Menu icons
SetShellVarContext all	; install in "All Users" if NT
CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME}.lnk" "$INSTDIR\smpackage.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Uninstall ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"

Exec '$WINDIR\explorer.exe "$SMPROGRAMS\${PRODUCT_ID}\"'

SectionEnd ; end of default section


;
; begin uninstall settings/section
;
UninstallText "This will uninstall StepMania from your system.$\nAny add-on packs or songs you have installed will not be deleted."

Section Uninstall

; add delete commands to delete whatever files/registry keys/etc you installed here.
Delete "$INSTDIR\uninst.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\StepMania"

DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip"

Delete "$INSTDIR\smpackage.exe"

RMDir "$INSTDIR"	; will delete only if empty

SetShellVarContext all	; delete from "All Users" if NT
; I'm being paranoid here:
Delete "$SMPROGRAMS\${PRODUCT_ID}\*.*"
RMDir "$SMPROGRAMS\${PRODUCT_ID}"

SectionEnd ; end of uninstall section

; eof
