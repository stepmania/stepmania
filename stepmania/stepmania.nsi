; NSIS Install Script
; created by 
;     BBF, GlennMaynard, ChrisDanford
; I use the following command to create the installer:
; NOTE: this .NSI script is designed for NSIS v2.0+

;--------------------------------
;Includes

	!include "MUI.nsh"	; Modern UI
	
	; Product info is in a separate file so that people will change
	; the version info for the installer and the program at the same time.
	; It's confusing when the installer and shortcut text doesn't match the 
	; title screen version text.
	!include "src\ProductInfo.inc"

;--------------------------------
;General

	!system "echo This may take a moment ..." ignore
	; upx isn't working with VC++2003 executables.  Disable temporarily.  ;!system "utils\upx Program\*.exe Program\*.dll" ignore

	Name "${PRODUCT_NAME_VER}"
	OutFile "${PRODUCT_NAME_VER}.exe"

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
	InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_NAME}\${PRODUCT_ID}" ""
	; DirShow show ; (make this hide to not let the user change it)
	DirText "${PRODUCT_NAME_VER}"
	InstallColors /windows
	InstProgressFlags smooth

;--------------------------------
;Interface Settings

	!define MUI_HEADERIMAGE
	!define MUI_HEADERIMAGE_BITMAP "Installer\header.bmp"
	!define MUI_ABORTWARNING
	!define MUI_ICON "Installer\install.ico"
	!define MUI_UNICON "Installer\uninstall.ico"

;--------------------------------
;Language Selection Dialog Settings

	;Remember the installer language
	!define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
	!define MUI_LANGDLL_REGISTRY_KEY "Software\${PRODUCT_NAME}\${PRODUCT_ID}" 
	!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
;Pages

!ifdef SHOW_AUTORUN
	Page custom ShowCustom LeaveCustom
!endif

	;!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Docs\Modern UI\License.txt"
	;!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English" # first language is the default language
	!insertmacro MUI_LANGUAGE "French"
	!insertmacro MUI_LANGUAGE "German"
	!insertmacro MUI_LANGUAGE "Spanish"
	!insertmacro MUI_LANGUAGE "Italian"
	;!insertmacro MUI_LANGUAGE "SimpChinese"
	;!insertmacro MUI_LANGUAGE "TradChinese"
	;!insertmacro MUI_LANGUAGE "Japanese"
	;!insertmacro MUI_LANGUAGE "Korean"
	;!insertmacro MUI_LANGUAGE "Dutch"
	;!insertmacro MUI_LANGUAGE "Danish"
	;!insertmacro MUI_LANGUAGE "Swedish"
	;!insertmacro MUI_LANGUAGE "Norwegian"
	;!insertmacro MUI_LANGUAGE "Finnish"
	;!insertmacro MUI_LANGUAGE "Greek"
	;!insertmacro MUI_LANGUAGE "Russian"
	;!insertmacro MUI_LANGUAGE "Portuguese"
	;!insertmacro MUI_LANGUAGE "PortugueseBR"
	;!insertmacro MUI_LANGUAGE "Polish"
	;!insertmacro MUI_LANGUAGE "Ukrainian"
	;!insertmacro MUI_LANGUAGE "Czech"
	;!insertmacro MUI_LANGUAGE "Slovak"
	;!insertmacro MUI_LANGUAGE "Croatian"
	;!insertmacro MUI_LANGUAGE "Bulgarian"
	;!insertmacro MUI_LANGUAGE "Hungarian"
	;!insertmacro MUI_LANGUAGE "Thai"
	;!insertmacro MUI_LANGUAGE "Romanian"
	;!insertmacro MUI_LANGUAGE "Latvian"
	;!insertmacro MUI_LANGUAGE "Macedonian"
	;!insertmacro MUI_LANGUAGE "Estonian"
	;!insertmacro MUI_LANGUAGE "Turkish"
	;!insertmacro MUI_LANGUAGE "Lithuanian"
	;!insertmacro MUI_LANGUAGE "Catalan"
	;!insertmacro MUI_LANGUAGE "Slovenian"
	;!insertmacro MUI_LANGUAGE "Serbian"
	;!insertmacro MUI_LANGUAGE "SerbianLatin"
	;!insertmacro MUI_LANGUAGE "Arabic"
	;!insertmacro MUI_LANGUAGE "Farsi"
	;!insertmacro MUI_LANGUAGE "Hebrew"
	;!insertmacro MUI_LANGUAGE "Indonesian"
	;!insertmacro MUI_LANGUAGE "Mongolian"
	;!insertmacro MUI_LANGUAGE "Luxembourgish"
	;!insertmacro MUI_LANGUAGE "Albanian"
	;!insertmacro MUI_LANGUAGE "Breton"
	;!insertmacro MUI_LANGUAGE "Belarusian"
	;!insertmacro MUI_LANGUAGE "Icelandic"
	;!insertmacro MUI_LANGUAGE "Malay"
	;!insertmacro MUI_LANGUAGE "Bosnian"
	;!insertmacro MUI_LANGUAGE "Kurdish"

