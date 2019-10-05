; NSIS Install Script 
; created by 
;     BBF, GlennMaynard, ChrisDanford
; hacked by AJ for sm-ssc
; NOTE: this .NSI script is designed for NSIS v2.0+

;-------------------------------------------------------------------------------
;Includes

	!include "MUI.nsh"	; Modern UI

	; Product info is in a separate file so that people will change
	; the version info for the installer and the program at the same time.
	; It's confusing/annoying when the installer and shortcut text doesn't match
	; the title screen version text.
	!include "src\ProductInfo.inc"

;-------------------------------------------------------------------------------
;General

	!system "echo Compressing executables. This may take a moment ..." ignore
	!system "utils\upx Program\*.exe" ignore

	Name "${PRODUCT_DISPLAY}"
	OutFile "${PRODUCT_DISPLAY}.exe"

	Caption "${PRODUCT_DISPLAY} | install"
	UninstallCaption "${PRODUCT_DISPLAY} | uninstall"

	; Some default compiler settings (uncomment and change at will):
!ifdef COMPRESS
	SetCompress auto
!else
	SetCompress off
!endif
	SetDatablockOptimize on ; (can be off)
!ifdef CRC_CHECK
	CRCCheck on
!else
	CRCCheck off
!endif

	; don't forget to change this before releasing a new version.
	; wish this could be automated, but it requires "X.Y.Z.a" format. -aj
	VIProductVersion "5.0.0.5"
	VIAddVersionKey "ProductName" "${PRODUCT_ID}"
	VIAddVersionKey "FileVersion" "${PRODUCT_VER}"
	VIAddVersionKey "FileDescription" "${PRODUCT_ID} Installer"

	AutoCloseWindow true ; (can be true for the window go away automatically at end)
	; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
	SetDateSave on ; (can be on to have files restored to their orginal date)

	; I think it may need actual admin privs for writing to the registry... -aj
	;RequestExecutionLevel user

	; $INSTDIR is defined in .onInit below. Old line is left here just in case.
	;InstallDir "C:\Games\${PRODUCT_ID}"
	InstallDir "${INSTDIR}"
	InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}" ""

	; DirShow show ; (make this hide to not let the user change it)
	DirText "${PRODUCT_DISPLAY}"
	InstallColors /windows
	InstProgressFlags smooth

;-------------------------------------------------------------------------------
;Interface Settings

	!define MUI_HEADERIMAGE
	!define MUI_HEADERIMAGE_BITMAP "Installer\header-${PRODUCT_BITMAP}.bmp"
	!define MUI_WELCOMEFINISHPAGE_BITMAP "Installer\welcome-${PRODUCT_BITMAP}.bmp"
	!define MUI_ABORTWARNING
	!define MUI_ICON "Installer\install.ico"
	!define MUI_UNICON "Installer\uninstall.ico"

;-------------------------------------------------------------------------------
;Language Selection Dialog Settings

	;Remember the installer language
	!define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
	!define MUI_LANGDLL_REGISTRY_KEY "Software\${PRODUCT_ID}" 
	!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;-------------------------------------------------------------------------------
;Pages

!ifdef SHOW_AUTORUN
	Page custom ShowAutorun LeaveAutorun
!endif

	!insertmacro MUI_PAGE_WELCOME

	;!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

		# These indented statements modify settings for MUI_PAGE_FINISH
		!define MUI_FINISHPAGE_NOAUTOCLOSE

		!define MUI_FINISHPAGE_RUN "$INSTDIR\Program\StepMania-SSE2.exe"
		!define MUI_FINISHPAGE_RUN_NOTCHECKED
		!define MUI_FINISHPAGE_RUN_TEXT "$(TEXT_IO_LAUNCH_THE_GAME)"

	!insertmacro MUI_PAGE_FINISH

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;-------------------------------------------------------------------------------
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

	; generate, then include installer strings
	;!delfile "nsis_strings_temp.inc"

	!system '"Program\StepMania.exe" --ExportNsisStrings'
	!include "nsis_strings_temp.inc"

