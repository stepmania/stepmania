# Microsoft Developer Studio Project File - Name="ul" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ul - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ul.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ul.mak" CFG="ul - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ul - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ul - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ul"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ul - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy release\*.lib ..\..\*.*	copy ul.h ..\..\ul.h	copy ulRTTI.h ..\..\ulRTTI.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ul - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\ul_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy debug\*.lib ..\..\*.*	copy ul.h ..\..\ul.h	copy ulRTTI.h ..\..\ulRTTI.h
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ul - Win32 Release"
# Name "ul - Win32 Debug"
# Begin Source File

SOURCE=.\ul.cxx
# End Source File
# Begin Source File

SOURCE=.\ul.h
# End Source File
# Begin Source File

SOURCE=.\ulClock.cxx
# End Source File
# Begin Source File

SOURCE=.\ulError.cxx
# End Source File
# Begin Source File

SOURCE=.\ulLinkedList.cxx
# End Source File
# Begin Source File

SOURCE=.\ulList.cxx
# End Source File
# Begin Source File

SOURCE=.\ulLocal.h
# End Source File
# Begin Source File

SOURCE=.\ulRTTI.cxx
# End Source File
# Begin Source File

SOURCE=.\ulRTTI.h
# End Source File
# End Target
# End Project
