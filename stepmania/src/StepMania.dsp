# Microsoft Developer Studio Project File - Name="StepMania" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101
# TARGTYPE "Xbox Application" 0x0b01

CFG=StepMania - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak" CFG="StepMania - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StepMania - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "StepMania - Xbox Debug" (based on "Xbox Application")
!MESSAGE "StepMania - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "StepMania - Xbox Release" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StepMania___Win32_Debug_OGL"
# PROP BASE Intermediate_Dir "StepMania___Win32_Debug_OGL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug6"
# PROP Intermediate_Dir "../Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /Fr /YX"global.h" /FD /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "SDL-1.2.6\include" /I "SDL_image-1.2" /I "SDL_net-1.2.5\include" /I "vorbis" /I "libjpeg" /I "lua-5.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /FR /YX"global.h" /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 $(intdir)\verstub.obj kernel32.lib shell32.lib user32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../debug6/StepMania-debug.pdb" /map /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"../StepMania-debug.exe"
# SUBTRACT BASE LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /map /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:"../Program/StepMania-debug.exe"
# SUBTRACT LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# Begin Special Build Tool
IntDir=.\../Debug6
TargetDir=\stepmania\stepmania\Program
TargetName=StepMania-debug
SOURCE="$(InputPath)"
PreLink_Cmds=disasm\verinc                                                                                    	cl                                                                                    /Zl                                                                                    /nologo                                                                                    /c                                                                                    verstub.cpp                                                                                    /Fo$(IntDir)\ 
PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug_Xbox"
# PROP Intermediate_Dir "../Debug_Xbox"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /stack:0x10000 /debug /out:"../default.xbe"
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /TMP
# ADD LINK32 $(intdir)\verstub.obj xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /map /debug /machine:I386 /nodefaultlib:"libcd" /nodefaultlib:"libcmt" /out:"../StepManiaXbox-debug.exe" /subsystem:xbox /fixed:no /TMP
# SUBTRACT LINK32 /incremental:no
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /I "SDL_sound-1.0.0" /I "vorbis" /D "WIN32" /D "_DEBUG" /D "_XBOX" /D "OGG_ONLY" /D "DEBUG" /FR /YX"global.h" /FD /G6 /Ztmp /c
# Begin Special Build Tool
PreLink_Cmds=disasm\verinc                                                                                                                                                                                                                                                                    	cl                                                                                                                                                                                                                                                                     /Zl                                                                                                                                                                                                                                                                     /nologo                                                                                                                                                                                                                                                                     /c  PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi ia32.vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "StepMania___Win32_Release_OGL"
# PROP BASE Intermediate_Dir "StepMania___Win32_Release_OGL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release6"
# PROP Intermediate_Dir "../Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O2 /Ob2 /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "SDL-1.2.6\include" /I "SDL_image-1.2" /I "SDL_net-1.2.5\include" /I "vorbis" /I "libjpeg" /I "lua-5.0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"../StepMania-StackTrace.bsc"
# ADD BSC32 /nologo /o"../StepMania-StackTrace.bsc"
LINK32=link.exe
# ADD BASE LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../release6/StepMania.pdb" /map /debug /machine:I386
# SUBTRACT BASE LINK32 /verbose /pdb:none
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /map /machine:I386 /out:"../Program/StepMania.exe"
# SUBTRACT LINK32 /verbose /pdb:none /debug
# Begin Special Build Tool
IntDir=.\../Release6
TargetDir=\stepmania\stepmania\Program
TargetName=StepMania
SOURCE="$(InputPath)"
PreLink_Cmds=disasm\verinc                                                                                     	cl                                              /Zl                                              /nologo                                              /c                                              verstub.cpp                                              /Fo$(IntDir)\ 
PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "StepMania___Xbox_Release"
# PROP BASE Intermediate_Dir "StepMania___Xbox_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Release_Xbox"
# PROP Intermediate_Dir "../Release_Xbox"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testid:"123456" /stack:0x10000 /debug /out:"../default.xbe"
LINK32=link.exe
# ADD BASE LINK32 $(intdir)\verstub.obj xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /pdb:"../debug6/StepMania-debug.pdb" /map /debug /machine:IX86 /nodefaultlib:"libcmtd.lib" /out:"../StepMania-debug.exe"
# SUBTRACT BASE LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# ADD LINK32 $(intdir)\verstub.obj xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib libcmt.lib /nologo /incremental:no /map /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcmtd" /out:"../StepManiaXbox.exe" /subsystem:xbox /fixed:no /TMP /OPT:REF
# SUBTRACT LINK32 /pdb:none /debug
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
CPP=cl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "_XBOX" /D "_DEBUG" /Fr /YX"global.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /I "SDL_sound-1.0.0" /I "vorbis" /D "WIN32" /D "NDEBUG" /D "_XBOX" /FR /YX /FD /c
# Begin Special Build Tool
PreLink_Cmds=disasm\verinc                                                                                                                                                                                                                                                                     	cl                                                                                                                                                                                                                                                                      /Zl                                                                                                                                                                                                                                                                      /nologo                                                                                                                                                                                                                                                                      /c  PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi ia32.vdi
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "StepMania - Win32 Debug"
# Name "StepMania - Xbox Debug"
# Name "StepMania - Win32 Release"
# Name "StepMania - Xbox Release"
# Begin Group "Rage"

