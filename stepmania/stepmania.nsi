; NSIS Install Script
; created by 
;     BBF 
; I use the following command to create the installer:
; D:\Program Files\NSIS>makensis.exe /v3 /cd "m:\Dev Projects\CVS\stepmania\stepmania.nsi"
;
; NOTE: this .NSI script is designed for NSIS v1.8+

Name "StepMania"
OutFile "stepmania300final.exe"
!define PRODUCT_NAME "StepMania 3.0 final"


; Some default compiler settings (uncomment and change at will):
SetCompress auto ; (can be off or force)
SetDatablockOptimize on ; (can be off)
CRCCheck on ; (can be off)
AutoCloseWindow true ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
; SetDateSave off ; (can be on to have files restored to their orginal date)
InstallDir "$PROGRAMFILES\StepMania"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania" ""
DirShow show ; (make this hide to not let the user change it)
DirText "Select the directory to install StepMania in:"


;
; .oninit is called before window is shown
;
Function .onInit

; force uninstall of previous version using Vise
IfFileExists "$INSTDIR\uninstal.log" prompt_uninstall_vise old_vise_not_installed
prompt_uninstall_vise:
MessageBox MB_YESNO|MB_ICONINFORMATION "The previous version of StepMania must be uninstalled before continuing.$\nDo you wish to continue?" IDYES do_uninstall_vise
Abort
do_uninstall_vise:
Exec '$WINDIR\unvise32.exe $INSTDIR\uninstal.log'	; don't use quotes here, or unvise32 will fail
old_vise_not_installed:

; force uninstall of previous version using NSIS
IfFileExists "$INSTDIR\uninst.exe" prompt_uninstall_nsis old_nsis_not_installed
prompt_uninstall_nsis:
MessageBox MB_YESNO|MB_ICONINFORMATION "The previous version of StepMania must be uninstalled before continuing.$\nDo you wish to continue?" IDYES do_uninstall_nsis
Abort
do_uninstall_nsis:
Exec "$INSTDIR\uninst.exe"
old_nsis_not_installed:

; Check for DirextX 8.1 (to be moved to the right section later)
ReadRegStr $0 HKEY_LOCAL_MACHINE "Software\Microsoft\DirectX" "Version"
StrCpy $1 $0 2 2 ;  8.1 is "4.08.01.0810"
IntCmpU $1 8 check_subversion old_dx ok
check_subversion:
StrCpy $1 $0 2 5
IntCmpU $1 1 ok old_dx ok

old_dx:
MessageBox MB_YESNO|MB_ICONINFORMATION "You need to upgrade to the latest DirectX to use this software (at least 8.1).$\n Do you wish to visit Microsoft's site now ?" IDYES open_dx_page
Abort

open_dx_page:
ExecShell "" "http://www.microsoft.com/directx/"
Abort