;-------------------------------------------------------------------------------
;Reserve Files
  
  ;These files should be inserted before other files in the data block
  ;Keep these lines before any File command
  ;Only for solid compression (by default, solid compression is enabled for BZIP2 and LZMA)
  
  !insertmacro MUI_RESERVEFILE_LANGDLL

;-------------------------------------------------------------------------------
;Utility Functions
!ifdef ASSOCIATE_SMZIP
!define SHCNE_ASSOCCHANGED 0x08000000
!define SHCNF_IDLIST 0
 
Function RefreshShellIcons
  ; By jerome tremblay - april 2003
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd
Function un.RefreshShellIcons
  ; By jerome tremblay - april 2003
  System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v \
  (${SHCNE_ASSOCCHANGED}, ${SHCNF_IDLIST}, 0, 0)'
FunctionEnd
!endif

;-------------------------------------------------------------------------------
;Installer Sections

Section "Main Section" SecMain

	; write out uninstaller
	SetOutPath "$INSTDIR"
	AllowSkipFiles off
	SetOverwrite on

!ifdef INSTALL_PROGRAM_LIBRARIES
	WriteUninstaller "$INSTDIR\uninstall.exe"

	; add registry entries
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}" "" "$INSTDIR"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "$(TEXT_IO_REMOVE_ONLY)"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayVersion" "$(PRODUCT_VER)"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "Comments" "StepMania 5 is a rhythm game simulator."
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "Publisher" "StepMania Team"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "URLInfoAbout" "http://www.stepmania.com/"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "URLUpdateInfo" "http://code.google.com/p/stepmania/"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninstall.exe"'
!endif

!ifdef INSTALL_EXTERNAL_PCKS
	; Do this copy before anything else.  It's the most likely to fail.  
	; Possible failure reasons are: scratched CD, user tried to copy the installer but forgot the pcks.
	CreateDirectory $INSTDIR\pcks
	CopyFiles "${EXTERNAL_PCK_DIR}\*.pck" $INSTDIR\pcks 650000	; assume a CD full of data
	IfErrors do_error_pck do_no_error_pck
	do_error_pck:
	MessageBox MB_OK|MB_ICONSTOP "$(TEXT_IO_FATAL_ERROR_COPYING_PCK)"
	Quit
	do_no_error_pck:
!endif

!ifdef ASSOCIATE_SMZIP
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "$(TEXT_IO_SMZIP_PACKAGE)"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\Program\StepMania.exe,1"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\Program\StepMania.exe" "%1"'
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip" "" "smzipfile"
!endif

!ifdef ASSOCIATE_SMURL
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\stepmania" "" "StepMania protocol handler"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\stepmania" "URL Protocol" ""
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\stepmania\DefaultIcon" "" "$INSTDIR\Program\StepMania.exe"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\stepmania\shell\open\command" "" '"$INSTDIR\Program\StepMania.exe" "%1"'
!endif

