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
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../"
# PROP Intermediate_Dir "../Release6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O2 /Ob2 /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../StepMania-StackTrace.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../release6/StepMania.pdb" /map /debug /machine:I386
# SUBTRACT LINK32 /verbose /pdb:none
# Begin Special Build Tool
IntDir=.\../Release6
TargetDir=\temp\stepmania
TargetName=StepMania
SOURCE="$(InputPath)"
PreLink_Cmds=disasm\verinc                                                                                                            	cl                                                                                                             /Zl                                                                                                             /nologo                                                                                                             /c                                                                                                             verstub.cpp                                                                                                             /Fo$(IntDir)\ 
PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi ia32.vdi
# End Special Build Tool

!ELSEIF  "$(CFG)" == "StepMania - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../"
# PROP Intermediate_Dir "../Debug6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /GX /ZI /Od /I "." /I "SDL-1.2.5\include" /I "SDL_image-1.2" /I "plib-1.6.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /Fr /YX"global.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 $(intdir)\verstub.obj kernel32.lib shell32.lib user32.lib gdi32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:"../debug6/StepMania-debug.pdb" /map /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"../StepMania-debug.exe"
# SUBTRACT LINK32 /verbose /profile /pdb:none /incremental:no /nodefaultlib
# Begin Special Build Tool
IntDir=.\../Debug6
TargetDir=\temp\stepmania
TargetName=StepMania-debug
SOURCE="$(InputPath)"
PreLink_Cmds=disasm\verinc                                                                                                            	cl                                                                                                             /Zl                                                                                                             /nologo                                                                                                             /c                                                                                                             verstub.cpp                                                                                                             /Fo$(IntDir)\ 
PostBuild_Cmds=disasm\mapconv $(IntDir)\$(TargetName).map $(TargetDir)\StepMania.vdi ia32.vdi
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "StepMania - Win32 Release"
# Name "StepMania - Win32 Debug"
# Begin Group "Rage"

# PROP Default_Filter ""
# Begin Group "Helpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\regex.c
# End Source File
# Begin Source File

SOURCE=.\regex.h
# End Source File
# Begin Source File

SOURCE=.\SDL_dither.cpp
# End Source File
# Begin Source File

SOURCE=.\SDL_dither.h
# End Source File
# Begin Source File

SOURCE=.\SDL_rotozoom.cpp
# End Source File
# Begin Source File

SOURCE=.\SDL_rotozoom.h
# End Source File
# Begin Source File

SOURCE=.\SDL_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\SDL_utils.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\RageBitmapTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\RageBitmapTexture.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\RageDisplay.h
# End Source File
# Begin Source File

SOURCE=.\RageDisplayInternal.h
# End Source File
# Begin Source File

SOURCE=.\RageException.cpp
# End Source File
# Begin Source File

SOURCE=.\RageException.h
# End Source File
# Begin Source File

SOURCE=.\RageInput.cpp
# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\RageInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\RageLog.cpp
# End Source File
# Begin Source File

SOURCE=.\RageLog.h
# End Source File
# Begin Source File

SOURCE=.\RageMath.cpp
# End Source File
# Begin Source File

SOURCE=.\RageMath.h
# End Source File
# Begin Source File

SOURCE=.\RageSound.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSound.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundManager.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Preload.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_Resample.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_SDL_Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundReader_SDL_Sound.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundResampler.h
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

SOURCE=.\RageThreads.cpp
# End Source File
# Begin Source File

SOURCE=.\RageThreads.h
# End Source File
# Begin Source File

SOURCE=.\RageTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\RageTimer.h
# End Source File
# Begin Source File

SOURCE=.\RageTypes.h
# End Source File
# Begin Source File

SOURCE=.\RageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RageUtil.h
# End Source File
# End Group
# Begin Group "Data Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ArrowBackdrop.cpp
# End Source File
# Begin Source File

SOURCE=.\ArrowBackdrop.h
# End Source File
# Begin Source File

SOURCE=.\Character.cpp
# End Source File
# Begin Source File

SOURCE=.\Character.h
# End Source File
# Begin Source File

SOURCE=.\CodeDetector.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeDetector.h
# End Source File
# Begin Source File

