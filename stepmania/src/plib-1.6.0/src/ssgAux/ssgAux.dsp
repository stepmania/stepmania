# Microsoft Developer Studio Project File - Name="ssgAux" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ssgAux - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ssgAux.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ssgAux.mak" CFG="ssgAux - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ssgAux - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ssgAux - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "ssgAux"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssgAux - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /Zi /O2 /I "..\sg" /I "..\util" /I "..\ssg" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy release\*.lib ..\..\*.*	copy ssgAux.h ..\..\ssgAux.h	copy ssgaFire.h ..\..\ssgaFire.h	copy ssgaLensFlare.h ..\..\ssgaLensFlare.h	copy ssgaParticleSystem.h ..\..\ssgaParticleSystem.h	copy ssgaShapes.h ..\..\ssgaShapes.h	copy ssgaWaveSystem.h ..\..\ssgaWaveSystem.h
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ssgAux - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I "..\sg" /I "..\util" /I "..\ssg" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\ssgAux_d.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy Library to plib directory
PostBuild_Cmds=copy debug\*.lib ..\..\*.*	copy ssgAux.h ..\..\ssgAux.h	copy ssgaFire.h ..\..\ssgaFire.h	copy ssgaLensFlare.h ..\..\ssgaLensFlare.h	copy ssgaParticleSystem.h ..\..\ssgaParticleSystem.h	copy ssgaShapes.h ..\..\ssgaShapes.h	copy ssgaWaveSystem.h ..\..\ssgaWaveSystem.h
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ssgAux - Win32 Release"
# Name "ssgAux - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ssgaFire.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaLensFlare.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaLensFlareTexture.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaParticleSystem.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaPatch.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaShapes.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaTeapot.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgaWaveSystem.cxx
# End Source File
# Begin Source File

SOURCE=.\ssgAux.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ssgaFire.h
# End Source File
# Begin Source File

SOURCE=.\ssgaLensFlare.h
# End Source File
# Begin Source File

SOURCE=.\ssgaParticleSystem.h
# End Source File
# Begin Source File

SOURCE=.\ssgaShapes.h
# End Source File
# Begin Source File

SOURCE=.\ssgaWaveSystem.h
# End Source File
# Begin Source File

SOURCE=.\ssgAux.h
# End Source File
# End Group
# End Target
# End Project
