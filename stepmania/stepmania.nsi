; NSIS Install Script
; created by 
;     BBF 
; I use the following command to create the installer:
; D:\Program Files\NSIS>makensis.exe /v3 /cd "m:\Dev Projects\CVS\stepmania\stepmania.nsi"
;
; NOTE: this .NSI script is designed for NSIS v2.0+


; Product info is in a separate file so that people will change
; the version info for the installer and the program at the same time.
; It's confusing when the installer and shortcut text doesn't match the 
; title screen version text.
!include "src\ProductInfo.inc"

!system "echo This may take a moment ..." ignore
!system "utils\upx Program\*.exe Program\*.dll" ignore

Name "${PRODUCT_NAME_VER}"
OutFile "${PRODUCT_NAME_VER}.exe"

; Don't say "StepMania 3.9 Setup"; just say "StepMania 3.9".
Caption "${PRODUCT_NAME_VER}"
UninstallCaption "${PRODUCT_NAME_VER}"

; Some default compiler settings (uncomment and change at will):
SetCompress auto ; (can be off or force)
SetDatablockOptimize on ; (can be off)
CRCCheck on ; (can be off)
AutoCloseWindow true ; (can be true for the window go away automatically at end)
; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
SetDateSave on ; (can be on to have files restored to their orginal date)
InstallDir "$PROGRAMFILES\${PRODUCT_ID}"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" ""
; DirShow show ; (make this hide to not let the user change it)
DirText "${PRODUCT_NAME_VER}"
InstallColors /windows
InstProgressFlags smooth

PageEx directory
	Caption " "
PageExEnd
Page instfiles


;
; .oninit is called before window is shown
;
Function .onInit


; force uninstall of previous version using NSIS
; We need to wait until the uninstaller finishes before continuing, since it's possible
; to click the next button again before the uninstaller's window appears and takes focus.
; This is tricky: we can't just ExecWait the uninstaller, since it copies off the uninstaller
; EXE and exec's that (otherwise it couldn't delete itself), so it appears to exit immediately.
; We need to copy it off ourself, run it with the hidden parameter to tell it it's a copy,
; and then delete the copy ourself.  There's one more trick: the hidden parameter changed
; between NSIS 1 and 2: in 1.x it was _=C:\Foo, in 2.x it's _?=C:\Foo.  Rename the installer
; for newer versions, so we can tell the difference: "uninst.exe" is the old 1.x uninstaller,
; "uninstall.exe" is 2.x.
StrCpy $R1 "$INSTDIR\uninst.exe"
StrCpy $R2 "_="
IfFileExists "$R1" prompt_uninstall_nsis
StrCpy $R1 "$INSTDIR\uninstall.exe"
StrCpy $R2 "_?="
IfFileExists "$R1" prompt_uninstall_nsis old_nsis_not_installed

prompt_uninstall_nsis:
MessageBox MB_YESNO|MB_ICONINFORMATION "The previous version of StepMania must be uninstalled before continuing.$\nDo you wish to continue?" IDYES do_uninstall_nsis
Abort

do_uninstall_nsis:
GetTempFileName $R3
CopyFiles /SILENT $R1 $R3
ExecWait '$R3 $R2$INSTDIR' $R4
; Delete the copy of the installer.
Delete $R3

; $R4 is the exit value of the uninstaller.  0 means success, anything else is
; failure (eg. aborted).
IntCmp $R4 0 old_nsis_not_installed ; jump if 0

MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONINFORMATION "Uninstallation failed.  Install anyway?" IDYES old_nsis_not_installed
Abort


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
MessageBox MB_YESNO|MB_ICONINFORMATION "The latest version of DirectX (8.1 or higher) is strongly recommended.$\n Do you wish to visit Microsoft's site now?" IDNO ok
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
AllowSkipFiles off
SetOverwrite on
WriteUninstaller "$INSTDIR\uninstall.exe"

; add registry entries
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\StepMania\${PRODUCT_ID}" "" "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "${PRODUCT_ID} (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninstall.exe"'

WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command" "" '"$INSTDIR\Program\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "SMZIP package"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\Program\smpackage.exe,0"
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\Program\smpackage.exe" "%1"'
WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip" "" "smzipfile"

; Begin copying files
CreateDirectory "$INSTDIR\Announcers"
SetOutPath "$INSTDIR\Announcers"
File "Announcers\instructions.txt"

CreateDirectory "$INSTDIR\BGAnimations"
SetOutPath "$INSTDIR\BGAnimations"
File "BGAnimations\instructions.txt"

CreateDirectory "$INSTDIR\CDTitles"
SetOutPath "$INSTDIR\CDTitles"
File "CDTitles\Instructions.txt"

RMDir /r "$INSTDIR\Characters\default"
CreateDirectory "$INSTDIR\Characters\default"
SetOutPath "$INSTDIR\Characters"
File /r "Characters\default"
# File "Characters\instructions.txt"