!ifdef INSTALL_NON_PCK_FILES
	CreateDirectory "$INSTDIR\Announcers"
	SetOutPath "$INSTDIR\Announcers"
	File "Announcers\instructions.txt"

	; background/movie related
	CreateDirectory "$INSTDIR\BGAnimations"
	SetOutPath "$INSTDIR\BGAnimations"
	File "BGAnimations\instructions.txt"

	CreateDirectory "$INSTDIR\BackgroundEffects"
	SetOutPath "$INSTDIR\"
	File /r /x CVS /x .svn "BackgroundEffects"

	CreateDirectory "$INSTDIR\BackgroundTransitions"
	SetOutPath "$INSTDIR\"
	File /r /x CVS /x .svn "BackgroundTransitions"

	;CreateDirectory "$INSTDIR\RandomMovies"
	;SetOutPath "$INSTDIR\RandomMovies"
	;File "RandomMovies\instructions.txt"
	; end background/movie related

	; Nobody should be using this directory anymore. -aj
	;CreateDirectory "$INSTDIR\CDTitles"
	;SetOutPath "$INSTDIR\CDTitles"
	;File "CDTitles\Instructions.txt"

	RMDir /r "$INSTDIR\Characters\default"
	CreateDirectory "$INSTDIR\Characters\default"
	SetOutPath "$INSTDIR\Characters"
	File /r /x CVS /x .svn "Characters\default"
	File "Characters\instructions.txt"

	CreateDirectory "$INSTDIR\Courses"
	SetOutPath "$INSTDIR\Courses"
	File "Courses\instructions.txt"
	File /r /x CVS /x .svn "Courses\Default"

	CreateDirectory "$INSTDIR\Packages"
	;File "Packages\Instructions.txt"

	; remove old noteskins
	RMDir /r "$INSTDIR\NoteSkins\common\default"
	RMDir /r "$INSTDIR\NoteSkins\common\_Editor"
	; dance
	RMDir /r "$INSTDIR\NoteSkins\dance\default"
	RMDir /r "$INSTDIR\NoteSkins\dance\Delta"
	; the "midi-*" noteskin series was formerly known as just "midi".
	RMDir /r "$INSTDIR\NoteSkins\dance\midi"
	; we may also want to remove the new ones.
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-note"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-note-3d"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-solo"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-vivid"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-vivid-3d"
	; old names
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-rhythm-p1"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-rhythm-p2"
	; new names
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-routine-p2"
	; retro, retrobar.
	RMDir /r "$INSTDIR\NoteSkins\dance\retro"
	RMDir /r "$INSTDIR\NoteSkins\dance\retrobar"
	RMDir /r "$INSTDIR\NoteSkins\dance\retrobar-splithand_whiteblue"
	; pump
	RMDir /r "$INSTDIR\NoteSkins\pump\cmd"
	RMDir /r "$INSTDIR\NoteSkins\pump\cmd-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\pump\cmd-routine-p2"
	RMDir /r "$INSTDIR\NoteSkins\pump\complex"
	RMDir /r "$INSTDIR\NoteSkins\pump\default"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-note"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-routine-p2"
	RMDir /r "$INSTDIR\NoteSkins\pump\frame5p"
	RMDir /r "$INSTDIR\NoteSkins\pump\newextra"
	RMDir /r "$INSTDIR\NoteSkins\pump\pad"
	RMDir /r "$INSTDIR\NoteSkins\pump\rhythm"
	RMDir /r "$INSTDIR\NoteSkins\pump\simple"
	; kb7
	RMDir /r "$INSTDIR\NoteSkins\kb7\default"
	RMDir /r "$INSTDIR\NoteSkins\kb7\orbital"
	; retrobar for kb7
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-iidx"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-o2jam"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-razor"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-razor_o2"
	; lights
	RMDir /r "$INSTDIR\NoteSkins\lights\default"
	SetOutPath "$INSTDIR\NoteSkins"
	File "NoteSkins\instructions.txt"

	; common noteskin
	SetOutPath "$INSTDIR\NoteSkins\common"
	File /r /x CVS /x .svn "NoteSkins\common\common"
	File /r /x CVS /x .svn "NoteSkins\common\_Editor"

	; install dance noteskins
	SetOutPath "$INSTDIR\NoteSkins\dance"
	File /r /x CVS /x .svn "NoteSkins\dance\default"
	File /r /x CVS /x .svn "NoteSkins\dance\Delta"
	; "midi" noteskin was split out after beta 2.
	File /r /x CVS /x .svn "NoteSkins\dance\midi-note"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-note-3d"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-routine-p1"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-routine-p2"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-solo"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-vivid"
	File /r /x CVS /x .svn "NoteSkins\dance\midi-vivid-3d"
	; retro and retrobar
	File /r /x CVS /x .svn "NoteSkins\dance\retro"
	File /r /x CVS /x .svn "NoteSkins\dance\retrobar"
	File /r /x CVS /x .svn "NoteSkins\dance\retrobar-splithand_whiteblue"
	SetOutPath "$INSTDIR"

	; install pump noteskins
	SetOutPath "$INSTDIR\NoteSkins\pump"
	File /r /x CVS /x .svn "NoteSkins\pump\cmd"
	File /r /x CVS /x .svn "NoteSkins\pump\cmd-routine-p1"
	File /r /x CVS /x .svn "NoteSkins\pump\cmd-routine-p2"
	File /r /x CVS /x .svn "NoteSkins\pump\complex"
	File /r /x CVS /x .svn "NoteSkins\pump\default"
	File /r /x CVS /x .svn "NoteSkins\pump\delta"
	File /r /x CVS /x .svn "NoteSkins\pump\delta-note"
	File /r /x CVS /x .svn "NoteSkins\pump\delta-routine-p1"
	File /r /x CVS /x .svn "NoteSkins\pump\delta-routine-p2"
	File /r /x CVS /x .svn "NoteSkins\pump\frame5p"
	File /r /x CVS /x .svn "NoteSkins\pump\newextra"
	File /r /x CVS /x .svn "NoteSkins\pump\rhythm"
	File /r /x CVS /x .svn "NoteSkins\pump\simple"
	SetOutPath "$INSTDIR"

	; install kb7 noteskins
	SetOutPath "$INSTDIR\NoteSkins\kb7"
	File /r /x CVS /x .svn "NoteSkins\kb7\default"
	;File /r /x CVS /x .svn "NoteSkins\kb7\orbital"
	; retrobar
	File /r /x CVS /x .svn "NoteSkins\kb7\retrobar"
	File /r /x CVS /x .svn "NoteSkins\kb7\retrobar-iidx"
	File /r /x CVS /x .svn "NoteSkins\kb7\retrobar-o2jam"
	File /r /x CVS /x .svn "NoteSkins\kb7\retrobar-razor"
	File /r /x CVS /x .svn "NoteSkins\kb7\retrobar-razor_o2"
	SetOutPath "$INSTDIR"

	; install lights noteskin
	SetOutPath "$INSTDIR\NoteSkins\lights"
	File /r /x CVS /x .svn "NoteSkins\lights\default"
	SetOutPath "$INSTDIR"

	; make songs dir
	CreateDirectory "$INSTDIR\Songs"
	SetOutPath "$INSTDIR\Songs"
	;File "Songs\Instructions.txt"
	File /r /x CVS /x .svn "Songs\*"

	; remove and install themes
	RMDir /r "$INSTDIR\Themes\_fallback"
	RMDir /r "$INSTDIR\Themes\_portKit-sm4"
	RMDir /r "$INSTDIR\Themes\default"
	CreateDirectory "$INSTDIR\Themes"
	SetOutPath "$INSTDIR\Themes"
	;File "Themes\instructions.txt"
	File /r /x CVS /x .svn "Themes\_fallback"
	; no more portkit sm4
	;File /r /x CVS /x .svn "Themes\_portKit-sm4"
	File /r /x CVS /x .svn "Themes\default"

	CreateDirectory "$INSTDIR\Data"
	SetOutPath "$INSTDIR\Data"
	File /r /x CVS /x .svn "Data\*"