SOURCE=.\Course.cpp
# End Source File
# Begin Source File

SOURCE=.\Course.h
# End Source File
# Begin Source File

SOURCE=.\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.cpp
# End Source File
# Begin Source File

SOURCE=.\FontCharAliases.h
# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.cpp
# End Source File
# Begin Source File

SOURCE=.\FontCharmaps.h
# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\GameConstantsAndTypes.h
# End Source File
# Begin Source File

SOURCE=.\GameDef.cpp
# End Source File
# Begin Source File

SOURCE=.\GameDef.h
# End Source File
# Begin Source File

SOURCE=.\GameInput.cpp
# End Source File
# Begin Source File

SOURCE=.\GameInput.h
# End Source File
# Begin Source File

SOURCE=.\GameTypes.h
# End Source File
# Begin Source File

SOURCE=.\Grade.cpp
# End Source File
# Begin Source File

SOURCE=.\Grade.h
# End Source File
# Begin Source File

SOURCE=.\Inventory.cpp
# End Source File
# Begin Source File

SOURCE=.\Inventory.h
# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\LyricsLoader.h
# End Source File
# Begin Source File

SOURCE=.\ModeChoice.h
# End Source File
# Begin Source File

SOURCE=.\NoteData.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteData.h
# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDataWithScoring.h
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteFieldPositioning.h
# End Source File
# Begin Source File

SOURCE=.\Notes.cpp
# End Source File
# Begin Source File

SOURCE=.\Notes.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoader.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderBMS.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderKSF.h
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesLoaderSM.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesWriterDWI.h
# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.cpp
# End Source File
# Begin Source File

SOURCE=.\NotesWriterSM.h
# End Source File
# Begin Source File

SOURCE=.\NoteTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteTypes.h
# End Source File
# Begin Source File

SOURCE=.\PlayerAI.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerAI.h
# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerNumber.h
# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\RandomSample.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSample.h
# End Source File
# Begin Source File

SOURCE=.\RaveHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\RaveHelper.h
# End Source File
# Begin Source File

SOURCE=.\Song.cpp
# End Source File
# Begin Source File

SOURCE=.\song.h
# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.cpp
# End Source File
# Begin Source File

SOURCE=.\SongCacheIndex.h
# End Source File
# Begin Source File

SOURCE=.\SongOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\SongOptions.h
# End Source File
# Begin Source File

SOURCE=.\StageStats.cpp
# End Source File
# Begin Source File

SOURCE=.\StageStats.h
# End Source File
# Begin Source File

SOURCE=.\StyleDef.cpp
# End Source File
# Begin Source File

SOURCE=.\StyleDef.h
# End Source File
# Begin Source File

SOURCE=.\StyleInput.h
# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.cpp
# End Source File
# Begin Source File

SOURCE=.\TitleSubstitution.h
# End Source File
# End Group
# Begin Group "File Types"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IniFile.cpp
# End Source File
# Begin Source File

SOURCE=.\IniFile.h
# End Source File
# Begin Source File

SOURCE=.\MsdFile.cpp
# End Source File
# Begin Source File

SOURCE=.\MsdFile.h
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

SOURCE=.\arch\LoadingWindow\LoadingWindow_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_SDL.h
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.h
# End Source File
# End Group
# Begin Group "Sound"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\DSoundHelpers.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_Null.h
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.h
# End Source File
# End Group
# Begin Group "ErrorDialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog.h
# End Source File
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog_null.h
# End Source File
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog_stdout.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog_stdout.h
# End Source File
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\ErrorDialog\ErrorDialog_Win32.h
# End Source File
# End Group
# Begin Group "ArchHooks"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_none.h
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\ArchHooks\ArchHooks_Win32.h
# End Source File
# End Group
# Begin Group "InputHandler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_SDL.h
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.h
# End Source File
# End Group
# Begin Group "MovieTexture"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShow.h
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.h
# End Source File
# End Group
# Begin Group "LowLevelWindow"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow.h
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_SDL.cpp
# End Source File
# Begin Source File

SOURCE=.\arch\LowLevelWindow\LowLevelWindow_SDL.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\arch\arch.cpp
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
# End Group
# Begin Group "system"

