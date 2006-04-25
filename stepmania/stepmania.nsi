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

	Name "${PRODUCT_ID_VER}"
	OutFile "${PRODUCT_ID_VER}.exe"

	Caption "${PRODUCT_ID_VER}"
	UninstallCaption "${PRODUCT_ID_VER}"

	; Some default compiler settings (uncomment and change at will):
	SetCompress auto ; (can be off or force)
	SetDatablockOptimize on ; (can be off)
	CRCCheck on ; (can be off)
	AutoCloseWindow true ; (can be true for the window go away automatically at end)
	; ShowInstDetails hide ; (can be show to have them shown, or nevershow to disable)
	SetDateSave on ; (can be on to have files restored to their orginal date)
	InstallDir "$PROGRAMFILES\${PRODUCT_ID}"
	InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}" ""
	; DirShow show ; (make this hide to not let the user change it)
	DirText "${PRODUCT_ID_VER}"
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
	!define MUI_LANGDLL_REGISTRY_KEY "Software\${PRODUCT_ID}" 
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

;--------------------------------
;Installer Sections

LangString TEXT_IO_CHECK_FOR_UPDATES		${LANG_ENGLISH} "Check for Updates"
LangString TEXT_IO_RUN						${LANG_ENGLISH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_OPEN_PROGRAM_FOLDER		${LANG_ENGLISH} "Open ${PRODUCT_ID} Program Folder"
LangString TEXT_IO_MANUAL					${LANG_ENGLISH} "${PRODUCT_ID} Manual"
LangString TEXT_IO_TOOLS					${LANG_ENGLISH} "${PRODUCT_ID} Tools"
LangString TEXT_IO_WEB_SITE					${LANG_ENGLISH} "${PRODUCT_ID} Web Site"
LangString TEXT_IO_UNINSTALL				${LANG_ENGLISH} "Uninstall ${PRODUCT_ID_VER}"
LangString TEXT_IO_VIEW_STATISTICS			${LANG_ENGLISH} "View Statistics"
LangString TEXT_IO_REMOVE_ONLY				${LANG_ENGLISH} "${PRODUCT_ID_VER} (remove only)"
LangString TEXT_IO_SMZIP_PACKAGE			${LANG_ENGLISH} "SMZIP package"
LangString TEXT_IO_FATAL_ERROR_COPYING_PCK	${LANG_ENGLISH} "Fatal error copying pck files."
LangString TEXT_IO_FATAL_ERROR_INSTALL		${LANG_ENGLISH} "Fatal error during install."
LangString TEXT_IO_CHECK_FOR_UPDATES		${LANG_FRENCH} "Vérifier les mises à jour"
LangString TEXT_IO_RUN						${LANG_FRENCH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_OPEN_PROGRAM_FOLDER		${LANG_FRENCH} "Open ${PRODUCT_ID} Program Folder"
LangString TEXT_IO_MANUAL					${LANG_FRENCH} "Documentation de ${PRODUCT_ID}"
LangString TEXT_IO_TOOLS					${LANG_FRENCH} "d'Outils de ${PRODUCT_ID}"
LangString TEXT_IO_WEB_SITE					${LANG_FRENCH} "Le site web de ${PRODUCT_ID}"
LangString TEXT_IO_UNINSTALL				${LANG_FRENCH} "Désinstaller ${PRODUCT_ID_VER}"
LangString TEXT_IO_VIEW_STATISTICS			${LANG_FRENCH} "Voir les statistiques"
LangString TEXT_IO_REMOVE_ONLY				${LANG_FRENCH} "${PRODUCT_ID_VER} (Supprimer uniquement)"
LangString TEXT_IO_SMZIP_PACKAGE			${LANG_FRENCH} "SMZIP package"
LangString TEXT_IO_FATAL_ERROR_COPYING_PCK	${LANG_FRENCH} "Fatal error copying pck files."
LangString TEXT_IO_FATAL_ERROR_INSTALL		${LANG_FRENCH} "Fatal error during install."
LangString TEXT_IO_CHECK_FOR_UPDATES		${LANG_GERMAN} "Updates zu überprüfen"
LangString TEXT_IO_RUN						${LANG_GERMAN} "${PRODUCT_ID_VER}"
LangString TEXT_IO_OPEN_PROGRAM_FOLDER		${LANG_GERMAN} "Open ${PRODUCT_ID} Program Folder"
LangString TEXT_IO_MANUAL					${LANG_GERMAN} "${PRODUCT_ID} Dokumentation"
LangString TEXT_IO_TOOLS					${LANG_GERMAN} "${PRODUCT_ID} Werkzeug"
LangString TEXT_IO_WEB_SITE					${LANG_GERMAN} "${PRODUCT_ID} Webseite"
LangString TEXT_IO_UNINSTALL				${LANG_GERMAN} "Deinstallation von ${PRODUCT_ID_VER}"
LangString TEXT_IO_VIEW_STATISTICS			${LANG_GERMAN} "Statistiken anschauen"
LangString TEXT_IO_REMOVE_ONLY				${LANG_GERMAN} "${PRODUCT_ID_VER} (Nur entfernen)"
LangString TEXT_IO_SMZIP_PACKAGE			${LANG_GERMAN} "SMZIP package"
LangString TEXT_IO_FATAL_ERROR_COPYING_PCK	${LANG_GERMAN} "Fatal error copying pck files."
LangString TEXT_IO_FATAL_ERROR_INSTALL		${LANG_GERMAN} "Fatal error during install."
LangString TEXT_IO_CHECK_FOR_UPDATES		${LANG_SPANISH} "Compruebe para actualizaciones"
LangString TEXT_IO_RUN						${LANG_SPANISH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_OPEN_PROGRAM_FOLDER		${LANG_SPANISH} "Open ${PRODUCT_ID} Program Folder"
LangString TEXT_IO_MANUAL					${LANG_SPANISH} "${PRODUCT_ID} Documenatione"
LangString TEXT_IO_TOOLS					${LANG_SPANISH} "${PRODUCT_ID} Herramientas"
LangString TEXT_IO_WEB_SITE					${LANG_SPANISH} "${PRODUCT_ID} Web Site"
LangString TEXT_IO_UNINSTALL				${LANG_SPANISH} "Uninstall ${PRODUCT_ID_VER}"
LangString TEXT_IO_VIEW_STATISTICS			${LANG_SPANISH} "Vea la estadística"
LangString TEXT_IO_REMOVE_ONLY				${LANG_SPANISH} "${PRODUCT_ID_VER} (Quite solamente)"
LangString TEXT_IO_SMZIP_PACKAGE			${LANG_SPANISH} "SMZIP package"
LangString TEXT_IO_FATAL_ERROR_COPYING_PCK	${LANG_SPANISH} "Fatal error copying pck files."
LangString TEXT_IO_FATAL_ERROR_INSTALL		${LANG_SPANISH} "Fatal error during install."
LangString TEXT_IO_CHECK_FOR_UPDATES		${LANG_ITALIAN} "Per cercare aggiornamenti"
LangString TEXT_IO_RUN						${LANG_ITALIAN} "${PRODUCT_ID_VER}"
LangString TEXT_IO_OPEN_PROGRAM_FOLDER		${LANG_ITALIAN} "Open ${PRODUCT_ID} Program Folder"
LangString TEXT_IO_MANUAL					${LANG_ITALIAN} "${PRODUCT_ID} Documentazione"
LangString TEXT_IO_TOOLS					${LANG_ITALIAN} "${PRODUCT_ID} Strumenti"
LangString TEXT_IO_WEB_SITE					${LANG_ITALIAN} "${PRODUCT_ID} Sito Web"
LangString TEXT_IO_UNINSTALL				${LANG_ITALIAN} "Disinstalla ${PRODUCT_ID_VER}"
LangString TEXT_IO_VIEW_STATISTICS			${LANG_ITALIAN} "Mostra Statistiche"
LangString TEXT_IO_REMOVE_ONLY				${LANG_ITALIAN} "${PRODUCT_ID_VER} (Elimina soltanto)"
LangString TEXT_IO_SMZIP_PACKAGE			${LANG_ITALIAN} "SMZIP package"
LangString TEXT_IO_FATAL_ERROR_COPYING_PCK	${LANG_ITALIAN} "Fatal error copying pck files."
LangString TEXT_IO_FATAL_ERROR_INSTALL		${LANG_ITALIAN} "Fatal error during install."

Section "Main Section" SecMain

	; write out uninstaller
	SetOutPath "$INSTDIR"
	AllowSkipFiles off
	SetOverwrite on
	WriteUninstaller "$INSTDIR\uninstall.exe"

	; add registry entries
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}" "" "$INSTDIR"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "DisplayName" "$(TEXT_IO_REMOVE_ONLY)"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}" "UninstallString" '"$INSTDIR\uninstall.exe"'

!ifdef INSTALL_TYPE_EXTERNAL_PCKS
	; Do this copy before anything else.  It's the most likely to fail.  
	; Possible failure reasons are: scratched CD, user tried to copy the installer but forgot the pcks.
	CreateDirectory $INSTDIR\pcks
	CopyFiles /SILENT "$EXEDIR\${PRODUCT_ID}.app\Contents\Resources\pcks\*.idx" $INSTDIR\pcks 1
	CopyFiles /SILENT "$EXEDIR\${PRODUCT_ID}.app\Contents\Resources\pcks\*.pck" $INSTDIR\pcks 650000	; assume a CD full of data
	IfErrors do_error do_no_error
	do_error:
	MessageBox MB_OK|MB_ICONSTOP "$(TEXT_IO_FATAL_ERROR_COPYING_PCK)"
	Quit
	do_no_error:
!endif

!ifdef ASSOCIATE_SMZIP
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\tools.exe\shell\open\command" "" '"$INSTDIR\Program\tools.exe" "%1"'
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile" "" "$(TEXT_IO_SMZIP_PACKAGE)"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\DefaultIcon" "" "$INSTDIR\Program\tools.exe,0"
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Classes\smzipfile\shell\open\command" "" '"$INSTDIR\Program\tools.exe" "%1"'
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

	SetOutPath "$INSTDIR\NoteSkins\para"
	File /r "NoteSkins\para\default"

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

	CreateDirectory "$INSTDIR\Data"
	SetOutPath "$INSTDIR\Data"
	File "Data\*.*"
!endif

!ifdef INSTALL_TYPE_INTERNAL_PCKS
	CreateDirectory "$INSTDIR\pcks"
	SetOutPath "$INSTDIR\pcks"
	File "pcks\*.*"
!endif

	SetOutPath "$INSTDIR\Program"
	File "Program\${PRODUCT_FAMILY}.exe"
	File "Program\${PRODUCT_FAMILY}.vdi"
	File "Program\tools.exe"
	File "Program\mfc71.dll"
!ifdef ASSOCIATE_SMZIP
	Call RefreshShellIcons
!endif
	File "Program\msvcr71.dll"
	File "Program\msvcp71.dll"
	File "Program\jpeg.dll"
	File "Program\avcodec.dll"
	File "Program\avformat.dll"
	File "Program\dbghelp.dll"
	File "Program\zlib1.dll"

	SetOutPath "$INSTDIR"
	File "Docs\Licenses.txt"

	CreateDirectory "$INSTDIR\Manual"
	SetOutPath "$INSTDIR\Manual"
	File "Manual\*.*"
	CreateDirectory "$INSTDIR\Manual\images"
	SetOutPath "$INSTDIR\Manual\images"
	File "Manual\images\*.*"

	; Create Start Menu icons
	SetShellVarContext current  # 	'all' doesn't work on Win9x
	CreateDirectory "$SMPROGRAMS\${PRODUCT_ID}\"
	CreateShortCut "$DESKTOP\$(TEXT_IO_RUN).lnk" "$INSTDIR\Program\${PRODUCT_FAMILY}.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_RUN).lnk" "$INSTDIR\Program\${PRODUCT_FAMILY}.exe"
!ifdef MAKE_OPEN_PROGRAM_FOLDER_SHORTCUT
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_OPEN_PROGRAM_FOLDER).lnk" "$WINDIR\explorer.exe" "$INSTDIR\"
!endif
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_VIEW_STATISTICS).lnk" "$INSTDIR\Program\tools.exe" "--machine-profile-stats"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_TOOLS).lnk" "$INSTDIR\Program\tools.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_MANUAL).lnk" "$INSTDIR\Manual\index.html"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_UNINSTALL).lnk" "$INSTDIR\uninstall.exe"
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_WEB_SITE).lnk" "${PRODUCT_URL}"
!ifdef MAKE_UPDATES_SHORTCUT
	CreateShortCut "$SMPROGRAMS\${PRODUCT_ID}\$(TEXT_IO_CHECK_FOR_UPDATES).lnk" "${UPDATES_URL}"