!endif

!ifdef INSTALL_INTERNAL_PCKS
	CreateDirectory "$INSTDIR\pcks"
	SetOutPath "$INSTDIR\pcks"
	File /r "pcks\*.*"
!endif

	SetOutPath "$INSTDIR\Program"
!ifdef INSTALL_EXECUTABLES
	; normal exec
	File "Program\StepMania.exe"
	File "Program\StepMania.vdi"
	; sse2 exec
	File "Program\StepMania-SSE2.exe"
	File "Program\StepMania-SSE2.vdi"
	; other programs
	File "Program\Texture Font Generator.exe"
	; AJ can never get this built properly:
	;File "Program\tools.exe" ; to be replaced eventually
!endif
!ifdef ASSOCIATE_SMZIP
	Call RefreshShellIcons
!endif
!ifdef ASSOCIATE_SMURL
	Call RefreshShellIcons
!endif
!ifdef INSTALL_PROGRAM_LIBRARIES
	; microsoft!
	; xxx: how many of these do we really need?
	File "Program\msvcp100.dll"
	File "Program\msvcr100.dll"
	File "Program\msvcp110.dll"
	File "Program\msvcr110.dll"
	File "Program\vccorlib110.dll"
	; FFmpeg and related
	File "Program\avcodec-53.dll"
	;File "Program\avdevice-52.dll"
	File "Program\avformat-53.dll"
	File "Program\avutil-51.dll"
	File "Program\swscale-2.dll"
	; parallel lights
	File "Program\parallel_lights_io.dll"
	; others
	File "Program\dbghelp.dll"
	File "Program\jpeg.dll"
	File "Program\zlib1.dll"

	; documentation
	CreateDirectory "$INSTDIR\Docs"
	SetOutPath "$INSTDIR\Docs"
	File "Docs\Licenses.txt"
	File "Docs\credits.txt"
	File "Docs\credits_old_Stepmania_Team.txt"
	File "Docs\Changelog_sm-ssc.txt"
	File "Docs\Changelog_sm5.txt"
	File "Docs\Changelog_SSCformat.txt"
	File "Docs\CommandLineArgs.txt"
	File "Docs\CourseFormat.txt"
	File "Docs\Userdocs\sm5_beginner.txt"
	File /r /x CVS /x .svn "Docs\license-ext"
	File /r /x CVS /x .svn "Docs\Luadoc"
	File /r /x CVS /x .svn "Docs\Themerdocs"

	CreateDirectory "$INSTDIR\Manual"
	SetOutPath "$INSTDIR\Manual"
	File /r /x CVS /x .svn /x _* /x desktop.ini "Manual\*.*"

	; Create Start Menu icons
	SetShellVarContext current  # 	'all' doesn't work on Win9x
	CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
	; todo: make desktop shortcut an option
	!ifdef MAKE_DESKTOP_SHORTCUT
		CreateShortCut "$DESKTOP\$(TEXT_IO_RUN).lnk" "$INSTDIR\Program\StepMania-SSE2.exe"
	!endif

	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_RUN).lnk" "$INSTDIR\Program\StepMania-SSE2.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_RUN_WITHOUT_SSE2).lnk" "$INSTDIR\Program\StepMania.exe"

	!ifdef MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
		CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_OPEN_PROGRAM_FOLDER).lnk" "$WINDIR\explorer.exe" "$INSTDIR\"
	!endif
	!ifdef MAKE_OPEN_SETTINGS_FOLDER_SHORTCUT
		CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_OPEN_SETTINGS_FOLDER).lnk" "$WINDIR\explorer.exe" "$APPDATA\${PRODUCT_ID}"
	!endif

	;CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_VIEW_STATISTICS).lnk" "$INSTDIR\Program\tools.exe" "--machine-profile-stats"
	;CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_TOOLS).lnk" "$INSTDIR\Program\tools.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_MANUAL).lnk" "$INSTDIR\Manual\index.html"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_UNINSTALL).lnk" "$INSTDIR\uninstall.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_WEB_SITE).lnk" "${PRODUCT_URL}"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_TEXTURE_FONT_GENERATOR).lnk" "$INSTDIR\Program\Texture Font Generator.exe"
	!ifdef MAKE_UPDATES_SHORTCUT
		CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_CHECK_FOR_UPDATES).lnk" "${UPDATES_URL}"
	!endif
	CreateShortCut "$INSTDIR\${PRODUCT_ID}.lnk" "$INSTDIR\Program\StepMania-SSE2.exe"
	CreateShortCut "$INSTDIR\${PRODUCT_ID} (non-SSE2).lnk" "$INSTDIR\Program\StepMania.exe"