# PROP Default_Filter ""
# Begin Group "Disasm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\archutils\Win32\Crash.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Crash.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\CrashList.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Disasm.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Disasm.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Tls.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\Tls.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\AppInstance.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\DebugInfoHunt.h
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\GotoURL.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.ICO
# End Source File
# Begin Source File

SOURCE=.\StepMania.RC
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.cpp
# End Source File
# Begin Source File

SOURCE=.\archutils\Win32\USB.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\global.cpp

!IF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD CPP /Yc"global.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\global.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDimensions.h
# End Source File
# Begin Source File

SOURCE=.\StdString.h
# End Source File
# Begin Source File

SOURCE=.\StepMania.cpp
# End Source File
# Begin Source File

SOURCE=.\StepMania.h
# End Source File
# End Group
# Begin Group "Actors"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Actor.cpp
# End Source File
# Begin Source File

SOURCE=.\Actor.h
# End Source File
# Begin Source File

SOURCE=.\ActorCollision.h
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorFrame.h
# End Source File
# Begin Source File

SOURCE=.\ActorScroller.cpp
# End Source File
# Begin Source File

SOURCE=.\ActorScroller.h
# End Source File
# Begin Source File

SOURCE=.\ActorUtil.h
# End Source File
# Begin Source File

SOURCE=.\BitmapText.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapText.h
# End Source File
# Begin Source File

SOURCE=.\Quad.h
# End Source File
# Begin Source File

SOURCE=.\Sample3dObject.cpp
# End Source File
# Begin Source File

SOURCE=.\Sample3dObject.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay & Menu"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Banner.cpp
# End Source File
# Begin Source File

SOURCE=.\Banner.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\BGAnimation.h
# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.cpp
# End Source File
# Begin Source File

SOURCE=.\BGAnimationLayer.h
# End Source File
# Begin Source File

SOURCE=.\CroppedSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\CroppedSprite.h
# End Source File
# Begin Source File

SOURCE=.\FadingBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\FadingBanner.h
# End Source File
# Begin Source File

SOURCE=.\Transition.cpp
# End Source File
# Begin Source File

SOURCE=.\Transition.h
# End Source File
# End Group
# Begin Group "Actors used in Gameplay"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActiveItemList.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveItemList.h
# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.cpp
# End Source File
# Begin Source File

SOURCE=.\ArrowEffects.h
# End Source File
# Begin Source File

SOURCE=.\Background.cpp
# End Source File
# Begin Source File

SOURCE=.\Background.h
# End Source File
# Begin Source File

SOURCE=.\Combo.cpp
# End Source File
# Begin Source File

SOURCE=.\Combo.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrowBright.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrowBright.h
# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.cpp
# End Source File
# Begin Source File

SOURCE=.\GhostArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\GrayArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayArrow.h
# End Source File
# Begin Source File

SOURCE=.\GrayArrowRow.cpp
# End Source File
# Begin Source File

SOURCE=.\GrayArrowRow.h
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldGhostArrow.h
# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldJudgment.h
# End Source File
# Begin Source File

SOURCE=.\Judgment.cpp
# End Source File
# Begin Source File

SOURCE=.\Judgment.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeter.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBar.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterBattery.h
# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\LyricDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteDisplay.h
# End Source File
# Begin Source File

SOURCE=.\NoteField.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteField.h
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplay.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayBattle.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayNormal.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayOni.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRave.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeper.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeper.h
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreKeeperMAX2.h
# End Source File
# Begin Source File

SOURCE=.\TimingAssist.cpp
# End Source File
# Begin Source File

SOURCE=.\TimingAssist.h
# End Source File
# End Group
# Begin Group "Actors used in Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BPMDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\BPMDisplay.h
# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.cpp
# End Source File
# Begin Source File

SOURCE=.\CourseContentsList.h
# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\CourseEntryDisplay.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyMeter.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyRating.h
# End Source File
# Begin Source File

SOURCE=.\EditMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\EditMenu.h
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.cpp
# End Source File
# Begin Source File

SOURCE=.\GrooveGraph.h
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.cpp
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.h
# End Source File
# Begin Source File