!endif

	CreateShortCut "$INSTDIR\${PRODUCT_ID}.lnk" "$INSTDIR\Program\${PRODUCT_FAMILY}.exe"

	IfErrors do_error do_no_error
	do_error:
	MessageBox MB_OK|MB_ICONSTOP "$(TEXT_IO_FATAL_ERROR_INSTALL)"
	Quit
	do_no_error:

	Exec '$WINDIR\explorer.exe "$SMPROGRAMS\${PRODUCT_ID}\"'

SectionEnd

;--------------------------------
;Installer Functions

LangString TEXT_IO_TITLE		${LANG_ENGLISH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_ENGLISH} " "
LangString TEXT_IO_INSTALL		${LANG_ENGLISH} "Install"
LangString TEXT_IO_PLAY			${LANG_ENGLISH} "Play"
LangString TEXT_IO_REINSTALL	${LANG_ENGLISH} "Reinstall"
LangString TEXT_IO_TITLE		${LANG_FRENCH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_FRENCH} " "
LangString TEXT_IO_INSTALL		${LANG_FRENCH} "Installer"
LangString TEXT_IO_PLAY			${LANG_FRENCH} "Jouer"
LangString TEXT_IO_REINSTALL	${LANG_FRENCH} "Réinstaller"
LangString TEXT_IO_TITLE		${LANG_GERMAN} "${PRODUCT_ID_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_GERMAN} " "
LangString TEXT_IO_INSTALL		${LANG_GERMAN} "Installieren"
LangString TEXT_IO_PLAY			${LANG_GERMAN} "Spielen"
LangString TEXT_IO_REINSTALL	${LANG_GERMAN} "Noch mal installieren"
LangString TEXT_IO_TITLE		${LANG_SPANISH} "${PRODUCT_ID_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_SPANISH} " "
LangString TEXT_IO_INSTALL		${LANG_SPANISH} "Instale"
LangString TEXT_IO_PLAY			${LANG_SPANISH} "Juego"
LangString TEXT_IO_REINSTALL	${LANG_SPANISH} "Reinstálese"
LangString TEXT_IO_TITLE		${LANG_ITALIAN} "${PRODUCT_ID_VER}"
LangString TEXT_IO_SUBTITLE		${LANG_ITALIAN} " "
LangString TEXT_IO_INSTALL		${LANG_ITALIAN} "Installa"
LangString TEXT_IO_PLAY			${LANG_ITALIAN} "Gioca"
LangString TEXT_IO_REINSTALL	${LANG_ITALIAN} "Re-installa"

