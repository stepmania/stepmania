; NSIS Install Script
; created by 
;     BBF 
; I use the following command to create the installer:
; D:\Program Files\NSIS>makensis.exe /v3 /cd "m:\Dev Projects\CVS\stepmania\stepmania.nsi"
;
; NOTE: this .NSI script is designed for NSIS v1.8+

; Don't change this.
!define PRODUCT_NAME "StepMania"

!define VERSION "CVS"
; !define VERSION "3.01"

; If this is changed, different versions of SM can be installed
; in parallel.  Normal releases should be StepMania; CVS releases
; should be StepMania CVS.
!define PRODUCT_ID "StepMania CVS"
!define PRODUCT_NAME_VER "${PRODUCT_NAME} ${VERSION}"

Name "${PRODUCT_NAME}"
OutFile "StepMania-CVS-20030315.exe"
;OutFile "StepMania301.exe"

; Some default compiler settings (uncomment and change at will):
SetCompress auto ; (can be off or force)
SetDatablockOptimize on ; (can be off)
CRCCheck on ; (can be off)
AutoCloseWindow true ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
SetDateSave on ; (can be on to have files restored to their orginal date)
InstallDir "$PROGRAMFILES\${PRODUCT_ID}"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" ""
DirShow show ; (make this hide to not let the user change it)
DirText "${PRODUCT_NAME_VER}"


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

; Check for DirectX 8.0 (to be moved to the right section later)
; We only use this for sound.  Actually, I could probably make the sound
; work with an earlier one; I'm not sure if that's needed or not.  For one
; thing, forcing people to upgrade drivers is somewhat of a good thing;
; but upgrading to DX8 if you really don't have to is also somewhat
; annoying, too ... -g
ReadRegStr $0 HKEY_LOCAL_MACHINE "Software\Microsoft\DirectX" "Version"
StrCpy $1 $0 2 2 ;  8.1 is "4.08.01.0810"
IntCmpU $1 8 check_subversion old_dx ok
check_subversion:
StrCpy $1 $0 2 5
IntCmpU $1 0 ok old_dx ok

; We can function without it (using WaveOut), so don't *require* this.
old_dx:
MessageBox MB_YESNO|MB_ICONINFORMATION "The latest version of DirectX (8.1 or higher) is strongly recommended.$\n Do you wish to visit Microsoft's site now ?" IDYES open_dx_page
Goto ok

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
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "${PRODUCT_ID} (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninst.exe"'

WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command" "" '"$INSTDIR\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "SMZIP package"
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
RMDir /r "$INSTDIR\NoteSkins\dance\default"
RMDir /r "$INSTDIR\NoteSkins\dance\flat"
RMDir /r "$INSTDIR\NoteSkins\dance\note"
RMDir /r "$INSTDIR\NoteSkins\dance\solo"
SetOutPath "$INSTDIR\NoteSkins"
File "NoteSkins\instructions.txt"
; why do this? -glenn
CreateDirectory "$INSTDIR\NoteSkins\dance\default"
CreateDirectory "$INSTDIR\NoteSkins\dance\flat"
CreateDirectory "$INSTDIR\NoteSkins\dance\note"
CreateDirectory "$INSTDIR\NoteSkins\dance\solo"
SetOutPath "$INSTDIR\NoteSkins\dance"
File /r "NoteSkins\dance\default"
File /r "NoteSkins\dance\flat"
File /r "NoteSkins\dance\note"
File /r "NoteSkins\dance\solo"

SetOutPath "$INSTDIR\NoteSkins\pump"
File /r "NoteSkins\pump\Classic" ; what the heck, they're tiny
File /r "NoteSkins\pump\default"

; temporarily disabled--noteskin needs updating
;SetOutPath "$INSTDIR\NoteSkins\ez2"
;File /r "NoteSkins\ez2\original"

;SetOutPath "$INSTDIR\NoteSkins\para"
;File /r "NoteSkins\para\original"

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
File "msvcr70.dll"
File "msvcp70.dll"
File "jpeg.dll"
File "libpng13a.dll"
File "ogg.dll"
File "vorbis.dll"
File "vorbisfile.dll"
File "zliba.dll"
File "SDL.dll"
File "SDL_image.dll"
; File "SDL_net.dll"

File "COPYING.txt"
File "README-FIRST.html"
File "NEWS"
File "stepmania.exe"
File "stepmania.vdi"
File "smpackage.exe"
File "Translation.dat"