;--------------------------------
;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)
  
  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
;Installer Sections

Section "Main Section" SecMain

	; write out uninstaller
	SetOutPath "$INSTDIR"
	AllowSkipFiles off
	SetOverwrite on
	WriteUninstaller "$INSTDIR\uninstall.exe"

	; add registry entries
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_NAME}\${PRODUCT_ID}" "" "$INSTDIR"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "${PRODUCT_ID} (remove only)"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninstall.exe"'

!ifdef INSTALL_TYPE_EXTERNAL_PCKS
	; Do this copy before anything else.  It's the most likely to fail.  
	; Possible failure reasons are: scratched CD, user tried to copy the installer but forgot the pcks.
	CreateDirectory $INSTDIR\pcks
	CopyFiles /SILENT "$EXEDIR\${PRODUCT_NAME}.app\Contents\Resources\*.idx" $INSTDIR\pcks 1
	CopyFiles /SILENT "$EXEDIR\${PRODUCT_NAME}.app\Contents\Resources\*.pck" $INSTDIR\pcks 650000	; assume a CD full of data
	IfErrors do_error do_no_error
	do_error:
	MessageBox MB_OK|MB_ICONSTOP "Fatal error copying pck files."
	Quit
	do_no_error:
!endif

!ifdef INSTALL_SMPACKAGE
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command" "" '"$INSTDIR\Program\smpackage.exe" "%1"'
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "SMZIP package"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\Program\smpackage.exe,0"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\Program\smpackage.exe" "%1"'
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip" "" "smzipfile"
!endif

!ifdef INSTALL_TYPE_NON_PCK_DATA
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
	File "Data\*.*"
	
	SetOutPath "$INSTDIR"
	File "Docs\Copying.txt"
	File "README-FIRST.html"
	File "NEWS"
!endif

!ifdef INSTALL_TYPE_INTERNAL_PCKS
	CreateDirectory "$INSTDIR\pcks"
	SetOutPath "$INSTDIR\pcks"
	File "pcks\*.*"
!endif

	SetOverwrite off
	SetOutPath "$INSTDIR\Save\MachineProfile"
	File "Docs\Stats.xml"
	SetOverwrite on

	SetOutPath "$INSTDIR\Program"
	File "Program\${PRODUCT_NAME}.exe"
	File "Program\${PRODUCT_NAME}.vdi"
!ifdef INSTALL_SMPACKAGE
	File "Program\smpackage.exe"
!endif
	File "Program\msvcr71.dll"
	File "Program\msvcp71.dll"
	File "Program\jpeg.dll"
	File "Program\avcodec.dll"
	File "Program\avformat.dll"
	File "Program\resample.dll"
	File "Program\dbghelp.dll"
	File "Program\zlib1.dll"

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

	;IfFileExists "$INSTDIR\${PRODUCT_NAME}.ini" stepmania_ini_present 
	;IfFileExists "$INSTDIR\Data\${PRODUCT_NAME}.ini" stepmania_ini_present 
	;IfFileExists "$INSTDIR\Save\${PRODUCT_NAME}.ini" stepmania_ini_present 
	;Goto stepmania_ini_not_present
	;stepmania_ini_present:
	;MessageBox MB_YESNO|MB_ICONQUESTION "Your settings from the previous installation of ${PRODUCT_ID} were found.$\nWould you like to keep these settings?" IDNO stepmania_ini_not_present
	;Delete "$INSTDIR\Save\${PRODUCT_NAME}.ini"
	;stepmania_ini_not_present:
	;; Move ini into Save\
	;Rename "$INSTDIR\Save\${PRODUCT_NAME}.ini" "$INSTDIR\${PRODUCT_NAME}.ini"

	; Create Start Menu icons
	SetShellVarContext current  # 	'all' doesn't work on Win9x
	CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
	CreateShortCut "$DESKTOP\${PRODUCT_NAME_VER}.lnk" "$INSTDIR\Program\${PRODUCT_NAME}.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME_VER}.lnk" "$INSTDIR\Program\${PRODUCT_NAME}.exe"
!ifdef MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Open ${PRODUCT_NAME} Program Folder.lnk" "$WINDIR\explorer.exe" "$INSTDIR\"
!endif
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\View Statistics.lnk" "$INSTDIR\Save\MachineProfile\Stats.xml"
!ifdef INSTALL_SMPACKAGE
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME} Tools and Package Exporter.lnk" "$INSTDIR\Program\smpackage.exe"
!endif
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\${PRODUCT_NAME} Documentation.lnk" "$INSTDIR\README-FIRST.html"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Uninstall ${PRODUCT_NAME_VER}.lnk" "$INSTDIR\uninstall.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\Go to the ${PRODUCT_NAME} web site.lnk" "${PRODUCT_URL}"

	CreateShortCut "$INSTDIR\${PRODUCT_NAME}.lnk" "$INSTDIR\Program\${PRODUCT_NAME}.exe"

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