!ifdef SHOW_AUTORUN
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

LangString TEXT_IO_COULD_NOT_EXECUTE	${LANG_ENGLISH} "Could not execute $INSTDIR\Program\${PRODUCT_FAMILY}.exe"
LangString TEXT_IO_COULD_NOT_EXECUTE	${LANG_FRENCH} "N'a pas pu exécuter $INSTDIR\Program\${PRODUCT_FAMILY}.exe"
LangString TEXT_IO_COULD_NOT_EXECUTE	${LANG_GERMAN} "Konnte $INSTDIR\Program\${PRODUCT_FAMILY}.exe nicht ausführen"
LangString TEXT_IO_COULD_NOT_EXECUTE	${LANG_SPANISH} "$INSTDIR\Program\${PRODUCT_FAMILY}.exe No podía ejecutarse"
LangString TEXT_IO_COULD_NOT_EXECUTE	${LANG_ITALIAN} "Impossibile eseguire $INSTDIR\Program\${PRODUCT_FAMILY}.exe"

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
	Exec "$INSTDIR\Program\${PRODUCT_FAMILY}.exe"
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

LangString TEXT_IO_UNINSTALL_PREVIOUS				${LANG_ENGLISH} "The previous version of ${PRODUCT_ID} must be uninstalled before continuing.$\nDo you wish to continue?"
LangString TEXT_IO_DIRECTX_VISIT_MICROSOFT			${LANG_ENGLISH} "The latest version of DirectX (8.1 or higher) is strongly recommended.$\n Do you wish to visit Microsoft's site now?"
LangString TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY	${LANG_ENGLISH} "Uninstallation failed.  Install anyway?"
LangString TEXT_IO_INSTALL_DIRECTX					${LANG_ENGLISH} "The latest version of DirectX (8.1 or higher) is required.$\n Do you wish to install DirectX 8.1 now?"
LangString TEXT_IO_UNINSTALL_PREVIOUS				${LANG_FRENCH} "La version précédente de ${PRODUCT_ID} doit être déinstallée pour pouvoir continuer. Voulez-vous continuer ?"
LangString TEXT_IO_DIRECTX_VISIT_MICROSOFT			${LANG_FRENCH} "La dernière version de DirectX (8.1 ou supérieure) est hautement recommandée. Voulez-vous visiter le site de Microsoft maintenant ?"
LangString TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY	${LANG_FRENCH} "Echec de la déinstallation. Voulez-vous tout de même lancer l'installation ?"
LangString TEXT_IO_INSTALL_DIRECTX					${LANG_FRENCH} "La dernière version de DirectX (8.1 ou supérieure) est hautement recommandée. Désirez-vous l'installer maintenant ?"
LangString TEXT_IO_UNINSTALL_PREVIOUS				${LANG_GERMAN} "Die letzte Version von ${PRODUCT_ID} muss deinstalliert werden, bevor Sie weiter machen. Wollen Sie fortfahren?"
LangString TEXT_IO_DIRECTX_VISIT_MICROSOFT			${LANG_GERMAN} "Die letzte Version von DirectX (8.1 oder höher) ist dringend zu empfehlen.  Wollen Sie die Webseite von Microsoft jetzt besuchen?"
LangString TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY	${LANG_GERMAN} "Deinstallation fehlgeschlagen.  Trotzdem installieren?"
LangString TEXT_IO_INSTALL_DIRECTX					${LANG_GERMAN} "Die letzte Version von DirectX (8.1 oder höher) ist dringend zu empfehlen.  Wollen Sie es jetzt installieren?"
LangString TEXT_IO_UNINSTALL_PREVIOUS				${LANG_SPANISH} "La versión ${PRODUCT_ID} anterior del debe ser uninstalled antes de continuar.  Usted desea continuar?"
LangString TEXT_IO_DIRECTX_VISIT_MICROSOFT			${LANG_SPANISH} "La versión más última de DirectX (8,1 o más alto) se recomienda fuertemente.  Usted desea ahora visitar el sitio de Microsoft?"
LangString TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY	${LANG_SPANISH} "Uninstallation falló.  ¿Instale de todos modos?"
LangString TEXT_IO_INSTALL_DIRECTX					${LANG_SPANISH} "La versión más última de DirectX (8,1 o más alto) se recomienda fuertemente.  Usted desea ahora instalarla?"
LangString TEXT_IO_UNINSTALL_PREVIOUS				${LANG_ITALIAN} "Prima di continuare va installata la versione precendente di ${PRODUCT_ID}.  Vuoi continuare?"
LangString TEXT_IO_DIRECTX_VISIT_MICROSOFT			${LANG_ITALIAN} "Si consiglia vivamente di installare la versione più recente di TDirectX (8.1 o superiore).  Vuoi visitare il sito Microsoft adesso?"
LangString TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY	${LANG_ITALIAN} "Disinstallazione non riuscita.  Vuoi installare comunque?"
LangString TEXT_IO_INSTALL_DIRECTX					${LANG_ITALIAN} "Si consiglia vivamente di installare la versione più recente di TDirectX (8.1 o superiore).  Vuoi installarla adesso?"

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
	MessageBox MB_YESNO|MB_ICONINFORMATION "$(TEXT_IO_UNINSTALL_PREVIOUS)" IDYES do_uninstall_nsis
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

	MessageBox MB_YESNO|MB_DEFBUTTON2|MB_ICONINFORMATION "$(TEXT_IO_UNINSTALL_FAILED_INSTALL_ANYWAY)" IDYES old_nsis_not_installed
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
;Uninstaller Section