; What to do here?  Better to just delete an existing INI than to
; drop the local one in ... -glenn
; Agreed. - Chris
; But we shouldn't delete, either, since that'll wipe peoples' prefs.
; Better to address upgrade problems than to make people nuke INI's.
; Maybe we should remove this for snapshot releases; that way we can
; track down problems with INI upgrades (and then possibly restore it
; for the release if we're not confident we've fixed most problems) -glenn
; Added message box to ask user. -Chris

IfFileExists "$INSTDIR\stepmania.ini" stepmania_ini_present stepmania_ini_not_present
stepmania_ini_present:
MessageBox MB_YESNO|MB_ICONQUESTION "Your settings from the previous installation of StepMania were found.$\nWould you like to keep these settings?" IDNO stepmania_ini_not_present
Delete "$INSTDIR\stepmania.ini"
stepmania_ini_not_present:

; Create Start Menu icons
SetShellVarContext all	; install in "All Users" if NT
CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
CreateShortCut "$DESKTOP\Play ${PRODUCT_NAME_VER}.lnk" "$INSTDIR\stepmania.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME_VER}.lnk" "$INSTDIR\stepmania.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Open Songs Folder.lnk" "$WINDIR\explorer.exe" "$INSTDIR\Songs\"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\SMZIP Package Exporter.lnk" "$INSTDIR\smpackage.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\README-FIRST.lnk" "$INSTDIR\README-FIRST.html"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Uninstall ${PRODUCT_NAME_VER}.lnk" "$INSTDIR\uninst.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Go To StepMania web site.lnk" "http://www.stepmania.com"

Exec '$WINDIR\explorer.exe "$SMPROGRAMS\${PRODUCT_ID}\"'

SectionEnd ; end of default section


;
; begin uninstall settings/section
;
UninstallText "This will uninstall StepMania from your system.$\nAny add-on packs or songs you have installed will not be deleted."

Section Uninstall

; add delete commands to delete whatever files/registry keys/etc you installed here.
Delete "$INSTDIR\uninst.exe"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}"
DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}"

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

RMDir /r "$INSTDIR\Cache"

Delete "$INSTDIR\Courses\instructions.txt"
RMDir "$INSTDIR\Courses"

Delete "$INSTDIR\Music\instructions.txt"
RMDir "$INSTDIR\Music"

Delete "$INSTDIR\NoteSkins\instructions.txt"
RMDir /r "$INSTDIR\NoteSkins\dance\default"
RMDir /r "$INSTDIR\NoteSkins\dance\flat"
RMDir /r "$INSTDIR\NoteSkins\dance\note"
RMDir /r "$INSTDIR\NoteSkins\dance\solo"
RMDir "$INSTDIR\NoteSkins\dance"
RMDir /r "$INSTDIR\NoteSkins\pump\classic"
RMDir /r "$INSTDIR\NoteSkins\pump\default"
RMDir "$INSTDIR\NoteSkins\pump"
RMDir /r "$INSTDIR\NoteSkins\ez2\original"
RMDir "$INSTDIR\NoteSkins\ez2"
RMDir /r "$INSTDIR\NoteSkins\para\original"
RMDir "$INSTDIR\NoteSkins\para"
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

Delete "$INSTDIR\msvcr70.dll"
Delete "$INSTDIR\msvcp70.dll"
Delete "$INSTDIR\jpeg.dll"
Delete "$INSTDIR\libpng13a.dll"
Delete "$INSTDIR\ogg.dll"
Delete "$INSTDIR\vorbis.dll"
Delete "$INSTDIR\vorbisfile.dll"
Delete "$INSTDIR\zliba.dll"
Delete "$INSTDIR\SDL.dll"
Delete "$INSTDIR\SDL_image.dll"
Delete "$INSTDIR\COPYING.txt"
Delete "$INSTDIR\README-FIRST.html"
Delete "$INSTDIR\NEWS"
Delete "$INSTDIR\stepmania.exe"
Delete "$INSTDIR\stepmania.ini"
Delete "$INSTDIR\Keymaps.ini"
Delete "$INSTDIR\GamePrefs.ini"
Delete "$INSTDIR\smpackage.exe"
Delete "$INSTDIR\Translation.dat"
Delete "$INSTDIR\StepMania.vdi"
Delete "$INSTDIR\log.txt"
Delete "$INSTDIR\info.txt"

RMDir "$INSTDIR"	; will delete only if empty

SetShellVarContext all	; delete from "All Users" if NT
Delete "$DESKTOP\Play {PRODUCT_NAME_VER}.lnk"
; I'm being paranoid here:
Delete "$SMPROGRAMS\${PRODUCT_ID}\*.*"
RMDir "$SMPROGRAMS\${PRODUCT_ID}"

SectionEnd ; end of uninstall section

; eof
