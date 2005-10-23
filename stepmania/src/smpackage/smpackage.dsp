# Microsoft Developer Studio Project File - Name="smpackage" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=smpackage - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "smpackage.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "smpackage.mak" CFG="smpackage - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "smpackage - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "smpackage - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smpackage - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Program/"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 ZipArchive\Release\ZipArchive.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "smpackage - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Program/"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"../../Program/smpackage-debug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "smpackage - Win32 Release"
# Name "smpackage - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ChangeGameSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\ChangeGameSettings.h
# End Source File
# Begin Source File

SOURCE=.\EditInsallations.cpp
# End Source File
# Begin Source File

SOURCE=.\EditInsallations.h
# End Source File
# Begin Source File

SOURCE=.\EditMetricsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\EditMetricsDlg.h
# End Source File
# Begin Source File

SOURCE=.\EnterComment.cpp
# End Source File
# Begin Source File

SOURCE=.\EnterComment.h
# End Source File
# Begin Source File

SOURCE=.\EnterName.cpp
# End Source File
# Begin Source File

SOURCE=.\EnterName.h
# End Source File
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\MainMenuDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainMenuDlg.h
# End Source File
# Begin Source File

SOURCE=.\onvertThemeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\onvertThemeDlg.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil.h
# End Source File
# Begin Source File

SOURCE=.\Registry.cpp
# End Source File
# Begin Source File

SOURCE=.\Registry.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\ShowComment.cpp
# End Source File
# Begin Source File

SOURCE=.\ShowComment.h
# End Source File
# Begin Source File

SOURCE=.\smpackage.cpp
# End Source File
# Begin Source File

SOURCE=.\smpackage.h
# End Source File
# Begin Source File

SOURCE=.\SmpackageExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SmpackageExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\SMPackageInstallDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SMPackageInstallDlg.h
# End Source File
# Begin Source File

SOURCE=.\SMPackageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\smpackageUtil.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlEx.cpp
# End Source File
# Begin Source File

SOURCE=.\TreeCtrlEx.h
# End Source File
# Begin Source File

SOURCE=.\UninstallOld.cpp
# End Source File
# Begin Source File

SOURCE=.\UninstallOld.h
# End Source File
# End Group
# Begin Group "Resources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\converttheme.bmp
# End Source File
# Begin Source File

SOURCE=.\editmetrics.bmp
# End Source File
# Begin Source File

SOURCE=.\install.bmp
# End Source File
# Begin Source File

SOURCE=.\manage.bmp
# End Source File
# Begin Source File

SOURCE=.\menu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\smpackage.ico
# End Source File
# Begin Source File

SOURCE=.\smpackage.ICO
# End Source File
# Begin Source File

SOURCE=.\smpackage.rc
# End Source File
# Begin Source File

SOURCE=.\res\smpackage.rc2
# End Source File
# Begin Source File

SOURCE=.\res\StepMania.ICO
# End Source File
# End Group
# Begin Source File

SOURCE=..\zlib\zdll.lib
# End Source File
# End Target
# End Project