# PROP Default_Filter ""
# Begin Group "Helpers"

# PROP Default_Filter ""
# Begin Group "pcre"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pcre\get.c

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre\internal.h
# End Source File
# Begin Source File

SOURCE=.\pcre\maketables.c

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre\pcre.c

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pcre\pcre.h
# End Source File
# Begin Source File

SOURCE=.\pcre\study.c

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\SDL_dither.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SDL_dither.h
# End Source File
# Begin Source File

SOURCE=.\SDL_rotozoom.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SDL_rotozoom.h
# End Source File
# Begin Source File

SOURCE=.\SDL_SaveJPEG.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SDL_SaveJPEG.h
# End Source File
# Begin Source File

SOURCE=.\SDL_utils.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SDL_utils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\RageBitmapTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /YX
# ADD CPP /YX

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageBitmapTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_D3D.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageDisplay_D3D.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_Null.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageDisplay_Null.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageDisplay_OGL.h
# End Source File
# Begin Source File

SOURCE=.\RageException.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageException.h
# End Source File
# Begin Source File

SOURCE=.\RageFile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFile.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriver.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFileDriver.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirect.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirect.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirectHelpers.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFileDriverDirectHelpers.h
# End Source File
# Begin Source File

SOURCE=.\RageFileDriverZip.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFileDriverZip.h
# End Source File
# Begin Source File

SOURCE=.\RageFileManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageFileManager.h
# End Source File
# Begin Source File

SOURCE=.\RageInput.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageInput.h
# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\RageLog.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageLog.h
# End Source File
# Begin Source File

SOURCE=.\RageMath.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageMath.h
# End Source File
# Begin Source File

SOURCE=.\RageModelGeometry.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageModelGeometry.h
# End Source File
# Begin Source File

SOURCE=.\RageSound.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSound.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_FileReader.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_FileReader.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_MP3.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_MP3.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Fast.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Fast.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Good.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample_Good.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Vorbisfile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Vorbisfile.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_WAV.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_WAV.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.h
# End Source File
# Begin Source File

SOURCE=.\RageSounds.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageSounds.h
# End Source File
# Begin Source File

SOURCE=.\RageTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageTextureID.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageTextureID.h
# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.h
# End Source File
# Begin Source File

SOURCE=.\RageThreads.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageThreads.h
# End Source File
# Begin Source File

SOURCE=.\RageTimer.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageTimer.h
# End Source File
# Begin Source File

SOURCE=.\RageTypes.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageUtil.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_CharConversions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageUtil_CharConversions.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil_FileDB.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageUtil_FileDB.h
# End Source File
# End Group
# Begin Group "Data Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ArrowBackdrop.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ArrowBackdrop.h
# End Source File
# Begin Source File

SOURCE=.\Attack.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Attack.h
# End Source File
# Begin Source File

SOURCE=.\BannerCache.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BannerCache.h
# End Source File
# Begin Source File