!endif

	IfErrors do_error do_no_error
	do_error:
	MessageBox MB_OK|MB_ICONSTOP "$(TEXT_IO_FATAL_ERROR_INSTALL)"
	Quit
	do_no_error:

SectionEnd

;-------------------------------------------------------------------------------
;Installer Functions

!ifdef SHOW_AUTORUN
Var hwnd ; Window handle of the custom page

Function ShowAutorun

	!insertmacro MUI_HEADER_TEXT "$(TEXT_IO_TITLE)" "$(TEXT_IO_SUBTITLE)"

	InstallOptions::initDialog /NOUNLOAD "$PLUGINSDIR\custom.ini"
	; In this mode InstallOptions returns the window handle so we can use it
	Pop $hwnd

	GetDlgItem $1 $HWNDPARENT 1 ; Next button
	ShowWindow $1 0

!ifdef AUTORUN_SHOW_ONLY_INSTALL
	Goto show_only_install
!endif

	StrCpy $R1 "$INSTDIR\uninst.exe"
	StrCpy $R2 "_="
	IfFileExists "$R1" show_play_and_reinstall
	StrCpy $R1 "$INSTDIR\uninstall.exe"
	StrCpy $R2 "_?="
	IfFileExists "$R1" show_play_and_reinstall

	show_only_install:
	GetDlgItem $1 $hwnd 1201 ; Second custom control
	ShowWindow $1 0
	GetDlgItem $1 $hwnd 1202 ; Third custom control
	ShowWindow $1 0
	Goto done

	show_play_and_reinstall:
	GetDlgItem $1 $hwnd 1200 ; First custom control
	ShowWindow $1 0

	done:

	; Now show the dialog and wait for it to finish
	InstallOptions::show

	; Finally fetch the InstallOptions status value (we don't care what it is though)
	Pop $0

FunctionEnd

Function LeaveAutorun

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
	Exec "$INSTDIR\Program\StepMania-SSE2.exe"
	IfErrors play_error
	quit

	play_error:
	MessageBox MB_ICONEXCLAMATION "$(TEXT_IO_COULD_NOT_EXECUTE)"
	abort

	proceed:
	GetDlgItem $1 $HWNDPARENT 1 ; Next button
	ShowWindow $1 1

FunctionEnd
!endif

Function PreInstall

!ifdef INSTALL_PROGRAM_LIBRARIES
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
		IfSilent do_uninstall_nsis
		MessageBox MB_YESNO|MB_ICONINFORMATION "$(TEXT_IO_UNINSTALL_PREVIOUS)" IDYES do_uninstall_nsis
		Abort

		do_uninstall_nsis:
		GetTempFileName $R3
		CopyFiles /SILENT $R1 $R3
		IfSilent 0 +3
		ExecWait '$R3 /S $R2$INSTDIR' $R4 ; uninstall silently
		Goto +2
		ExecWait '$R3 $R2$INSTDIR' $R4 ; uninstall prompt
		; Delete the copy of the installer.
		Delete $R3

		; $R4 is the exit value of the uninstaller.  0 means success, anything else is
		; failure (eg. aborted).
		IntCmp $R4 0 old_nsis_not_installed ; jump if 0

		MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONINFORMATION "$(TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY)" IDYES old_nsis_not_installed
		Abort


		old_nsis_not_installed:

		; todo: this needs to be updated for DirectX 9.0c
		; HKEY_LOCAL_MACHINE "Software\Microsoft\DirectX" "Version"
		; 9.0c is "4.09.00.0904"

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
		MessageBox MB_YESNO|MB_ICONINFORMATION "$(TEXT_IO_INSTALL_DIRECTX)" IDNO ok
		Exec "DirectX81\dxsetup.exe"
		quit
		ok:
	!else
		MessageBox MB_YESNO|MB_ICONINFORMATION "$(TEXT_IO_DIRECTX_VISIT_MICROSOFT)" IDNO ok
		ExecShell "" "http://www.microsoft.com/directx/"
		Abort
		ok:
	!endif
!else
		; Check that full version is installed.
		IfFileExists "$INSTDIR\Program\StepMania.exe" proceed_with_patch
		MessageBox MB_YESNO|MB_ICONINFORMATION "$(TEXT_IO_FULL_INSTALL_NOT_FOUND)" IDYES proceed_with_patch
		Abort
		proceed_with_patch:
!endif

FunctionEnd

Function .onInit

	; Force show language selection for debugging
	;!define MUI_LANGDLL_ALWAYSSHOW
	!insertmacro MUI_LANGDLL_DISPLAY

	; determine root drive where Windows was installed, and suggest a
	; reasonable directory for installation.
	StrCpy $0 "$WINDIR" 2
	StrCpy $INSTDIR "$0\Games\${PRODUCT_ID}"

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
	File /oname=$PLUGINSDIR\image.bmp "Installer\custom-${PRODUCT_BITMAP}.bmp"
	WriteINIStr $PLUGINSDIR\custom.ini "Field 5" "Text" $PLUGINSDIR\image.bmp	
!else

	call PreInstall
!endif

FunctionEnd
 
;-------------------------------------------------------------------------------
;Uninstaller Section
; todo: update big time

Section "Uninstall"

	; add delete commands to delete whatever files/registry keys/etc you installed here.
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}"

