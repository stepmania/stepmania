# Microsoft Developer Studio Project File - Name="BaseClasses" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=BaseClasses - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "baseclasses.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "baseclasses.mak" CFG="BaseClasses - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BaseClasses - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "BaseClasses - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "BaseClasses - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /Gi /GX /O2 /I "." /I "..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /D WINVER=0x400 /Yu"streams.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 ..\..\..\..\lib\strmiids.lib /nologo /out:"Release\STRMBASE.lib" /nodefaultlib

!ELSEIF  "$(CFG)" == "BaseClasses - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Gz /MTd /W3 /Gm /Gi /GX /Zi /Od /I "." /I "..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "_WIN32_DCOM" /D "DEBUG" /D WINVER=0x400 /Yu"streams.h" /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 ..\..\..\..\lib\strmiids.lib /nologo /out:"debug\strmbasd.lib" /nodefaultlib

!ENDIF 

# Begin Target

# Name "BaseClasses - Win32 Release"
# Name "BaseClasses - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\amextra.cpp
# End Source File
# Begin Source File

SOURCE=.\amfilter.cpp
# End Source File
# Begin Source File

SOURCE=.\amvideo.cpp
# End Source File
# Begin Source File

SOURCE=.\combase.cpp
# End Source File
# Begin Source File

SOURCE=.\cprop.cpp
# End Source File
# Begin Source File

SOURCE=.\ctlutil.cpp
# End Source File
# Begin Source File

SOURCE=.\ddmm.cpp
# End Source File
# Begin Source File

SOURCE=.\dllentry.cpp
# ADD CPP /Yc"streams.h"
# End Source File
# Begin Source File

SOURCE=.\dllsetup.cpp
# End Source File
# Begin Source File

SOURCE=.\mtype.cpp
# End Source File
# Begin Source File

SOURCE=.\outputq.cpp
# End Source File
# Begin Source File

SOURCE=.\pstream.cpp
# End Source File
# Begin Source File

SOURCE=.\pullpin.cpp
# End Source File
# Begin Source File

SOURCE=.\refclock.cpp
# End Source File
# Begin Source File

SOURCE=.\renbase.cpp
# End Source File
# Begin Source File

SOURCE=.\schedule.cpp
# End Source File
# Begin Source File

SOURCE=.\seekpt.cpp
# End Source File
# Begin Source File

SOURCE=.\source.cpp
# End Source File
# Begin Source File

SOURCE=.\strmctl.cpp
# End Source File
# Begin Source File

SOURCE=.\sysclock.cpp
# End Source File
# Begin Source File

SOURCE=.\transfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\transip.cpp
# End Source File
# Begin Source File

SOURCE=.\videoctl.cpp
# End Source File
# Begin Source File

SOURCE=.\vtrans.cpp
# End Source File
# Begin Source File

SOURCE=.\winctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\winutil.cpp
# End Source File
# Begin Source File

SOURCE=.\wxdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\wxlist.cpp
# End Source File
# Begin Source File

SOURCE=.\wxutil.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\activex.rcv
# End Source File
# Begin Source File

SOURCE=.\activex.ver
# End Source File
# Begin Source File

SOURCE=.\dllsetup.h
# End Source File
# Begin Source File

SOURCE=.\dsschedule.h
# End Source File
# Begin Source File

SOURCE=.\fourcc.h
# End Source File
# Begin Source File

SOURCE=.\measure.h
# End Source File
# Begin Source File

SOURCE=.\msgthrd.h
# End Source File
# Begin Source File

SOURCE=.\mtype.h
# End Source File
# Begin Source File

SOURCE=.\outputq.h
# End Source File
# Begin Source File

SOURCE=.\pstream.h
# End Source File
# Begin Source File

SOURCE=.\pullpin.h
# End Source File
# Begin Source File

SOURCE=.\refclock.h
# End Source File
# Begin Source File

SOURCE=.\reftime.h
# End Source File
# Begin Source File

SOURCE=.\renbase.h
# End Source File
# Begin Source File

SOURCE=.\seekpt.h
# End Source File
# Begin Source File

SOURCE=.\source.h
# End Source File
# Begin Source File

SOURCE=.\streams.h
# End Source File
# Begin Source File

SOURCE=.\strmctl.h
# End Source File
# Begin Source File

SOURCE=.\sysclock.h
# End Source File
# Begin Source File

SOURCE=.\transfrm.h
# End Source File
# Begin Source File

SOURCE=.\transip.h
# End Source File
# Begin Source File

SOURCE=.\videoctl.h
# End Source File
# Begin Source File

SOURCE=.\vtrans.h
# End Source File
# Begin Source File

SOURCE=.\winctrl.h
# End Source File
# Begin Source File

SOURCE=.\winutil.h
# End Source File
# Begin Source File

SOURCE=.\wxdebug.h
# End Source File
# Begin Source File

SOURCE=.\wxlist.h
# End Source File
# Begin Source File

SOURCE=.\wxutil.h
# End Source File
# End Group
# End Target
# End Project