SOURCE=.\Character.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\CodeDetector.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CodeDetector.h
# End Source File
# Begin Source File

SOURCE=.\Course.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Course.h
# End Source File
# Begin Source File

SOURCE=.\Font.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.h
# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.h
# End Source File
# Begin Source File

SOURCE=.\ForeachEnum.h
# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.h
# End Source File
# Begin Source File

SOURCE=.\GameDef.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameDef.h
# End Source File
# Begin Source File

SOURCE=.\GameInput.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameInput.h
# End Source File
# Begin Source File

SOURCE=.\GameTypes.h
# End Source File
# Begin Source File

SOURCE=.\Grade.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Grade.h
# End Source File
# Begin Source File

SOURCE=.\HighScore.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HighScore.h
# End Source File
# Begin Source File

SOURCE=.\Inventory.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Inventory.h
# End Source File
# Begin Source File

SOURCE=.\LuaFunctions.h
# End Source File
# Begin Source File

SOURCE=.\LuaHelpers.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LuaHelpers.h
# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.h
# End Source File
# Begin Source File

SOURCE=.\ModeChoice.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModeChoice.h
# End Source File
# Begin Source File

SOURCE=.\NoteData.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteData.h
# End Source File
# Begin Source File

SOURCE=.\NoteDataUtil.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteDataUtil.h
# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.h
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoader.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesLoader.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.h
# End Source File
# Begin Source File

SOURCE=.\NoteTypes.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteTypes.h
# End Source File
# Begin Source File

SOURCE=.\PlayerAI.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PlayerAI.h
# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.h
# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\Profile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Profile.h
# End Source File
# Begin Source File

SOURCE=.\ProfileHtml.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ProfileHtml.h
# End Source File
# Begin Source File

SOURCE=.\RandomSample.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RandomSample.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeper.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperRave.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperRave.h
# End Source File
# Begin Source File

SOURCE=.\Song.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\song.h
# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.h
# End Source File
# Begin Source File

SOURCE=.\SongOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SongOptions.h
# End Source File
# Begin Source File

SOURCE=.\StageStats.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StageStats.h
# End Source File
# Begin Source File

SOURCE=.\Steps.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Steps.h
# End Source File
# Begin Source File

SOURCE=.\StyleDef.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StyleDef.h
# End Source File
# Begin Source File

SOURCE=.\StyleInput.h
# End Source File
# Begin Source File

SOURCE=.\TimeConstants.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TimeConstants.h
# End Source File
# Begin Source File

SOURCE=.\TimingData.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TimingData.h
# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.h
# End Source File
# End Group
# Begin Group "File Types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IniFile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\MsdFile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MsdFile.h
# End Source File
# Begin Source File

SOURCE=.\XmlFile.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\XmlFile.h
# End Source File
# End Group
# Begin Group "StepMania"

# PROP Default_Filter ""
# Begin Group "arch"

# PROP Default_Filter ""
# Begin Group "LoadingWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_SDL.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_SDL.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Generic_Software.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Generic_Software.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.h
# End Source File
# End Group
# Begin Group "ArchHooks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_none.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.h
# End Source File
# End Group
# Begin Group "InputHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInput.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInput.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInputHelper.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_DirectInputHelper.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_SDL.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_SDL.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Para.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Para.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.h
# End Source File
# End Group
# Begin Group "MovieTexture"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_FFMpeg.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_FFMpeg.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_Null.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_Null.h
# End Source File
# End Group
# Begin Group "LowLevelWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow.h

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_SDL.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_SDL.h

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Lights"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_SystemMessage.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_SystemMessage.h
# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Win32Parallel.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\Lights\LightsDriver_Win32Parallel.h
# End Source File
# End Group
# Begin Group "MemoryCard"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded.h
# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded_Windows.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded_Windows.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\arch\arch.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\arch\arch.h
# End Source File
# Begin Source File

SOURCE=.\arch\arch_default.h
# End Source File
# Begin Source File

SOURCE=.\arch\arch_Win32.h
# End Source File
# Begin Source File

SOURCE=.\arch\arch_xbox.h
# End Source File
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\arch_setup.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\arch_setup.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Crash.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Crash.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RestartProgram.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\RestartProgram.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.ICO
# End Source File
# Begin Source File

