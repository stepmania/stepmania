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
# PROP Output_Dir "../"
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
# PROP Output_Dir "../"
# PROP Intermediate_Dir "../Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"../StepMania-debug.exe" /pdbtype:sept

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

SOURCE=.\RageHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\RageHelper.h
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

SOURCE=.\RageSoundSample.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundSample.h
# End Source File
# Begin Source File

SOURCE=.\RageSoundStream.cpp
# End Source File
# Begin Source File

SOURCE=.\RageSoundStream.h
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

SOURCE=.\WindowCaution.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowCaution.h
# End Source File
# Begin Source File

SOURCE=.\WindowConfigurePads.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowConfigurePads.h
# End Source File
# Begin Source File

SOURCE=.\WindowCustomGraphicOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowCustomGraphicOptions.h
# End Source File
# Begin Source File

SOURCE=.\WindowDancing.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowDancing.h
# End Source File
# Begin Source File

SOURCE=.\WindowEdit.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowEdit.h
# End Source File
# Begin Source File

SOURCE=.\WindowEditMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowEditMenu.h
# End Source File
# Begin Source File

SOURCE=.\WindowGameOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowGameOptions.h
# End Source File
# Begin Source File

SOURCE=.\WindowGraphicOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowGraphicOptions.h
# End Source File
# Begin Source File

SOURCE=.\WindowManager.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowManager.h
# End Source File
# Begin Source File

SOURCE=.\WindowMessage.h
# End Source File
# Begin Source File

SOURCE=.\WindowOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowOptions.h
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

SOURCE=.\WindowSelectDifficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectGroup.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectGroup.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectMusic.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectMusic.h
# End Source File
# Begin Source File

SOURCE=.\WindowSelectStyle.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSelectStyle.h
# End Source File
# Begin Source File

SOURCE=.\WindowSongOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSongOptions.h
# End Source File
# Begin Source File

SOURCE=.\WindowSynchronize.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSynchronize.h
# End Source File
# Begin Source File

SOURCE=.\WindowSynchronizeMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowSynchronizeMenu.h
# End Source File
# Begin Source File

SOURCE=.\WindowTitleMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowTitleMenu.h
# End Source File
# End Group
# Begin Group "Data Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Font.cpp
# End Source File
# Begin Source File

SOURCE=.\Font.h
# End Source File
# Begin Source File

SOURCE=.\FontManager.cpp
# End Source File
# Begin Source File

SOURCE=.\FontManager.h
# End Source File
# Begin Source File

SOURCE=.\GameConstants.h
# End Source File
# Begin Source File

SOURCE=.\GameDef.cpp
# End Source File
# Begin Source File

SOURCE=.\GameDef.h
# End Source File
# Begin Source File

SOURCE=.\GameInput.h
# End Source File
# Begin Source File

SOURCE=.\GameManager.cpp
# End Source File
# Begin Source File

SOURCE=.\GameManager.h
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

SOURCE=.\NoteData.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteData.h
# End Source File
# Begin Source File

SOURCE=.\NoteMetadata.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteMetadata.h
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

SOURCE=.\PrefsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\PrefsManager.h
# End Source File
# Begin Source File

SOURCE=.\RandomSample.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomSample.h
# End Source File
# Begin Source File

SOURCE=.\RandomStream.cpp
# End Source File
# Begin Source File

SOURCE=.\RandomStream.h
# End Source File
# Begin Source File

SOURCE=.\Song.cpp
# End Source File
# Begin Source File

SOURCE=.\song.h
# End Source File
# Begin Source File

SOURCE=.\SongManager.cpp
# End Source File
# Begin Source File

SOURCE=.\SongManager.h
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

SOURCE=.\ThemeManager.cpp
# End Source File
# Begin Source File

SOURCE=.\ThemeManager.h
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
# End Group
# Begin Group "System"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dxutil.cpp
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

SOURCE=.\StepMania.h
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

SOURCE=.\TransitionInvisible.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionInvisible.h
# End Source File
# Begin Source File

SOURCE=.\TransitionKeepAlive.cpp
# End Source File
# Begin Source File

SOURCE=.\TransitionKeepAlive.h
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
# Begin Group "Actors"

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

SOURCE=.\BitmapText.cpp
# End Source File
# Begin Source File

SOURCE=.\BitmapText.h
# End Source File
# Begin Source File

SOURCE=.\RectangleActor.cpp
# End Source File
# Begin Source File

SOURCE=.\RectangleActor.h
# End Source File
# Begin Source File

SOURCE=.\Sprite.cpp
# End Source File
# Begin Source File

SOURCE=.\Sprite.h
# End Source File
# End Group
# Begin Group "Actors used in Menus"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Banner.cpp
# End Source File
# Begin Source File

SOURCE=.\Banner.h
# End Source File
# Begin Source File

SOURCE=.\BPMDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\BPMDisplay.h
# End Source File
# Begin Source File

SOURCE=.\CroppedSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\CroppedSprite.h
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.cpp
# End Source File
# Begin Source File

SOURCE=.\DifficultyIcon.h
# End Source File
# Begin Source File

SOURCE=.\FootMeter.cpp
# End Source File
# Begin Source File

SOURCE=.\FootMeter.h
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\GradeDisplay.h
# End Source File
# Begin Source File

SOURCE=.\GranularityIndicator.cpp
# End Source File
# Begin Source File

SOURCE=.\GranularityIndicator.h
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.cpp
# End Source File
# Begin Source File

SOURCE=.\GrooveRadar.h
# End Source File
# Begin Source File

SOURCE=.\GroupInfoFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupInfoFrame.h
# End Source File
# Begin Source File

SOURCE=.\MenuElements.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuElements.h
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicSortDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MusicStatusDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicStatusDisplay.h
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicWheel.h
# End Source File
# Begin Source File

SOURCE=.\SongInfoFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\SongInfoFrame.h
# End Source File
# Begin Source File

SOURCE=.\TextBanner.cpp
# End Source File
# Begin Source File

SOURCE=.\TextBanner.h
# End Source File
# Begin Source File

SOURCE=.\TipDisplay.cpp
# End Source File
# Begin Source File

SOURCE=.\TipDisplay.h
# End Source File
# End Group
# Begin Group "Actors used in Dancing"

# PROP Default_Filter ""
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

SOURCE=.\ColorArrow.cpp
# End Source File
# Begin Source File

SOURCE=.\ColorArrow.h
# End Source File
# Begin Source File

SOURCE=.\Combo.cpp
# End Source File
# Begin Source File

SOURCE=.\Combo.h
# End Source File
# Begin Source File

SOURCE=.\FocusingSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\FocusingSprite.h
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

SOURCE=.\HoldJudgement.cpp
# End Source File
# Begin Source File

SOURCE=.\HoldJudgement.h
# End Source File
# Begin Source File

SOURCE=.\Judgement.cpp
# End Source File
# Begin Source File

SOURCE=.\Judgement.h
# End Source File
# Begin Source File

SOURCE=.\LifeMeterPills.cpp
# End Source File
# Begin Source File

SOURCE=.\LifeMeterPills.h
# End Source File
# Begin Source File

SOURCE=.\MotionBlurSprite.cpp
# End Source File
# Begin Source File

SOURCE=.\MotionBlurSprite.h
# End Source File
# Begin Source File

SOURCE=.\NoteField.cpp
# End Source File
# Begin Source File

SOURCE=.\NoteField.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRolling.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRolling.h
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRollingWithFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\ScoreDisplayRollingWithFrame.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\error.bmp
# End Source File
# Begin Source File

SOURCE=.\splash.bmp
# End Source File
# End Target
# End Project