!ifdef INSTALL_EXTERNAL_PCKS | INSTALL_INTERNAL_PCKS
	RMDir /r "$INSTDIR\pcks"
!endif

!ifdef ASSOCIATE_SMZIP
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\.smzip"
!endif

!ifdef ASSOCIATE_SMURL
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\stepmania"
!endif

!ifdef INSTALL_NON_PCK_FILES
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
	RMDir /r "$INSTDIR\Courses\Default"
	RMDir "$INSTDIR\Courses"

	Delete "$INSTDIR\NoteSkins\instructions.txt"
	RMDir /r "$INSTDIR\NoteSkins\common\default"
	RMDir "$INSTDIR\NoteSkins\common"
	RMDir /r "$INSTDIR\NoteSkins\dance\default"
	RMDir /r "$INSTDIR\NoteSkins\dance\Delta"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-note"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-note-3d"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-routine-p2"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-solo"
	RMDir /r "$INSTDIR\NoteSkins\dance\midi-vivid"
	RMDir /r "$INSTDIR\NoteSkins\dance\retro"
	RMDir /r "$INSTDIR\NoteSkins\dance\retrobar"
	RMDir /r "$INSTDIR\NoteSkins\dance\retrobar-splithand_whiteblue"
	RMDir "$INSTDIR\NoteSkins\dance"

	RMDir /r "$INSTDIR\NoteSkins\pump\cmd"
	RMDir /r "$INSTDIR\NoteSkins\pump\cmd-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\pump\cmd-routine-p2"
	RMDir /r "$INSTDIR\NoteSkins\pump\complex"
	RMDir /r "$INSTDIR\NoteSkins\pump\default"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-note"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-routine-p1"
	RMDir /r "$INSTDIR\NoteSkins\pump\delta-routine-p2"
	RMDir /r "$INSTDIR\NoteSkins\pump\frame5p"
	RMDir /r "$INSTDIR\NoteSkins\pump\newextra"
	RMDir /r "$INSTDIR\NoteSkins\pump\rhythm"
	RMDir /r "$INSTDIR\NoteSkins\pump\simple"
	RMDir "$INSTDIR\NoteSkins\pump"

	RMDir /r "$INSTDIR\NoteSkins\kb7\default"
	RMDir /r "$INSTDIR\NoteSkins\kb7\orbital"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-iidx"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-o2jam"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-razor"
	RMDir /r "$INSTDIR\NoteSkins\kb7\retrobar-razor_o2"
	RMDir "$INSTDIR\NoteSkins\kb7"

	; we don't currently install para noteskins...
	;RMDir /r "$INSTDIR\NoteSkins\para\default"
	;RMDir "$INSTDIR\NoteSkins\para"
	;RMDir "$INSTDIR\NoteSkins"

	RMDir /r "$INSTDIR\BackgroundEffects"

	RMDir /r "$INSTDIR\BackgroundTransitions"

	Delete "$INSTDIR\RandomMovies\instructions.txt"
	RMDir "$INSTDIR\RandomMovies"

	Delete "$INSTDIR\Songs\Instructions.txt"
	RMDir "$INSTDIR\Songs"	; will delete only if empty

	Delete "$INSTDIR\Themes\instructions.txt"
	RMDir /r "$INSTDIR\Themes\_fallback"
	RMDir /r "$INSTDIR\Themes\_portKit-sm4"
	RMDir /r "$INSTDIR\Themes\default"
	RMDir "$INSTDIR\Themes"

	Delete "$INSTDIR\Data\*.*"
	RMDir "$INSTDIR\Data"