SOURCE=.\StepMania.RC

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\VideoDriverInfo.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\VideoDriverInfo.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\custom_launch_params.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\custom_launch_params.h
# End Source File
# Begin Source File

SOURCE=.\global.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /Yc"global.h"
# ADD CPP /Yc"global.h"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDimensions.h
# End Source File
# Begin Source File

SOURCE=".\SDL-1.2.5\src\main\xbox\SDL_main.c"

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdString.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StepMania.h
# End Source File
# End Group
# Begin Group "Actors"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorCollision.h
# End Source File
# Begin Source File

SOURCE=.\ActorCommands.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ActorCommands.h
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ActorFrame.h
# End Source File
# Begin Source File

SOURCE=.\ActorScroller.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ActorScroller.h
# End Source File
# Begin Source File

SOURCE=.\ActorUtil.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ActorUtil.h
# End Source File
# Begin Source File

SOURCE=.\BitmapText.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BitmapText.h
# End Source File
# Begin Source File

SOURCE=.\Model.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\ModelManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModelManager.h
# End Source File
# Begin Source File

SOURCE=.\ModelTypes.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModelTypes.h
# End Source File
# Begin Source File

SOURCE=.\Quad.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay & Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Banner.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Banner.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimation.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BGAnimation.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.h
# End Source File
# Begin Source File

SOURCE=.\ConditionalBGA.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ConditionalBGA.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.h
# End Source File
# Begin Source File

SOURCE=.\FadingBanner.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FadingBanner.h
# End Source File
# Begin Source File

SOURCE=.\MeterDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MeterDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Transition.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Transition.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActiveAttackList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ActiveAttackList.h
# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.h
# End Source File
# Begin Source File

SOURCE=.\AttackDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AttackDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Background.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Background.h
# End Source File
# Begin Source File

SOURCE=.\BeginnerHelper.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BeginnerHelper.h
# End Source File
# Begin Source File

SOURCE=.\CharacterHead.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CharacterHead.h
# End Source File
# Begin Source File

SOURCE=.\CombinedLifeMeterTug.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CombinedLifeMeterTug.h
# End Source File
# Begin Source File

SOURCE=.\Combo.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Combo.h
# End Source File
# Begin Source File

SOURCE=.\DancingCharacters.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DancingCharacters.h
# End Source File
# Begin Source File

SOURCE=.\Foreground.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Foreground.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.h
# End Source File
# Begin Source File

SOURCE=.\Judgment.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Judgment.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeter.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.h
# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteField.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteField.h
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPlus.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteFieldPlus.h
# End Source File
# Begin Source File

SOURCE=.\PercentageDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PercentageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\Player.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\ProTimingDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ProTimingDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ReceptorArrow.h
# End Source File
# Begin Source File

SOURCE=.\ReceptorArrowRow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ReceptorArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayPercentage.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayPercentage.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.h
# End Source File
# End Group
# Begin Group "Actors used in Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BPMDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\BPMDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ComboGraph.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ComboGraph.h
# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.h
# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DifficultyDisplay.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DifficultyList.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.h
# End Source File
# Begin Source File

SOURCE=.\DualScrollBar.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DualScrollBar.h
# End Source File
# Begin Source File

SOURCE=.\EditCoursesMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EditCoursesMenu.h
# End Source File
# Begin Source File

SOURCE=.\EditMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\EditMenu.h
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GraphDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GraphDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.h
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.h
# End Source File
# Begin Source File

SOURCE=.\GroupList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GroupList.h
# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.h
# End Source File
# Begin Source File

SOURCE=.\JukeboxMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\JukeboxMenu.h
# End Source File
# Begin Source File

SOURCE=.\ListDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ListDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MemoryCardDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MemoryCardDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MenuElements.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MenuElements.h
# End Source File
# Begin Source File

SOURCE=.\MenuTimer.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MenuTimer.h
# End Source File
# Begin Source File

SOURCE=.\ModeSwitcher.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ModeSwitcher.h
# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MusicList.h
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MusicWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.h
# End Source File
# Begin Source File

SOURCE=.\OptionIcon.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\OptionIcon.h
# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.h
# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.h
# End Source File
# Begin Source File

