# Microsoft Developer Studio Project File - Name="cryptlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=cryptlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cryptlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cryptlib.mak" CFG="cryptlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cryptlib - Win32 FIPS 140 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 FIPS 140 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cryptlib - Win32 FIPS 140 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cryptlib___Win32_FIPS_140_Release"
# PROP BASE Intermediate_Dir "cryptlib___Win32_FIPS_140_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "FIPS_140_Release"
# PROP Intermediate_Dir "FIPS_140_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /Gz /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /Yu"pch.h" /FD /c
# ADD CPP /nologo /G5 /Gz /MT /W3 /GX /Zi /O2 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2=1 /Yu"pch.h" /Fd"FIPS_140_Release/cryptopp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"FIPS_140_Release\cryptopp.lib"

!ELSEIF  "$(CFG)" == "cryptlib - Win32 FIPS 140 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cryptlib___Win32_FIPS_140_Debug"
# PROP BASE Intermediate_Dir "cryptlib___Win32_FIPS_140_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "FIPS_140_Debug"
# PROP Intermediate_Dir "FIPS_140_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /Yu"pch.h" /FD /c
# ADD CPP /nologo /G5 /Gz /MTd /W3 /GX /ZI /Od /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2=1 /Yu"pch.h" /Fd"FIPS_140_Debug/cryptopp" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"FIPS_140_Debug\cryptopp.lib"

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "cryptlib"
# PROP BASE Intermediate_Dir "cryptlib"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /YX"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "cryptli0"
# PROP BASE Intermediate_Dir "cryptli0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /YX"pch.h" /FD /c
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "cryptlib - Win32 FIPS 140 Release"
# Name "cryptlib - Win32 FIPS 140 Debug"
# Name "cryptlib - Win32 Release"
# Name "cryptlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\algebra.cpp
# End Source File
# Begin Source File

SOURCE=.\algebra.h
# End Source File
# Begin Source File

SOURCE=.\algparam.cpp
# End Source File
# Begin Source File

SOURCE=.\algparam.h
# End Source File
# Begin Source File

SOURCE=.\argnames.h
# End Source File
# Begin Source File

SOURCE=.\asn.cpp
# End Source File
# Begin Source File

SOURCE=.\asn.h
# End Source File
# Begin Source File

SOURCE=.\cryptlib.cpp
# End Source File
# Begin Source File

SOURCE=.\cryptlib.h
# End Source File
# Begin Source File

SOURCE=.\files.cpp
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\filters.cpp
# End Source File
# Begin Source File

SOURCE=.\filters.h
# End Source File
# Begin Source File

SOURCE=.\fltrimpl.h
# End Source File
# Begin Source File

SOURCE=.\integer.cpp
# End Source File
# Begin Source File

SOURCE=.\integer.h
# End Source File
# Begin Source File

SOURCE=.\iterhash.cpp
# End Source File
# Begin Source File

SOURCE=.\iterhash.h
# End Source File
# Begin Source File

SOURCE=.\mdc.h
# End Source File
# Begin Source File

SOURCE=.\misc.cpp
# End Source File
# Begin Source File

SOURCE=.\misc.h
# End Source File
# Begin Source File

SOURCE=.\modarith.h
# End Source File
# Begin Source File

SOURCE=.\modes.cpp
# End Source File
# Begin Source File

SOURCE=.\modes.h
# End Source File
# Begin Source File

SOURCE=.\mqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\mqueue.h
# End Source File
# Begin Source File

SOURCE=.\nbtheory.cpp
# End Source File
# Begin Source File

SOURCE=.\nbtheory.h
# End Source File
# Begin Source File

SOURCE=.\oaep.cpp
# End Source File
# Begin Source File

SOURCE=.\oaep.h
# End Source File
# Begin Source File

SOURCE=.\oids.h
# End Source File
# Begin Source File

SOURCE=.\osrng.cpp
# End Source File
# Begin Source File

SOURCE=.\osrng.h
# End Source File
# Begin Source File

SOURCE=.\pch.h
# End Source File
# Begin Source File

SOURCE=.\pkcspad.cpp
# End Source File
# Begin Source File

SOURCE=.\pkcspad.h
# End Source File
# Begin Source File

SOURCE=.\pubkey.cpp
# End Source File
# Begin Source File

SOURCE=.\pubkey.h
# End Source File
# Begin Source File

SOURCE=.\queue.cpp
# End Source File
# Begin Source File

SOURCE=.\queue.h
# End Source File
# Begin Source File

SOURCE=.\randpool.cpp
# End Source File
# Begin Source File

SOURCE=.\randpool.h
# End Source File
# Begin Source File

SOURCE=.\rng.h
# End Source File
# Begin Source File

SOURCE=.\rsa.cpp
# End Source File
# Begin Source File

SOURCE=.\rsa.h
# End Source File
# Begin Source File

SOURCE=.\secblock.h
# End Source File
# Begin Source File

SOURCE=.\seckey.h
# End Source File
# Begin Source File

SOURCE=.\sha.cpp
# End Source File
# Begin Source File

SOURCE=.\sha.h
# End Source File
# Begin Source File

SOURCE=.\simple.h
# End Source File
# Begin Source File

SOURCE=.\smartptr.h
# End Source File
# Begin Source File

SOURCE=.\strciphr.cpp
# End Source File
# Begin Source File

SOURCE=.\strciphr.h
# End Source File
# Begin Source File

SOURCE=.\words.h
# End Source File
# End Group
# Begin Group "Miscellaneous"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Doxyfile
# End Source File
# Begin Source File

SOURCE=.\GNUmakefile
# End Source File
# Begin Source File

SOURCE=.\license.txt
# End Source File
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Group
# End Target
# End Project
