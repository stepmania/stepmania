# Microsoft Developer Studio Project File - Name="StepMania" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=StepMania - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak" CFG="StepMania - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StepMania - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "StepMania - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StepMania - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../Release"
# PROP Intermediate_Dir "../Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../Debug"
# PROP Intermediate_Dir "../Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "StepMania - Win32 Release"
# Name "StepMania - Win32 Debug"
# Begin Group "Rage"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RageBitmapTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\RageBitmapTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageInput.cpp
# End Source File
# Begin Source File

SOURCE=.\RageInput.h
# End Source File
# Begin Source File

SOURCE=.\RageMovieTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\RageMovieTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\RageMusic.h
# End Source File
# Begin Source File

SOURCE=.\RageScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\RageScreen.h
# End Source File
# Begin Source File

SOURCE=.\RageSound.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSound.h
# End Source File
# Begin Source File

SOURCE=.\RageTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTextureManager.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil.h
# End Source File
# End Group
# Begin Group "Windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Window.cpp
# End Source File
# Begin Source File

SOURCE=.\Window.h
# End Source File
# Begin Source File

SOURCE=.\WindowConfigurePads.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowConfigurePads.h
# End Source File
# Begin Source File

SOURCE=.\WindowDancing.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowDancing.h
# End Source File
# Begin Source File

SOURCE=.\WindowIntroCovers.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowIntroCovers.h
# End Source File
# Begin Source File

SOURCE=.\WindowLoading.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowLoading.h
# End Source File
# Begin Source File

SOURCE=.\WindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowManager.h
# End Source File
# Begin Source File

SOURCE=.\WindowPlayerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowPlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\WindowPrompt.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowPrompt.h
# End Source File
# Begin Source File

SOURCE=.\WindowResults.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowResults.h
# End Source File
# Begin Source File

SOURCE=.\WindowSandbox.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSandbox.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectGameMode.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectGameMode.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectSong.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectSong.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectSteps.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectSteps.h
# End Source File
# Begin Source File

SOURCE=.\WindowTitleMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowTitleMenu.h
# End Source File
# End Group
# Begin Group "Game Objects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor.cpp
# End Source File
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.h
# End Source File
# Begin Source File

SOURCE=.\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\Background.h
# End Source File
# Begin Source File

SOURCE=.\Banner.cpp
# End Source File
# Begin Source File

SOURCE=.\Banner.h
# End Source File
# Begin Source File

SOURCE=.\BitmapText.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapText.h
# End Source File
# Begin Source File

SOURCE=.\BlurredTitle.cpp
# End Source File
# Begin Source File

SOURCE=.\BlurredTitle.h
# End Source File
# Begin Source File

SOURCE=.\ColorArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorArrow.h
# End Source File
# Begin Source File

SOURCE=.\GameInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\GameInfo.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\GrayArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayArrow.h
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\PadInput.h
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\PlayerInput.h
# End Source File
# Begin Source File

SOURCE=.\PreviewGraphic.cpp
# End Source File
# Begin Source File

SOURCE=.\previewgraphic.h
# End Source File
# Begin Source File

SOURCE=.\Rectangle.cpp
# End Source File
# Begin Source File

SOURCE=.\Rectangle.h
# End Source File
# Begin Source File

SOURCE=.\Song.cpp
# End Source File
# Begin Source File

SOURCE=.\song.h
# End Source File
# Begin Source File

SOURCE=.\SoundSet.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundSet.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# Begin Source File

SOURCE=.\SpriteSequence.cpp
# End Source File
# Begin Source File

SOURCE=.\SpriteSequence.h
# End Source File
# Begin Source File

SOURCE=.\Steps.cpp
# End Source File
# Begin Source File

SOURCE=.\Steps.h
# End Source File
# Begin Source File

SOURCE=.\TextBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\TextBanner.h
# End Source File
# End Group
# Begin Group "File Types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BmsFile.cpp
# End Source File
# Begin Source File

SOURCE=.\BmsFile.h
# End Source File
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=.\getdxver.cpp
# End Source File
# Begin Source File

SOURCE=.\getdxver.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDimensions.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.cpp
# End Source File
# Begin Source File

SOURCE=.\StepMania.ICO
# End Source File
# Begin Source File

SOURCE=.\StepMania.RC
# End Source File
# End Group
# Begin Group "Transitions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Transition.cpp
# End Source File
# Begin Source File

SOURCE=.\Transition.h
# End Source File
# Begin Source File

SOURCE=.\TransitionFade.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionFade.h
# End Source File
# Begin Source File

SOURCE=.\TransitionFadeWipe.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionFadeWipe.h
# End Source File
# Begin Source File

SOURCE=.\TransitionRectWipe.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionRectWipe.h
# End Source File
# Begin Source File

SOURCE=.\TransitionStarburst.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionStarburst.h
# End Source File
# Begin Source File

SOURCE=.\TransitionStarWipe.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionStarWipe.h
# End Source File
# End Group
# End Target
# End Project