SOURCE=.\PaneDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PaneDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ScrollBar.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScrollBar.h
# End Source File
# Begin Source File

SOURCE=.\ScrollingList.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScrollingList.h
# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.h
# End Source File
# Begin Source File

SOURCE=.\SongCreditDisplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SongCreditDisplay.h
# End Source File
# Begin Source File

SOURCE=.\TextBanner.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TextBanner.h
# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.h
# End Source File
# End Group
# Begin Group "Screens"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Screen.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Screen.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAward.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenAward.h
# End Source File
# Begin Source File

SOURCE=.\ScreenBookkeeping.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenBookkeeping.h
# End Source File
# Begin Source File

SOURCE=.\ScreenBranch.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenBranch.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCaution.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenCaution.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCenterImage.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenCenterImage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDownloadMachineStats.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenDownloadMachineStats.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditCoursesMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEditCoursesMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD BASE CPP /YX"global.h"
# ADD CPP /YX"global.h"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEnding.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEnding.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEndlessBreak.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEndlessBreak.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.h
# End Source File
# Begin Source File

SOURCE=.\ScreenExit.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenExit.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameOver.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenGameOver.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJukeboxMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenJukeboxMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMessage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntryTraditional.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntryTraditional.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMaster.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMaster.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMasterPrefs.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMasterPrefs.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenProfileOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenProfileOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.h
# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.h
# End Source File
# Begin Source File

SOURCE=.\ScreenRaveOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenRaveOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenReloadSongs.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenReloadSongs.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCourse.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCourse.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficultyEX.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficultyEX.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMaster.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMaster.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSetTime.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSetTime.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenStage.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenStage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenStyleSplash.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenStyleSplash.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTest.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTest.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestInput.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTestInput.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestLights.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTestLights.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenUnlock.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenUnlock.h
# End Source File
# End Group
# Begin Group "Global Singletons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnnouncerManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\AnnouncerManager.h
# End Source File
# Begin Source File

SOURCE=.\Bookkeeper.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Bookkeeper.h
# End Source File
# Begin Source File

SOURCE=.\CryptManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\CryptManager.h
# End Source File
# Begin Source File

SOURCE=.\ezsockets.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ezsockets.h
# End Source File
# Begin Source File

SOURCE=.\FontManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\FontManager.h
# End Source File
# Begin Source File

SOURCE=.\GameManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameManager.h
# End Source File
# Begin Source File

SOURCE=.\GameState.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GameState.h
# End Source File
# Begin Source File

SOURCE=.\InputFilter.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\InputFilter.h
# End Source File
# Begin Source File

SOURCE=.\InputMapper.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\InputMapper.h
# End Source File
# Begin Source File

SOURCE=.\InputQueue.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\InputQueue.h
# End Source File
# Begin Source File

SOURCE=.\LightsManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\LightsManager.h
# End Source File
# Begin Source File

SOURCE=.\MemoryCardManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MemoryCardManager.h
# End Source File
# Begin Source File

SOURCE=.\NetworkSyncManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NetworkSyncManager.h
# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.h
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\PrefsManager.h
# End Source File
# Begin Source File

SOURCE=.\ProfileManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ProfileManager.h
# End Source File
# Begin Source File

SOURCE=.\ScreenManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenManager.h
# End Source File
# Begin Source File

SOURCE=.\SongManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SongManager.h
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ThemeManager.h
# End Source File
# Begin Source File

SOURCE=.\UnlockSystem.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\UnlockSystem.h
# End Source File
# End Group
# Begin Group "Crypto"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\crypto\CryptBn.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptBn.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptMD5.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptMD5.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptNoise.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptPrime.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptPrime.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRand.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRand.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRSA.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptRSA.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSH512.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSH512.h
# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSHA.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Debug"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Xbox Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypto\CryptSHA.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\error.bmp
# End Source File
# Begin Source File

SOURCE=.\error2.bmp
# End Source File
# Begin Source File

SOURCE=.\loading.bmp
# End Source File
# Begin Source File

SOURCE=.\StepMania.xpm
# End Source File
# End Target
# End Project