CreateDirectory "$INSTDIR\Courses"
SetOutPath "$INSTDIR\Courses"
File "Courses\instructions.txt"
File /r "Courses\Samples"

CreateDirectory "$INSTDIR\Packages"
File "Packages\Instructions.txt"

RMDir /r "$INSTDIR\NoteSkins\common\default"
RMDir /r "$INSTDIR\NoteSkins\dance\MAX"
RMDir /r "$INSTDIR\NoteSkins\dance\default"
RMDir /r "$INSTDIR\NoteSkins\dance\flat"
RMDir /r "$INSTDIR\NoteSkins\dance\note"
RMDir /r "$INSTDIR\NoteSkins\dance\solo"
SetOutPath "$INSTDIR\NoteSkins"
File "NoteSkins\instructions.txt"

SetOutPath "$INSTDIR\NoteSkins\common"
File /r "NoteSkins\common\default"

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

SetOutPath "$INSTDIR"

CreateDirectory "$INSTDIR\BackgroundEffects"
File /r "BackgroundEffects"

CreateDirectory "$INSTDIR\BackgroundTransitions"
File /r "BackgroundTransitions"

CreateDirectory "$INSTDIR\RandomMovies"
SetOutPath "$INSTDIR\RandomMovies"
File "RandomMovies\instructions.txt"

CreateDirectory "$INSTDIR\Songs"
SetOutPath "$INSTDIR\Songs"
File "Songs\Instructions.txt"

RMDir /r "$INSTDIR\Themes\default"
CreateDirectory "$INSTDIR\Themes"
SetOutPath "$INSTDIR\Themes"
File "Themes\instructions.txt"
File /r "Themes\default"

CreateDirectory "$INSTDIR\Docs"
SetOutPath "$INSTDIR\Docs"
File "Docs\ChangeLog.txt"

CreateDirectory "$INSTDIR\Data"
SetOutPath "$INSTDIR\Data"
File "Data\Translations.xml"
File "Data\AI.ini"
File "Data\splash.png"

SetOutPath "$INSTDIR\Program"
File "Program\stepmania.exe"
File "Program\stepmania.vdi"
File "Program\smpackage.exe"
File "Program\msvcr70.dll"
File "Program\msvcp70.dll"
File "Program\msvcp71.dll"
File "Program\jpeg.dll"
File "Program\avcodec.dll"
File "Program\avformat.dll"
File "Program\resample.dll"
File "Program\dbghelp.dll"
File "Program\zlib1.dll"

SetOverwrite off
SetOutPath "$INSTDIR\Data\MachineProfile"
File "Docs\Stats.xml"
SetOverwrite on

SetOutPath "$INSTDIR"
File "Docs\Copying.txt"
File "README-FIRST.html"
File "NEWS"

; What to do here?  Better to just delete an existing INI than to
; drop the local one in ... -glenn
; Agreed. - Chris
; But we shouldn't delete, either, since that'll wipe peoples' prefs.
; Better to address upgrade problems than to make people nuke INI's.
; Maybe we should remove this for snapshot releases; that way we can
; track down problems with INI upgrades (and then possibly restore it
; for the release if we're not confident we've fixed most problems) -glenn
; Added message box to ask user. -Chris
; This is a silly check, because we force an uninstall and the installer 
; removes stepmania.ini.  -Chris

;IfFileExists "$INSTDIR\stepmania.ini" stepmania_ini_present 
;IfFileExists "$INSTDIR\Data\stepmania.ini" stepmania_ini_present 
;Goto stepmania_ini_not_present
;stepmania_ini_present:
;MessageBox MB_YESNO|MB_ICONQUESTION "Your settings from the previous installation of StepMania were found.$\nWould you like to keep these settings?" IDNO stepmania_ini_not_present
;Delete "$INSTDIR\Data\stepmania.ini"
;stepmania_ini_not_present:
;; Move ini into Data\
;Rename "$INSTDIR\Data\stepmania.ini" "$INSTDIR\stepmania.ini"

; Create Start Menu icons
SetShellVarContext current  # 	'all' doesn't work on Win9x
CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
CreateShortCut "$DESKTOP\Play ${PRODUCT_NAME_VER}.lnk" "$INSTDIR\Program\stepmania.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME_VER}.lnk" "$INSTDIR\Program\stepmania.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Open StepMania Program Folder.lnk" "$WINDIR\explorer.exe" "$INSTDIR\"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\View Machine Statistics.lnk" "$INSTDIR\Data\MachineProfile\Stats.xml"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\StepMania Tools and Package Exporter.lnk" "$INSTDIR\Program\smpackage.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\README-FIRST.lnk" "$INSTDIR\README-FIRST.html"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Uninstall ${PRODUCT_NAME_VER}.lnk" "$INSTDIR\uninstall.exe"
CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Go To StepMania web site.lnk" "http://www.stepmania.com"