LangString TEXT_IO_FATAL_ERROR_UNINSTALL		${LANG_ENGLISH} "Fatal error during uninstall."
LangString TEXT_IO_FATAL_ERROR_UNINSTALL		${LANG_FRENCH} "Fatal error during uninstall."
LangString TEXT_IO_FATAL_ERROR_UNINSTALL		${LANG_GERMAN} "Fatal error during uninstall."
LangString TEXT_IO_FATAL_ERROR_UNINSTALL		${LANG_SPANISH} "Fatal error during uninstall."
LangString TEXT_IO_FATAL_ERROR_UNINSTALL		${LANG_ITALIAN} "Fatal error during uninstall."

Section "Uninstall"

	; add delete commands to delete whatever files/registry keys/etc you installed here.
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}"
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_ID}"

!ifdef INSTALL_TYPE_EXTERNAL_PCKS | INSTALL_TYPE_INTERNAL_PCKS
	RMDir /r "$INSTDIR\pcks"
!endif

!ifdef ASSOCIATE_SMZIP
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Classes\Applications\tools.exe\shell\open\command"
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
	RMDir /r "$INSTDIR\NoteSkins\para\default"
	RMDir "$INSTDIR\NoteSkins\para"
	RMDir "$INSTDIR\NoteSkins"

	RMDir /r "$INSTDIR\BackgroundEffects"

	RMDir /r "$INSTDIR\BackgroundTransitions"

	Delete "$INSTDIR\RandomMovies\instructions.txt"
	RMDir "$INSTDIR\RandomMovies"

	Delete "$INSTDIR\Songs\Instructions.txt"
	RMDir "$INSTDIR\Songs"	; will delete only if empty

	Delete "$INSTDIR\Themes\instructions.txt"
	RMDir /r "$INSTDIR\Themes\default"
	RMDir "$INSTDIR\Themes"

	Delete "$INSTDIR\Data\*.*"
	RMDir "$INSTDIR\Data"