ok:

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
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\StepMania" "DisplayName" "StepMania (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\StepMania" "UninstallString" '"$INSTDIR\uninst.exe"'

WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command" "" '"$INSTDIR\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "StepMania package"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\smpackage.exe,0"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip" "" "smzipfile"

; Begin copying files
SetOverwrite ifnewer

CreateDirectory "$INSTDIR\Announcers"
SetOutPath "$INSTDIR\Announcers"
File "Announcers\instructions.txt"

CreateDirectory "$INSTDIR\BGAnimations"
SetOutPath "$INSTDIR\BGAnimations"
File "BGAnimations\instructions.txt"

CreateDirectory "$INSTDIR\Cache"
SetOutPath "$INSTDIR\Cache"
File "Cache\instructions.txt"

CreateDirectory "$INSTDIR\CDTitles"
SetOutPath "$INSTDIR\CDTitles"
File "CDTitles\instructions.txt"

CreateDirectory "$INSTDIR\Courses"
SetOutPath "$INSTDIR\Courses"
File "Courses\instructions.txt"

CreateDirectory "$INSTDIR\Music"
SetOutPath "$INSTDIR\Music"
File "Music\instructions.txt"

RMDir /r "$INSTDIR\NoteSkins\dance\MAX"
CreateDirectory "$INSTDIR\NoteSkins\dance\MAX"
SetOutPath "$INSTDIR\NoteSkins"
File "NoteSkins\instructions.txt"
SetOutPath "$INSTDIR\NoteSkins\dance"
File /r "NoteSkins\dance\MAX"

CreateDirectory "$INSTDIR\RandomMovies"
SetOutPath "$INSTDIR\RandomMovies"
File "RandomMovies\instructions.txt"

CreateDirectory "$INSTDIR\Songs"
SetOutPath "$INSTDIR\Songs"
File "Songs\instructions.txt"

RMDir /r "$INSTDIR\Themes\default"
CreateDirectory "$INSTDIR\Themes"
SetOutPath "$INSTDIR\Themes"
File "Themes\instructions.txt"
File /r "Themes\default"

CreateDirectory "$INSTDIR\Visualizations"
SetOutPath "$INSTDIR\Visualizations"
File "Visualizations\instructions.txt"

SetOutPath "$INSTDIR"
File "bass.dll"
File "COPYING.txt"
File "README-FIRST.TXT"
File "NEWS"
File "stepmania.exe"
; What to do here?  Better to just delete an existing INI than to
; drop the local one in ... -glenn
; Agreed. - Chris
Delete "$INSTDIR\stepmania.ini"
File "stepmania.vdi"
File "smpackage.exe"

; Create Start Menu icons
SetShellVarContext all	; install in "All Users" if NT
CreateDirectory "$SMPROGRAMS\StepMania\"
CreateShortCut "$DESKTOP\Play ${PRODUCT_NAME}.lnk" "$INSTDIR\stepmania.exe"
CreateShortCut "$SMPROGRAMS\StepMania\${PRODUCT_NAME}.lnk" "$INSTDIR\stepmania.exe"
CreateShortCut "$SMPROGRAMS\StepMania\Open Songs Folder.lnk" "$WINDIR\explorer.exe" "$INSTDIR\Songs\"
CreateShortCut "$SMPROGRAMS\StepMania\Package Exporter.lnk" "$INSTDIR\smpackage.exe"
CreateShortCut "$SMPROGRAMS\StepMania\README.lnk" "$INSTDIR\README-FIRST.txt"
CreateShortCut "$SMPROGRAMS\StepMania\Uninstall ${PRODUCT_NAME}.lnk" "$INSTDIR\uninst.exe"
CreateShortCut "$SMPROGRAMS\StepMania\Go To StepMania web site.lnk" "http://www.stepmania.com"

Exec '$WINDIR\explorer.exe "$SMPROGRAMS\StepMania\"'

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

Delete "$INSTDIR\Announcers\instructions.txt"
RMDir "$INSTDIR\Announcers"

Delete "$INSTDIR\BGAnimations\instructions.txt"
RMDir "$INSTDIR\BGAnimations"

Delete "$INSTDIR\Cache\instructions.txt"
RMDir "$INSTDIR\Cache"

Delete "$INSTDIR\CDTitles\instructions.txt"
RMDir "$INSTDIR\CDTitles"

Delete "$INSTDIR\Cache\instructions.txt"
RMDir "$INSTDIR\Cache"

Delete "$INSTDIR\Courses\instructions.txt"
RMDir "$INSTDIR\Courses"

Delete "$INSTDIR\Music\instructions.txt"
RMDir "$INSTDIR\Music"

Delete "$INSTDIR\NoteSkins\instructions.txt"
RMDir /r "$INSTDIR\NoteSkins\dance"
RMDir "$INSTDIR\NoteSkins"

Delete "$INSTDIR\RandomMovies\instructions.txt"
RMDir "$INSTDIR\RandomMovies"

Delete "$INSTDIR\Songs\instructions.txt"
RMDir "$INSTDIR\Songs"	; will delete only if empty

Delete "$INSTDIR\Themes\instructions.txt"
RMDir /r "$INSTDIR\Themes\default"
RMDir "$INSTDIR\Themes"

Delete "$INSTDIR\Visualizations\instructions.txt"
RMDir "$INSTDIR\Visualizations"

Delete "$INSTDIR\bass.dll"
Delete "$INSTDIR\COPYING.txt"
Delete "$INSTDIR\README-FIRST.TXT"
Delete "$INSTDIR\NEWS"
Delete "$INSTDIR\stepmania.exe"
Delete "$INSTDIR\stepmania.ini"
Delete "$INSTDIR\smpackage.exe"

RMDir "$INSTDIR"	; will delete only if empty

SetShellVarContext all	; delete from "All Users" if NT
Delete "$DESKTOP\Play {PRODUCT_NAME}.lnk"
Delete "$SMPROGRAMS\StepMania\*.*"
RMDir "$SMPROGRAMS\StepMania"

SectionEnd ; end of uninstall section

; eof