!endif

!ifdef INSTALL_EXECUTABLES
	Delete "$INSTDIR\Program\StepMania.exe"
	Delete "$INSTDIR\Program\StepMania.vdi"
	Delete "$INSTDIR\Program\StepMania-SSE2.exe"
	Delete "$INSTDIR\Program\StepMania-SSE2.vdi"
	Delete "$INSTDIR\Program\tools.exe"
	Delete "$INSTDIR\Program\Texture Font Generator.exe"
!endif
!ifdef ASSOCIATE_SMZIP
	Call un.RefreshShellIcons
!endif
!ifdef ASSOCIATE_SMURL
	Call un.RefreshShellIcons
!endif
!ifdef INSTALL_PROGRAM_LIBRARIES
	Delete "$INSTDIR\Program\mfc71.dll"
	Delete "$INSTDIR\Program\msvcr71.dll"
	Delete "$INSTDIR\Program\msvcp71.dll"
	Delete "$INSTDIR\Program\msvcr80.dll"
	Delete "$INSTDIR\Program\msvcp80.dll"
	Delete "$INSTDIR\Program\msvcr90.dll"
	Delete "$INSTDIR\Program\msvcp90.dll"
	; FFmpeg and related
	Delete "$INSTDIR\Program\avcodec-53.dll"
	Delete "$INSTDIR\Program\avcodec-52.dll"
	Delete "$INSTDIR\Program\avdevice-52.dll"
	Delete "$INSTDIR\Program\avformat-53.dll"
	Delete "$INSTDIR\Program\avformat-52.dll"
	Delete "$INSTDIR\Program\avutil-51.dll"
	Delete "$INSTDIR\Program\avutil-50.dll"
	Delete "$INSTDIR\Program\swscale-2.dll"
	Delete "$INSTDIR\Program\swscale-0.dll"
	; others
	Delete "$INSTDIR\Program\dbghelp.dll"
	Delete "$INSTDIR\Program\jpeg.dll"
	Delete "$INSTDIR\Program\zlib1.dll"
	RMDir "$INSTDIR\Program"

	Delete "$INSTDIR\Docs\Licenses.txt"
	RMDir /r "$INSTDIR\Manual"