!endif

	Delete "$INSTDIR\Program\${PRODUCT_FAMILY}.exe"
	Delete "$INSTDIR\Program\tools.exe"
	Delete "$INSTDIR\Program\mfc71.dll"
!ifdef ASSOCIATE_SMZIP
	Call un.RefreshShellIcons
!endif
	Delete "$INSTDIR\Program\${PRODUCT_FAMILY}.vdi"
	Delete "$INSTDIR\Program\msvcr71.dll"
	Delete "$INSTDIR\Program\msvcp71.dll"
	Delete "$INSTDIR\Program\jpeg.dll"
	Delete "$INSTDIR\Program\avcodec.dll"
	Delete "$INSTDIR\Program\avformat.dll"
	Delete "$INSTDIR\Program\dbghelp.dll"
	Delete "$INSTDIR\Program\zlib1.dll"
	RMDir "$INSTDIR\Program"

	Delete "$INSTDIR\Licenses.txt"
	RMDir /r "$INSTDIR\Manual"
	Delete "$INSTDIR\log.txt"
	Delete "$INSTDIR\info.txt"
	Delete "$INSTDIR\crashinfo.txt"
	Delete "$INSTDIR\${PRODUCT_ID}.lnk"

	RMDir "$INSTDIR"	; will delete only if empty

	SetShellVarContext current
	Delete "$DESKTOP\Play StepMania CVS.lnk"
	Delete "$DESKTOP\${PRODUCT_ID_VER}.lnk"
	; I'm being paranoid here:
	Delete "$SMPROGRAMS\${PRODUCT_ID}\*.*"
	RMDir "$SMPROGRAMS\${PRODUCT_ID}"

	Delete "$INSTDIR\Uninstall.exe"

	DeleteRegKey /ifempty HKEY_LOCAL_MACHINE "SOFTWARE\${PRODUCT_ID}"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

	!insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