SectionEnd

;--------------------------------
;Installer Functions

LangString TEXT_IO_TITLE		${LANG_ENGLISH} "${PRODUCT_NAME_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_ENGLISH} ""
LangString TEXT_IO_INSTALL		${LANG_ENGLISH} "Install"
LangString TEXT_IO_PLAY			${LANG_ENGLISH} "Play"
LangString TEXT_IO_REINSTALL	${LANG_ENGLISH} "Reinstall"
LangString TEXT_IO_TITLE		${LANG_FRENCH} "${PRODUCT_NAME_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_FRENCH} ""
LangString TEXT_IO_INSTALL		${LANG_FRENCH} "Install (S)"
LangString TEXT_IO_PLAY			${LANG_FRENCH} "Play (S)"
LangString TEXT_IO_REINSTALL	${LANG_FRENCH} "Reinstall (S)"
LangString TEXT_IO_TITLE		${LANG_GERMAN} "${PRODUCT_NAME_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_GERMAN} ""
LangString TEXT_IO_INSTALL		${LANG_GERMAN} "Install (G)"
LangString TEXT_IO_PLAY			${LANG_GERMAN} "Play (G)"
LangString TEXT_IO_REINSTALL	${LANG_GERMAN} "Reinstall (G)"
LangString TEXT_IO_TITLE		${LANG_SPANISH} "${PRODUCT_NAME_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_SPANISH} ""
LangString TEXT_IO_INSTALL		${LANG_SPANISH} "Install (S)"
LangString TEXT_IO_PLAY			${LANG_SPANISH} "Play (S)"
LangString TEXT_IO_REINSTALL	${LANG_SPANISH} "Reinstall (S)"
LangString TEXT_IO_TITLE		${LANG_ITALIAN} "${PRODUCT_NAME_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_ITALIAN} ""
LangString TEXT_IO_INSTALL		${LANG_ITALIAN} "Install (I)"
LangString TEXT_IO_PLAY			${LANG_ITALIAN} "Play (I)"
LangString TEXT_IO_REINSTALL	${LANG_ITALIAN} "Reinstall (I)"

Var hwnd ; Window handle of the custom page