SOURCE=.\GroupList.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupList.h
# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpDisplay.h
# End Source File
# Begin Source File

SOURCE=.\JukeboxMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\JukeboxMenu.h
# End Source File
# Begin Source File

SOURCE=.\MenuElements.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuElements.h
# End Source File
# Begin Source File

SOURCE=.\MenuTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuTimer.h
# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicBannerWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicList.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicList.h
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicWheelItem.h
# End Source File
# Begin Source File

SOURCE=.\OptionIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionIcon.h
# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionIconRow.h
# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.cpp
# End Source File
# Begin Source File

SOURCE=.\OptionsCursor.h
# End Source File
# Begin Source File

SOURCE=.\ScrollBar.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollBar.h
# End Source File
# Begin Source File

SOURCE=.\ScrollingList.cpp
# End Source File
# Begin Source File

SOURCE=.\ScrollingList.h
# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\SnapDisplay.h
# End Source File
# Begin Source File

SOURCE=.\TextBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\TextBanner.h
# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\WheelNotifyIcon.h
# End Source File
# End Group
# Begin Group "Screens"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Screen.cpp
# End Source File
# Begin Source File

SOURCE=.\Screen.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAlbums.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenAlbums.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAppearanceOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenAppearanceOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenAttract.h
# End Source File
# Begin Source File

SOURCE=.\ScreenAutogenOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenAutogenOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCaution.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenCaution.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCompany.h
# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenCredits.h
# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenDemonstration.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEdit.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Release"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Debug"

# ADD CPP /YX"global.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ScreenEditMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEvaluation.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenEz2SelectPlayer.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameOver.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGameOver.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplayOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGameplayOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenGraphicOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenGraphicOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenHowToPlay.h
# End Source File
# Begin Source File

SOURCE=.\ScreenInputOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenInputOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenInstructions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenIntroMovie.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenIntroMovie.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenJukebox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenJukeboxMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenJukeboxMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenLogo.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMachineOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMachineOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMapControllers.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMemoryCard.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMessage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMiniMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenMusicScroll.h
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenNameEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenOptionsMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPlayerOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenPrompt.h
# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenRanking.h
# End Source File
# Begin Source File

SOURCE=.\ScreenRaveOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenRaveOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSandbox.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelect.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCharacter.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCourse.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectCourse.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficultyEX.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectDifficultyEX.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGame.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectGroup.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMode.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle5th.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSelectStyle5th.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSongOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenSoundOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenSoundOptions.h
# End Source File
# Begin Source File

SOURCE=.\ScreenStage.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenStage.h
# End Source File
# Begin Source File

SOURCE=.\ScreenStyleSplash.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenStyleSplash.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTest.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestFonts.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTestSound.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTextEntry.h
# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenTitleMenu.h
# End Source File
# Begin Source File

SOURCE=.\ScreenUnlock.h
# End Source File
# Begin Source File

SOURCE=.\ScreenWarning.h
# End Source File
# End Group
# Begin Group "Utils"

# PROP Default_Filter ""
# End Group
# Begin Group "Global Singletons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AnnouncerManager.cpp
# End Source File
# Begin Source File

SOURCE=.\AnnouncerManager.h
# End Source File
# Begin Source File

SOURCE=.\FontManager.cpp
# End Source File
# Begin Source File

SOURCE=.\FontManager.h
# End Source File
# Begin Source File

SOURCE=.\GameManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameManager.h
# End Source File
# Begin Source File

SOURCE=.\GameState.cpp
# End Source File
# Begin Source File

SOURCE=.\GameState.h
# End Source File
# Begin Source File

SOURCE=.\InputFilter.cpp
# End Source File
# Begin Source File

SOURCE=.\InputFilter.h
# End Source File
# Begin Source File

SOURCE=.\InputMapper.cpp
# End Source File
# Begin Source File

SOURCE=.\InputMapper.h
# End Source File
# Begin Source File

SOURCE=.\InputQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\InputQueue.h
# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteSkinManager.h
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.h
# End Source File
# Begin Source File

SOURCE=.\ScreenManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ScreenManager.h
# End Source File
# Begin Source File

SOURCE=.\SongManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SongManager.h
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.h
# End Source File
# End Group
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
