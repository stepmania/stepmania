; NSIS Install Script
; created by BBF
; I use the following command to create the installer:
; D:\Program Files\NSIS>makensis.exe /v3 /cd "m:\Dev Projects\CVS\stepmania\stepmania.nsi"
;
; NOTE: this .NSI script is designed for NSIS v1.8+

Name "StepMania"
OutFile "stepmania-install.exe"

; Some default compiler settings (uncomment and change at will):
SetCompress auto ; (can be off or force)
SetDatablockOptimize on ; (can be off)
CRCCheck on ; (can be off)
; AutoCloseWindow false ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
; SetDateSave off ; (can be on to have files restored to their orginal date)

InstallDir "$PROGRAMFILES\StepMania"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania" ""
DirShow show ; (make this hide to not let the user change it)
DirText "Select the directory to install StepMania in:"


; Since there's only 1 page, let's check for DX when the user selects NEXT

Function .onInit

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


Section "" ; (default section)
SetOutPath "$INSTDIR"
; add files / whatever that need to be installed here.
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\StepMania" "DisplayName" "StepMania (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\StepMania" "UninstallString" '"$INSTDIR\uninst.exe"'
SetOverwrite on
; write out uninstaller
WriteUninstaller "$INSTDIR\uninst.exe"

; Begin SM Install
SetOverwrite ifnewer

; Announcer:
CreateDirectory "$INSTDIR\Announcers"
SetOutPath "$INSTDIR\Announcers"
File /r "Announcers\default"

CreateDirectory "$INSTDIR\BGAnimations"
SetOutPath "$INSTDIR"
File /r "BGAnimations"

CreateDirectory "$INSTDIR\Cache"
SetOutPath "$INSTDIR\Cache"
File "Cache\README.TXT"

CreateDirectory "$INSTDIR\Courses"
SetOutPath "$INSTDIR\Courses"
File "Courses\README.TXT"

CreateDirectory "$INSTDIR\Skins"
SetOutPath "$INSTDIR\Skins"
File /r "Skins\dance"
File /r "Skins\ez2"
File /r "Skins\pump"

CreateDirectory "$INSTDIR\Songs"
SetOutPath "$INSTDIR\Songs"
File "Songs\Instructions.txt"

CreateDirectory "$INSTDIR\Themes"
SetOutPath "$INSTDIR\Themes"
File /r "Themes\default"
File /r "Themes\dance"
File /r "Themes\ez2"
File /r "Themes\pump"

CreateDirectory "$INSTDIR\Visualizations"
SetOutPath "$INSTDIR\Visualizations"
File "Visualizations\Visualization movie files go here"

SetOutPath "$INSTDIR"
File "bass.dll"
File "COPYING.txt"
File "README-FIRST.TXT"
File "StepMania.exe"

SectionEnd ; end of default section


; begin uninstall settings/section
UninstallText "This will uninstall StepMania from your system.$\n BE SURE TO BACKUP YOUR SONGS BEFORE DOING THIS."

Section Uninstall
; add delete commands to delete whatever files/registry keys/etc you installed here.
Delete "$INSTDIR\uninst.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\StepMania"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\StepMania"

; Nicely try to clean out everything...

RMDir /r "$INSTDIR\Announcers\default"
RMDir "$INSTDIR\Announcers" ; Only remove it if it's empty now

RMDir /r "$INSTDIR\BGAnimations"

Delete "$INSTDIR\Cache\README.TXT"
RMDir "$INSTDIR\Cache"

Delete "$INSTDIR\Courses\README.TXT"
RMDir "$INSTDIR\Courses"

RMDir /r "$INSTDIR\Skins\dance"
RMDir /r "$INSTDIR\Skins\ez2"
RMDir /r "$INSTDIR\Skins\pump"
RMDir "$INSTDIR\Skins"

Delete "$INSTDIR\Songs\Instructions.txt"
RMDir "$INSTDIR\Songs" ; It'll only remove it if it's empty ( I hope )

RMDir /r "$INSTDIR\Themes\default"
RMDir /r "$INSTDIR\Themes\dance"
RMDir /r "$INSTDIR\Themes\ez2"
RMDir /r "$INSTDIR\Themes\pump"
RMDir "$INSTDIR\Themes"

Delete "$INSTDIR\Visualizations\Visualization movie files go here"
RMDir "$INSTDIR\Visualizations"

Delete "$INSTDIR\bass.dll"
Delete "$INSTDIR\COPYING.txt"
Delete "$INSTDIR\README-FIRST.TXT"
Delete "$INSTDIR\StepMania.exe"

RMDir "$INSTDIR"
SectionEnd ; end of uninstall section

; eof