Function ShowCustom

	!insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"
	
	InstallOptions::initDialog /NOUNLOAD "$PLUGINSDIR\custom.ini"
	; In this mode InstallOptions returns the window handle so we can use it
	Pop $hwnd
	
	GetDlgItem $1 $HWNDPARENT 1 ; Next button
	ShowWindow $1 0
	
	
	StrCpy $R1 "$INSTDIR\uninst.exe"
	StrCpy $R2 "_="
	IfFileExists "$R1" uninstall_available
	StrCpy $R1 "$INSTDIR\uninstall.exe"
	StrCpy $R2 "_?="
	IfFileExists "$R1" uninstall_available

	GetDlgItem $1 $hwnd 1201 ; Second cutom control
	ShowWindow $1 0
	GetDlgItem $1 $hwnd 1202 ; Third cutom control
	ShowWindow $1 0
	Goto uninstall_done

	uninstall_available:
	GetDlgItem $1 $hwnd 1200 ; First cutom control
	ShowWindow $1 0

	uninstall_done:

	; Now show the dialog and wait for it to finish
	InstallOptions::show
	
	; Finally fetch the InstallOptions status value (we don't care what it is though)
	Pop $0

FunctionEnd

Function LeaveCustom

	; At this point the user has either pressed Next or one of our custom buttons
	; We find out which by reading from the INI file
	ReadINIStr $0 "$PLUGINSDIR\custom.ini" "Settings" "State"
	StrCmp $0 1 install
	StrCmp $0 2 play
	StrCmp $0 3 install
	Goto proceed

	install:
	Call PreInstall
	GoTo proceed
	
	play:
	Exec "$INSTDIR\Program\${PRODUCT_NAME}.exe"
	IfErrors play_error
	quit

	play_error:
	MessageBox MB_ICONEXCLAMATION "Could not execute $INSTDIR\Program\${PRODUCT_NAME}.exe"
	abort
	
	proceed:
	GetDlgItem $1 $HWNDPARENT 1 ; Next button
	ShowWindow $1 1

FunctionEnd

Function PreInstall

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
	MessageBox MB_YESNO|MB_ICONINFORMATION "The previous version of ${PRODUCT_ID} must be uninstalled before continuing.$\nDo you wish to continue?" IDYES do_uninstall_nsis
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
!ifdef DIRECTX_81_REDIST_PRESENT
	MessageBox MB_YESNO|MB_ICONINFORMATION "The latest version of DirectX (8.1 or higher) is required.$\n Do you wish to install DirectX 8.1 now?" IDNO ok
	Exec "DirectX81\dxsetup.exe"
	quit
	ok:
!else
	MessageBox MB_YESNO|MB_ICONINFORMATION "The latest version of DirectX (8.1 or higher) is strongly recommended.$\n Do you wish to visit Microsoft's site now?" IDNO ok
	ExecShell "" "http://www.microsoft.com/directx/"
	Abort
	ok:
!endif

FunctionEnd

Function .onInit

	; Force show language selection for debugging
	;!define MUI_LANGDLL_ALWAYSSHOW
	!insertmacro MUI_LANGDLL_DISPLAY

!ifdef SHOW_AUTORUN
	;
	; Extract files for the InstallOptions page
	;
	!insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "Installer\custom.ini" "custom.ini"
	;$PLUGINSDIR will automatically be removed when the installer closes
	InitPluginsDir
	
	WriteINIStr $PLUGINSDIR\custom.ini "Field 1" "Text" "$(TEXT_IO_INSTALL)"
	WriteINIStr $PLUGINSDIR\custom.ini "Field 2" "Text" "$(TEXT_IO_PLAY)"
	WriteINIStr $PLUGINSDIR\custom.ini "Field 3" "Text" "$(TEXT_IO_REINSTALL)"

	WriteINIStr $PLUGINSDIR\custom.ini "Field 4" "Text" "${PRODUCT_URL}"
	WriteINIStr $PLUGINSDIR\custom.ini "Field 4" "State" "${PRODUCT_URL}"
	File /oname=$PLUGINSDIR\image.bmp "Installer\custom.bmp"
	WriteINIStr $PLUGINSDIR\custom.ini "Field 5" "Text" $PLUGINSDIR\image.bmp	
!else
	call PreInstall
!endif

FunctionEnd

;--------------------------------
;Descriptions

	;USE A LANGUAGE STRING IF YOU WANT YOUR DESCRIPTIONS TO BE LANGAUGE SPECIFIC

	;Assign descriptions to sections
	!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
		!insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "A test section."
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

	; add delete commands to delete whatever files/registry keys/etc you installed here.
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_NAME}\${PRODUCT_ID}"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}"

!ifdef INSTALL_TYPE_EXTERNAL_PCKS | INSTALL_TYPE_INTERNAL_PCKS
	RMDir /r "$INSTDIR\pcks"
!endif

!ifdef INSTALL_SMPACKAGE
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\smpackage.exe\shell\open\command"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip"
!endif

!ifdef INSTALL_TYPE_NON_PCK_DATA
	Delete "$INSTDIR\Announcers\instructions.txt"
	RMDir "$INSTDIR\Announcers"

	Delete "$INSTDIR\BGAnimations\instructions.txt"
	RMDir "$INSTDIR\BGAnimations"

	RMDir /r "$INSTDIR\Cache"

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

	Delete "$INSTDIR\Docs\*.*"
	RMDir "$INSTDIR\Docs"

	Delete "$INSTDIR\Data\*.*"
	RMDir "$INSTDIR\Data"
!endif

	Delete "$INSTDIR\Program\${PRODUCT_NAME}.exe"
!ifdef INSTALL_SMPACKAGE
	Delete "$INSTDIR\Program\smpackage.exe"
!endif
	Delete "$INSTDIR\Program\${PRODUCT_NAME}.vdi"
	Delete "$INSTDIR\Program\msvcr71.dll"
	Delete "$INSTDIR\Program\msvcp71.dll"
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
	Delete "$INSTDIR\Save\MachineProfile\stats.html"
	RMDir "$INSTDIR\Save\MachineProfile"

	Delete "$INSTDIR\COPYING.txt"
	Delete "$INSTDIR\README-FIRST.html"
	Delete "$INSTDIR\NEWS"
	Delete "$INSTDIR\log.txt"
	Delete "$INSTDIR\info.txt"
	Delete "$INSTDIR\crashinfo.txt"
	Delete "$INSTDIR\${PRODUCT_NAME}.lnk"

	RMDir "$INSTDIR"	; will delete only if empty

	SetShellVarContext current
	Delete "$DESKTOP\Play StepMania CVS.lnk"
	Delete "$DESKTOP\${PRODUCT_NAME_VER}.lnk"
	; I'm being paranoid here:
	Delete "$SMPROGRAMS\${PRODUCT_ID}\*.*"
	RMDir "$SMPROGRAMS\${PRODUCT_ID}"

	Delete "$INSTDIR\Uninstall.exe"

	DeleteRegKey /ifempty HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_NAME}\${PRODUCT_ID}"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

	!insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd