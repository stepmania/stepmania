# Microsoft Developer Studio Project File - Name="sl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sl - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sl.mak" CFG="sl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sl - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sl - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "sl"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sl - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /Zi /O2 /I "..\util" /I ".." /D "NDEBUG" /D "_LIB" /D "NEEDNAMESPACESTD" /D "HAVE_CONFIG_H" /D "HAVE_WINDOWS_H" /D "WIN32" /D "_MBCS" /FD /c
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
PostBuild_Cmds=copy release\*.lib ..\..\*.*	copy sl.h ..\..\sl.h  	copy slPortability.h ..\..\slPortability.h 	copy sm.h ..\..\sm.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "sl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sl___Win32_Debug"
# PROP BASE Intermediate_Dir "sl___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GX /Zi /Od /I "..\util" /I ".." /D "_DEBUG" /D "_LIB" /D "NEEDNAMESPACESTD" /D "HAVE_CONFIG_H" /D "HAVE_WINDOWS_H" /D "WIN32" /D "_MBCS" /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\sl_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy debug\*.lib ..\..\*.*	copy sl.h ..\..\sl.h  	copy slPortability.h ..\..\slPortability.h 	copy sm.h ..\..\sm.h
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "sl - Win32 Release"
# Name "sl - Win32 Debug"
# Begin Source File

SOURCE=.\sl.h
# End Source File
# Begin Source File

SOURCE=.\slDSP.cxx
# End Source File
# Begin Source File

SOURCE=.\slEnvelope.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODdacio.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODfile.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODfile.h
# End Source File
# Begin Source File

SOURCE=.\slMODinst.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODnote.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODPlayer.cxx
# End Source File
# Begin Source File

SOURCE=.\slMODPrivate.h
# End Source File
# Begin Source File

SOURCE=.\slPlayer.cxx
# End Source File
# Begin Source File

SOURCE=.\slPortability.h
# End Source File
# Begin Source File

SOURCE=.\slSample.cxx
# End Source File
# Begin Source File

SOURCE=.\slSamplePlayer.cxx
# End Source File
# Begin Source File

SOURCE=.\slScheduler.cxx
# End Source File
# Begin Source File

SOURCE=.\sm.h
# End Source File
# Begin Source File

SOURCE=.\smMixer.cxx
# End Source File
# End Target
# End Project