CreateShortCut "$INSTDIR\StepMania.lnk" "$INSTDIR\Program\stepmania.exe"

# We want to delete a few old desktop icons, since they weren't being
# uninstalled correctly during alpha 2 and 3.  They were installed in
# the 'all' context.  Try to delete them in both contexts.
SetShellVarContext all
Delete "$DESKTOP\Play StepMania 3.9 alpha 2.lnk"
Delete "$DESKTOP\Play StepMania 3.9 alpha 3.lnk"

SetShellVarContext current
Delete "$DESKTOP\Play StepMania 3.9 alpha 2.lnk"
Delete "$DESKTOP\Play StepMania 3.9 alpha 3.lnk"

Exec '$WINDIR\explorer.exe "$SMPROGRAMS\${PRODUCT_ID}\"'

SectionEnd ; end of default section


;
; begin uninstall settings/section
;
UninstallText "This will uninstall StepMania from your system.$\nAny add-on packs or songs you have installed will not be deleted."

Section Uninstall

; add delete commands to delete whatever files/registry keys/etc you installed here.
Delete "$INSTDIR\uninstall.exe"
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

Delete "$INSTDIR\CDTitles\Instructions.txt"
RMDir "$INSTDIR\CDTitles"

RMDir /r "$INSTDIR\Characters\default"
RMDir "$INSTDIR\Characters"

RMDir /r "$INSTDIR\Cache"

Delete "$INSTDIR\Packages\instructions.txt"
RMDir "$INSTDIR\Packages"

Delete "$INSTDIR\Courses\instructions.txt"
RMDir /r "$INSTDIR\Courses\Samples"
RMDir "$INSTDIR\Courses"

Delete "$INSTDIR\NoteSkins\instructions.txt"
RMDir /r "$INSTDIR\NoteSkins\common\default"
RMDir "$INSTDIR\NoteSkins\common"
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

Delete "$INSTDIR\Songs\Instructions.txt"
RMDir "$INSTDIR\Songs"	; will delete only if empty

Delete "$INSTDIR\Themes\instructions.txt"
RMDir /r "$INSTDIR\Themes\default"
RMDir "$INSTDIR\Themes"

Delete "$INSTDIR\Visualizations\instructions.txt"
RMDir "$INSTDIR\Visualizations"

Delete "$INSTDIR\Docs\ChangeLog.txt"
RMDir "$INSTDIR\Docs"

; Don't delete high scores.
Delete "$INSTDIR\Data\Translation.dat"
Delete "$INSTDIR\Data\AI.ini"
;Delete "$INSTDIR\Data\stepmania.ini"
;Delete "$INSTDIR\Data\Keymaps.ini"
;Delete "$INSTDIR\Data\GamePrefs.ini"
Delete "$INSTDIR\Data\splash.png"
RMDir "$INSTDIR\Data"

Delete "$INSTDIR\Program\stepmania.exe"
Delete "$INSTDIR\Program\smpackage.exe"
Delete "$INSTDIR\Program\StepMania.vdi"
Delete "$INSTDIR\Program\msvcr70.dll"
Delete "$INSTDIR\Program\msvcp70.dll"
Delete "$INSTDIR\Program\jpeg.dll"
Delete "$INSTDIR\Program\avcodec.dll"
Delete "$INSTDIR\Program\avformat.dll"
Delete "$INSTDIR\Program\resample.dll"
Delete "$INSTDIR\Program\dbghelp.dll"
Delete "$INSTDIR\Program\zlib1.dll"
Delete "$INSTDIR\Program\lua.dll"
RMDir "$INSTDIR\Program"

; It's harmless to delete this, as long as we leave Stats.xml alone; it'll be rewritten
; by the program the next time it's run.
Delete "$INSTDIR\Data\MachineProfile\stats.html"
RMDir "$INSTDIR\Data\MachineProfile"

Delete "$INSTDIR\COPYING.txt"
Delete "$INSTDIR\README-FIRST.html"
Delete "$INSTDIR\NEWS"
Delete "$INSTDIR\log.txt"
Delete "$INSTDIR\info.txt"
Delete "$INSTDIR\crashinfo.txt"
Delete "$INSTDIR\StepMania.lnk"

RMDir "$INSTDIR"	; will delete only if empty

SetShellVarContext current
Delete "$DESKTOP\Play StepMania CVS.lnk"
Delete "$DESKTOP\Play ${PRODUCT_NAME_VER}.lnk"
; I'm being paranoid here:
Delete "$SMPROGRAMS\${PRODUCT_ID}\*.*"
RMDir "$SMPROGRAMS\${PRODUCT_ID}"

SectionEnd ; end of uninstall section

; eof
