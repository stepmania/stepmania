# Microsoft Developer Studio Project File - Name="ZipArchive" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ZipArchive - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ZipArchive_DLL.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ZipArchive_DLL.mak" CFG="ZipArchive - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ZipArchive - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZipArchive - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ZipArchive - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/ZipArchive", LDAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ZipArchive - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ZipArchive___Win32_Release"
# PROP BASE Intermediate_Dir "ZipArchive___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZipArchive_DLL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "./zlib" /D "NDEBUG" /D "_USRDLL" /D "ZLIB_DLL" /D "ZIP_BUILD_DLL" /D "_WINDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "ZIP_HAS_DLL" /D "_AFXDLL" /D "ZIP_ARCHIVE_MFC_PROJ" /Fp"Release/ZipArchive.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 User32.lib /nologo /dll /machine:I386 /out:"Release/ZipArchive.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ZipArchive - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ZipArchive___Win32_Debug"
# PROP BASE Intermediate_Dir "ZipArchive___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZipArchive_DLL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "./zlib" /D "_DEBUG" /D "_USRDLL" /D "ZLIB_DLL" /D "ZIP_BUILD_DLL" /D "_WINDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "ZIP_HAS_DLL" /D "_AFXDLL" /D "ZIP_ARCHIVE_MFC_PROJ" /Fp"Debug/ZipArchive.pch" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 User32.lib /nologo /dll /debug /machine:I386 /out:"Debug/ZipArchive.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ZipArchive - Win32 Unicode Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ZipArchive___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "ZipArchive___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unicode_Release"
# PROP Intermediate_Dir "Unicode_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /I "./zlib" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_DLL" /D "ZIP_HAS_DLL" /D "ZIP_BUILD_DLL" /D "_WINDLL" /D "_AFXDLL" /Fp"Release/ZipArchive.pch" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "./zlib" /D "NDEBUG" /D "_UNICODE" /D "_USRDLL" /D "ZLIB_DLL" /D "ZIP_BUILD_DLL" /D "_WINDLL" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "ZIP_HAS_DLL" /D "_AFXDLL" /D "ZIP_ARCHIVE_MFC_PROJ" /Fp"Release/ZipArchive.pch" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 User32.lib /nologo /dll /machine:I386 /out:"Release/ZipArchive.dll"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 User32.lib /nologo /dll /machine:I386 /out:"Unicode_Release/ZipArchive.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ZipArchive - Win32 Release"
# Name "ZipArchive - Win32 Debug"
# Name "ZipArchive - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.rc
# End Source File
# Begin Source File

SOURCE=.\ZipAutoBuffer.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipCentralDir.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipCompatibility.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipException.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipFileHeader.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipMemFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipPathComponent.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipPlatform.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipPlatformComm.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.cpp
# End Source File
# Begin Source File

SOURCE=.\ZipString.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\ZipAbstractFile.h
# End Source File
# Begin Source File

SOURCE=.\ZipArchive.h
# End Source File
# Begin Source File

SOURCE=.\ZipAutoBuffer.h
# End Source File
# Begin Source File

SOURCE=.\ZipBaseException.h
# End Source File
# Begin Source File

SOURCE=.\ZipCentralDir.h
# End Source File
# Begin Source File

SOURCE=.\ZipCollections.h
# End Source File
# Begin Source File

SOURCE=.\ZipCompatibility.h
# End Source File
# Begin Source File

SOURCE=.\ZipException.h
# End Source File
# Begin Source File

SOURCE=.\ZipExport.h
# End Source File
# Begin Source File

SOURCE=.\ZipFile.h
# End Source File
# Begin Source File

SOURCE=.\ZipFileHeader.h
# End Source File
# Begin Source File

SOURCE=.\ZipFileMapping.h
# End Source File
# Begin Source File

SOURCE=.\ZipMemFile.h
# End Source File
# Begin Source File

SOURCE=.\ZipPathComponent.h
# End Source File
# Begin Source File

SOURCE=.\ZipPlatform.h
# End Source File
# Begin Source File

SOURCE=.\ZipStorage.h
# End Source File
# Begin Source File

SOURCE=.\ZipString.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