!endif

	Delete "$INSTDIR\log.txt"
	Delete "$INSTDIR\info.txt"
	Delete "$INSTDIR\crashinfo.txt"
	Delete "$INSTDIR\${PRODUCT_ID}.lnk"
	Delete "$INSTDIR\${PRODUCT_ID} (non-SSE2).lnk"

	RMDir "$INSTDIR"	; will delete only if empty

	SetShellVarContext current

	; kill shortcuts
	!ifdef MAKE_DESKTOP_SHORTCUT
		Delete "$DESKTOP\$(TEXT_IO_RUN).lnk"
	!endif
	!ifdef MAKE_DESKTOP_SHORTCUT
		Delete "$DESKTOP\$(TEXT_IO_RUN).lnk"
	!endif

	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_RUN).lnk"'
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_RUN_WITHOUT_SSE2).lnk"'

	!ifdef MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
		Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_OPEN_PROGRAM_FOLDER).lnk"'
	!endif
	!ifdef MAKE_OPEN_SETTINGS_FOLDER_SHORTCUT
		Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_OPEN_SETTINGS_FOLDER).lnk"'
	!endif

	;Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_VIEW_STATISTICS).lnk"'
	;Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_TOOLS).lnk"'
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_MANUAL).lnk"'
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_UNINSTALL).lnk"'
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_WEB_SITE).lnk"'
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_TEXTURE_FONT_GENERATOR).lnk"'
	!ifdef MAKE_UPDATES_SHORTCUT
		Delete '"$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_CHECK_FOR_UPDATES).lnk"'
	!endif
	Delete '"$INSTDIR\${PRODUCT_ID}.lnk"'
	Delete '"$INSTDIR\${PRODUCT_ID} (non-SSE2).lnk"'

	; I'm being paranoid here:
	Delete '"$SMPROGRAMS\${PRODUCT_ID}\*.*"'
	RMDir '"$SMPROGRAMS\${PRODUCT_ID}"'

	Delete "$INSTDIR\Uninstall.exe"

	DeleteRegKey /ifempty HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}"

SectionEnd

;-------------------------------------------------------------------------------
;Uninstaller Functions

Function un.onInit

	!insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
