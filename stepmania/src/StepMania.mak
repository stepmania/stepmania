# Microsoft Developer Studio Generated NMAKE File, Based on StepMania.dsp
!IF "$(CFG)" == ""
CFG=StepMania - Win32 Release
!MESSAGE No configuration specified. Defaulting to StepMania - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "StepMania - Win32 Debug" && "$(CFG)" != "StepMania - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StepMania.mak" CFG="StepMania - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StepMania - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "StepMania - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "StepMania - Win32 Debug"

OUTDIR=.\../Debug6
INTDIR=.\../Debug6
# Begin Custom Macros
OutDir=.\../Debug6
# End Custom Macros

ALL : "..\Program\StepMania-debug.exe" "$(OUTDIR)\StepMania.pch" "$(OUTDIR)\StepMania.bsc"


CLEAN :
	-@erase "$(INTDIR)\ActiveAttackList.obj"
	-@erase "$(INTDIR)\ActiveAttackList.sbr"
	-@erase "$(INTDIR)\Actor.obj"
	-@erase "$(INTDIR)\Actor.sbr"
	-@erase "$(INTDIR)\ActorCommands.obj"
	-@erase "$(INTDIR)\ActorCommands.sbr"
	-@erase "$(INTDIR)\ActorFrame.obj"
	-@erase "$(INTDIR)\ActorFrame.sbr"
	-@erase "$(INTDIR)\ActorScroller.obj"
	-@erase "$(INTDIR)\ActorScroller.sbr"
	-@erase "$(INTDIR)\ActorUtil.obj"
	-@erase "$(INTDIR)\ActorUtil.sbr"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algebra.sbr"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\algparam.sbr"
	-@erase "$(INTDIR)\AnnouncerManager.obj"
	-@erase "$(INTDIR)\AnnouncerManager.sbr"
	-@erase "$(INTDIR)\AppInstance.obj"
	-@erase "$(INTDIR)\AppInstance.sbr"
	-@erase "$(INTDIR)\arch.obj"
	-@erase "$(INTDIR)\arch.sbr"
	-@erase "$(INTDIR)\arch_setup.obj"
	-@erase "$(INTDIR)\arch_setup.sbr"
	-@erase "$(INTDIR)\ArchHooks.obj"
	-@erase "$(INTDIR)\ArchHooks.sbr"
	-@erase "$(INTDIR)\ArchHooks_Win32.obj"
	-@erase "$(INTDIR)\ArchHooks_Win32.sbr"
	-@erase "$(INTDIR)\ArrowEffects.obj"
	-@erase "$(INTDIR)\ArrowEffects.sbr"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\asn.sbr"
	-@erase "$(INTDIR)\Attack.obj"
	-@erase "$(INTDIR)\Attack.sbr"
	-@erase "$(INTDIR)\AttackDisplay.obj"
	-@erase "$(INTDIR)\AttackDisplay.sbr"
	-@erase "$(INTDIR)\AutoActor.obj"
	-@erase "$(INTDIR)\AutoActor.sbr"
	-@erase "$(INTDIR)\AutoKeysounds.obj"
	-@erase "$(INTDIR)\AutoKeysounds.sbr"
	-@erase "$(INTDIR)\Background.obj"
	-@erase "$(INTDIR)\Background.sbr"
	-@erase "$(INTDIR)\Banner.obj"
	-@erase "$(INTDIR)\Banner.sbr"
	-@erase "$(INTDIR)\BannerCache.obj"
	-@erase "$(INTDIR)\BannerCache.sbr"
	-@erase "$(INTDIR)\BeginnerHelper.obj"
	-@erase "$(INTDIR)\BeginnerHelper.sbr"
	-@erase "$(INTDIR)\BGAnimation.obj"
	-@erase "$(INTDIR)\BGAnimation.sbr"
	-@erase "$(INTDIR)\BGAnimationLayer.obj"
	-@erase "$(INTDIR)\BGAnimationLayer.sbr"
	-@erase "$(INTDIR)\BitmapText.obj"
	-@erase "$(INTDIR)\BitmapText.sbr"
	-@erase "$(INTDIR)\Bookkeeper.obj"
	-@erase "$(INTDIR)\Bookkeeper.sbr"
	-@erase "$(INTDIR)\BPMDisplay.obj"
	-@erase "$(INTDIR)\BPMDisplay.sbr"
	-@erase "$(INTDIR)\CatalogXml.obj"
	-@erase "$(INTDIR)\CatalogXml.sbr"
	-@erase "$(INTDIR)\Character.obj"
	-@erase "$(INTDIR)\Character.sbr"
	-@erase "$(INTDIR)\CharacterHead.obj"
	-@erase "$(INTDIR)\CharacterHead.sbr"
	-@erase "$(INTDIR)\CodeDetector.obj"
	-@erase "$(INTDIR)\CodeDetector.sbr"
	-@erase "$(INTDIR)\CombinedLifeMeterTug.obj"
	-@erase "$(INTDIR)\CombinedLifeMeterTug.sbr"
	-@erase "$(INTDIR)\Combo.obj"
	-@erase "$(INTDIR)\Combo.sbr"
	-@erase "$(INTDIR)\ComboGraph.obj"
	-@erase "$(INTDIR)\ComboGraph.sbr"
	-@erase "$(INTDIR)\Command.obj"
	-@erase "$(INTDIR)\Command.sbr"
	-@erase "$(INTDIR)\CommonMetrics.obj"
	-@erase "$(INTDIR)\CommonMetrics.sbr"
	-@erase "$(INTDIR)\Course.obj"
	-@erase "$(INTDIR)\Course.sbr"
	-@erase "$(INTDIR)\CourseContentsList.obj"
	-@erase "$(INTDIR)\CourseContentsList.sbr"
	-@erase "$(INTDIR)\CourseEntryDisplay.obj"
	-@erase "$(INTDIR)\CourseEntryDisplay.sbr"
	-@erase "$(INTDIR)\CourseUtil.obj"
	-@erase "$(INTDIR)\CourseUtil.sbr"
	-@erase "$(INTDIR)\Crash.obj"
	-@erase "$(INTDIR)\Crash.sbr"
	-@erase "$(INTDIR)\CryptBn.obj"
	-@erase "$(INTDIR)\CryptBn.sbr"
	-@erase "$(INTDIR)\CryptHelpers.obj"
	-@erase "$(INTDIR)\CryptHelpers.sbr"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.sbr"
	-@erase "$(INTDIR)\CryptManager.obj"
	-@erase "$(INTDIR)\CryptManager.sbr"
	-@erase "$(INTDIR)\CryptMD5.obj"
	-@erase "$(INTDIR)\CryptMD5.sbr"
	-@erase "$(INTDIR)\CryptNoise.obj"
	-@erase "$(INTDIR)\CryptNoise.sbr"
	-@erase "$(INTDIR)\CryptPrime.obj"
	-@erase "$(INTDIR)\CryptPrime.sbr"
	-@erase "$(INTDIR)\CryptRand.obj"
	-@erase "$(INTDIR)\CryptRand.sbr"
	-@erase "$(INTDIR)\CryptRSA.obj"
	-@erase "$(INTDIR)\CryptRSA.sbr"
	-@erase "$(INTDIR)\CryptSH512.obj"
	-@erase "$(INTDIR)\CryptSH512.sbr"
	-@erase "$(INTDIR)\CryptSHA.obj"
	-@erase "$(INTDIR)\CryptSHA.sbr"
	-@erase "$(INTDIR)\DancingCharacters.obj"
	-@erase "$(INTDIR)\DancingCharacters.sbr"
	-@erase "$(INTDIR)\DateTime.obj"
	-@erase "$(INTDIR)\DateTime.sbr"
	-@erase "$(INTDIR)\DebugInfoHunt.obj"
	-@erase "$(INTDIR)\DebugInfoHunt.sbr"
	-@erase "$(INTDIR)\Dialog.obj"
	-@erase "$(INTDIR)\Dialog.sbr"
	-@erase "$(INTDIR)\DialogDriver_Win32.obj"
	-@erase "$(INTDIR)\DialogDriver_Win32.sbr"
	-@erase "$(INTDIR)\Difficulty.obj"
	-@erase "$(INTDIR)\Difficulty.sbr"
	-@erase "$(INTDIR)\DifficultyDisplay.obj"
	-@erase "$(INTDIR)\DifficultyDisplay.sbr"
	-@erase "$(INTDIR)\DifficultyIcon.obj"
	-@erase "$(INTDIR)\DifficultyIcon.sbr"
	-@erase "$(INTDIR)\DifficultyList.obj"
	-@erase "$(INTDIR)\DifficultyList.sbr"
	-@erase "$(INTDIR)\DifficultyMeter.obj"
	-@erase "$(INTDIR)\DifficultyMeter.sbr"
	-@erase "$(INTDIR)\DifficultyRating.obj"
	-@erase "$(INTDIR)\DifficultyRating.sbr"
	-@erase "$(INTDIR)\DSoundHelpers.obj"
	-@erase "$(INTDIR)\DSoundHelpers.sbr"
	-@erase "$(INTDIR)\DualScrollBar.obj"
	-@erase "$(INTDIR)\DualScrollBar.sbr"
	-@erase "$(INTDIR)\EditCoursesMenu.obj"
	-@erase "$(INTDIR)\EditCoursesMenu.sbr"
	-@erase "$(INTDIR)\EditCoursesSongMenu.obj"
	-@erase "$(INTDIR)\EditCoursesSongMenu.sbr"
	-@erase "$(INTDIR)\EditMenu.obj"
	-@erase "$(INTDIR)\EditMenu.sbr"
	-@erase "$(INTDIR)\EnumHelper.obj"
	-@erase "$(INTDIR)\EnumHelper.sbr"
	-@erase "$(INTDIR)\ezsockets.obj"
	-@erase "$(INTDIR)\ezsockets.sbr"
	-@erase "$(INTDIR)\FadingBanner.obj"
	-@erase "$(INTDIR)\FadingBanner.sbr"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\files.sbr"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\filters.sbr"
	-@erase "$(INTDIR)\Font.obj"
	-@erase "$(INTDIR)\Font.sbr"
	-@erase "$(INTDIR)\FontCharAliases.obj"
	-@erase "$(INTDIR)\FontCharAliases.sbr"
	-@erase "$(INTDIR)\FontCharmaps.obj"
	-@erase "$(INTDIR)\FontCharmaps.sbr"
	-@erase "$(INTDIR)\FontManager.obj"
	-@erase "$(INTDIR)\FontManager.sbr"
	-@erase "$(INTDIR)\Foreground.obj"
	-@erase "$(INTDIR)\Foreground.sbr"
	-@erase "$(INTDIR)\Game.obj"
	-@erase "$(INTDIR)\Game.sbr"
	-@erase "$(INTDIR)\GameCommand.obj"
	-@erase "$(INTDIR)\GameCommand.sbr"
	-@erase "$(INTDIR)\GameConstantsAndTypes.obj"
	-@erase "$(INTDIR)\GameConstantsAndTypes.sbr"
	-@erase "$(INTDIR)\GameInput.obj"
	-@erase "$(INTDIR)\GameInput.sbr"
	-@erase "$(INTDIR)\GameManager.obj"
	-@erase "$(INTDIR)\GameManager.sbr"
	-@erase "$(INTDIR)\GameSoundManager.obj"
	-@erase "$(INTDIR)\GameSoundManager.sbr"
	-@erase "$(INTDIR)\GameState.obj"
	-@erase "$(INTDIR)\GameState.sbr"
	-@erase "$(INTDIR)\get.obj"
	-@erase "$(INTDIR)\get.sbr"
	-@erase "$(INTDIR)\GetFileInformation.obj"
	-@erase "$(INTDIR)\GetFileInformation.sbr"
	-@erase "$(INTDIR)\GhostArrow.obj"
	-@erase "$(INTDIR)\GhostArrow.sbr"
	-@erase "$(INTDIR)\GhostArrowRow.obj"
	-@erase "$(INTDIR)\GhostArrowRow.sbr"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\global.sbr"
	-@erase "$(INTDIR)\GotoURL.obj"
	-@erase "$(INTDIR)\GotoURL.sbr"
	-@erase "$(INTDIR)\Grade.obj"
	-@erase "$(INTDIR)\Grade.sbr"
	-@erase "$(INTDIR)\GradeDisplay.obj"
	-@erase "$(INTDIR)\GradeDisplay.sbr"
	-@erase "$(INTDIR)\GraphDisplay.obj"
	-@erase "$(INTDIR)\GraphDisplay.sbr"
	-@erase "$(INTDIR)\GraphicsWindow.obj"
	-@erase "$(INTDIR)\GraphicsWindow.sbr"
	-@erase "$(INTDIR)\GrooveGraph.obj"
	-@erase "$(INTDIR)\GrooveGraph.sbr"
	-@erase "$(INTDIR)\GrooveRadar.obj"
	-@erase "$(INTDIR)\GrooveRadar.sbr"
	-@erase "$(INTDIR)\GroupList.obj"
	-@erase "$(INTDIR)\GroupList.sbr"
	-@erase "$(INTDIR)\HelpDisplay.obj"
	-@erase "$(INTDIR)\HelpDisplay.sbr"
	-@erase "$(INTDIR)\HighScore.obj"
	-@erase "$(INTDIR)\HighScore.sbr"
	-@erase "$(INTDIR)\HoldGhostArrow.obj"
	-@erase "$(INTDIR)\HoldGhostArrow.sbr"
	-@erase "$(INTDIR)\HoldJudgment.obj"
	-@erase "$(INTDIR)\HoldJudgment.sbr"
	-@erase "$(INTDIR)\IniFile.obj"
	-@erase "$(INTDIR)\IniFile.sbr"
	-@erase "$(INTDIR)\InputFilter.obj"
	-@erase "$(INTDIR)\InputFilter.sbr"
	-@erase "$(INTDIR)\InputHandler.obj"
	-@erase "$(INTDIR)\InputHandler.sbr"
	-@erase "$(INTDIR)\InputHandler_DirectInput.obj"
	-@erase "$(INTDIR)\InputHandler_DirectInput.sbr"
	-@erase "$(INTDIR)\InputHandler_DirectInputHelper.obj"
	-@erase "$(INTDIR)\InputHandler_DirectInputHelper.sbr"
	-@erase "$(INTDIR)\InputHandler_MonkeyKeyboard.obj"
	-@erase "$(INTDIR)\InputHandler_MonkeyKeyboard.sbr"
	-@erase "$(INTDIR)\InputHandler_Win32_Para.obj"
	-@erase "$(INTDIR)\InputHandler_Win32_Para.sbr"
	-@erase "$(INTDIR)\InputHandler_Win32_Pump.obj"
	-@erase "$(INTDIR)\InputHandler_Win32_Pump.sbr"
	-@erase "$(INTDIR)\InputMapper.obj"
	-@erase "$(INTDIR)\InputMapper.sbr"
	-@erase "$(INTDIR)\InputQueue.obj"
	-@erase "$(INTDIR)\InputQueue.sbr"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\integer.sbr"
	-@erase "$(INTDIR)\Inventory.obj"
	-@erase "$(INTDIR)\Inventory.sbr"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\iterhash.sbr"
	-@erase "$(INTDIR)\Judgment.obj"
	-@erase "$(INTDIR)\Judgment.sbr"
	-@erase "$(INTDIR)\LifeMeterBar.obj"
	-@erase "$(INTDIR)\LifeMeterBar.sbr"
	-@erase "$(INTDIR)\LifeMeterBattery.obj"
	-@erase "$(INTDIR)\LifeMeterBattery.sbr"
	-@erase "$(INTDIR)\LifeMeterTime.obj"
	-@erase "$(INTDIR)\LifeMeterTime.sbr"
	-@erase "$(INTDIR)\LightsDriver_SystemMessage.obj"
	-@erase "$(INTDIR)\LightsDriver_SystemMessage.sbr"
	-@erase "$(INTDIR)\LightsDriver_Win32Parallel.obj"
	-@erase "$(INTDIR)\LightsDriver_Win32Parallel.sbr"
	-@erase "$(INTDIR)\LightsManager.obj"
	-@erase "$(INTDIR)\LightsManager.sbr"
	-@erase "$(INTDIR)\LoadingWindow_Win32.obj"
	-@erase "$(INTDIR)\LoadingWindow_Win32.sbr"
	-@erase "$(INTDIR)\LowLevelWindow_Win32.obj"
	-@erase "$(INTDIR)\LowLevelWindow_Win32.sbr"
	-@erase "$(INTDIR)\LuaManager.obj"
	-@erase "$(INTDIR)\LuaManager.sbr"
	-@erase "$(INTDIR)\LuaReference.obj"
	-@erase "$(INTDIR)\LuaReference.sbr"
	-@erase "$(INTDIR)\LyricDisplay.obj"
	-@erase "$(INTDIR)\LyricDisplay.sbr"
	-@erase "$(INTDIR)\LyricsLoader.obj"
	-@erase "$(INTDIR)\LyricsLoader.sbr"
	-@erase "$(INTDIR)\maketables.obj"
	-@erase "$(INTDIR)\maketables.sbr"
	-@erase "$(INTDIR)\MemoryCardDisplay.obj"
	-@erase "$(INTDIR)\MemoryCardDisplay.sbr"
	-@erase "$(INTDIR)\MemoryCardDriver.obj"
	-@erase "$(INTDIR)\MemoryCardDriver.sbr"
	-@erase "$(INTDIR)\MemoryCardDriverThreaded_Windows.obj"
	-@erase "$(INTDIR)\MemoryCardDriverThreaded_Windows.sbr"
	-@erase "$(INTDIR)\MemoryCardManager.obj"
	-@erase "$(INTDIR)\MemoryCardManager.sbr"
	-@erase "$(INTDIR)\MenuTimer.obj"
	-@erase "$(INTDIR)\MenuTimer.sbr"
	-@erase "$(INTDIR)\MessageManager.obj"
	-@erase "$(INTDIR)\MessageManager.sbr"
	-@erase "$(INTDIR)\MeterDisplay.obj"
	-@erase "$(INTDIR)\MeterDisplay.sbr"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\misc.sbr"
	-@erase "$(INTDIR)\Model.obj"
	-@erase "$(INTDIR)\Model.sbr"
	-@erase "$(INTDIR)\ModelManager.obj"
	-@erase "$(INTDIR)\ModelManager.sbr"
	-@erase "$(INTDIR)\ModelTypes.obj"
	-@erase "$(INTDIR)\ModelTypes.sbr"
	-@erase "$(INTDIR)\ModeSwitcher.obj"
	-@erase "$(INTDIR)\ModeSwitcher.sbr"
	-@erase "$(INTDIR)\MovieTexture.obj"
	-@erase "$(INTDIR)\MovieTexture.sbr"
	-@erase "$(INTDIR)\MovieTexture_DShow.obj"
	-@erase "$(INTDIR)\MovieTexture_DShow.sbr"
	-@erase "$(INTDIR)\MovieTexture_DShowHelper.obj"
	-@erase "$(INTDIR)\MovieTexture_DShowHelper.sbr"
	-@erase "$(INTDIR)\MovieTexture_FFMpeg.obj"
	-@erase "$(INTDIR)\MovieTexture_FFMpeg.sbr"
	-@erase "$(INTDIR)\MovieTexture_Null.obj"
	-@erase "$(INTDIR)\MovieTexture_Null.sbr"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqueue.sbr"
	-@erase "$(INTDIR)\MsdFile.obj"
	-@erase "$(INTDIR)\MsdFile.sbr"
	-@erase "$(INTDIR)\MusicBannerWheel.obj"
	-@erase "$(INTDIR)\MusicBannerWheel.sbr"
	-@erase "$(INTDIR)\MusicList.obj"
	-@erase "$(INTDIR)\MusicList.sbr"
	-@erase "$(INTDIR)\MusicSortDisplay.obj"
	-@erase "$(INTDIR)\MusicSortDisplay.sbr"
	-@erase "$(INTDIR)\MusicWheel.obj"
	-@erase "$(INTDIR)\MusicWheel.sbr"
	-@erase "$(INTDIR)\MusicWheelItem.obj"
	-@erase "$(INTDIR)\MusicWheelItem.sbr"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\nbtheory.sbr"
	-@erase "$(INTDIR)\NetworkSyncManager.obj"
	-@erase "$(INTDIR)\NetworkSyncManager.sbr"
	-@erase "$(INTDIR)\NetworkSyncServer.obj"
	-@erase "$(INTDIR)\NetworkSyncServer.sbr"
	-@erase "$(INTDIR)\NoteData.obj"
	-@erase "$(INTDIR)\NoteData.sbr"
	-@erase "$(INTDIR)\NoteDataUtil.obj"
	-@erase "$(INTDIR)\NoteDataUtil.sbr"
	-@erase "$(INTDIR)\NoteDataWithScoring.obj"
	-@erase "$(INTDIR)\NoteDataWithScoring.sbr"
	-@erase "$(INTDIR)\NoteDisplay.obj"
	-@erase "$(INTDIR)\NoteDisplay.sbr"
	-@erase "$(INTDIR)\NoteField.obj"
	-@erase "$(INTDIR)\NoteField.sbr"
	-@erase "$(INTDIR)\NoteFieldPositioning.obj"
	-@erase "$(INTDIR)\NoteFieldPositioning.sbr"
	-@erase "$(INTDIR)\NoteSkinManager.obj"
	-@erase "$(INTDIR)\NoteSkinManager.sbr"
	-@erase "$(INTDIR)\NotesLoader.obj"
	-@erase "$(INTDIR)\NotesLoader.sbr"
	-@erase "$(INTDIR)\NotesLoaderBMS.obj"
	-@erase "$(INTDIR)\NotesLoaderBMS.sbr"
	-@erase "$(INTDIR)\NotesLoaderDWI.obj"
	-@erase "$(INTDIR)\NotesLoaderDWI.sbr"
	-@erase "$(INTDIR)\NotesLoaderKSF.obj"
	-@erase "$(INTDIR)\NotesLoaderKSF.sbr"
	-@erase "$(INTDIR)\NotesLoaderSM.obj"
	-@erase "$(INTDIR)\NotesLoaderSM.sbr"
	-@erase "$(INTDIR)\NotesWriterDWI.obj"
	-@erase "$(INTDIR)\NotesWriterDWI.sbr"
	-@erase "$(INTDIR)\NotesWriterSM.obj"
	-@erase "$(INTDIR)\NotesWriterSM.sbr"
	-@erase "$(INTDIR)\NoteTypes.obj"
	-@erase "$(INTDIR)\NoteTypes.sbr"
	-@erase "$(INTDIR)\OptionIcon.obj"
	-@erase "$(INTDIR)\OptionIcon.sbr"
	-@erase "$(INTDIR)\OptionIconRow.obj"
	-@erase "$(INTDIR)\OptionIconRow.sbr"
	-@erase "$(INTDIR)\OptionRow.obj"
	-@erase "$(INTDIR)\OptionRow.sbr"
	-@erase "$(INTDIR)\OptionRowHandler.obj"
	-@erase "$(INTDIR)\OptionRowHandler.sbr"
	-@erase "$(INTDIR)\OptionsCursor.obj"
	-@erase "$(INTDIR)\OptionsCursor.sbr"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\osrng.sbr"
	-@erase "$(INTDIR)\PaneDisplay.obj"
	-@erase "$(INTDIR)\PaneDisplay.sbr"
	-@erase "$(INTDIR)\pcre.obj"
	-@erase "$(INTDIR)\pcre.sbr"
	-@erase "$(INTDIR)\PercentageDisplay.obj"
	-@erase "$(INTDIR)\PercentageDisplay.sbr"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\pkcspad.sbr"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Player.sbr"
	-@erase "$(INTDIR)\PlayerAI.obj"
	-@erase "$(INTDIR)\PlayerAI.sbr"
	-@erase "$(INTDIR)\PlayerNumber.obj"
	-@erase "$(INTDIR)\PlayerNumber.sbr"
	-@erase "$(INTDIR)\PlayerOptions.obj"
	-@erase "$(INTDIR)\PlayerOptions.sbr"
	-@erase "$(INTDIR)\PlayerStageStats.obj"
	-@erase "$(INTDIR)\PlayerStageStats.sbr"
	-@erase "$(INTDIR)\PlayerState.obj"
	-@erase "$(INTDIR)\PlayerState.sbr"
	-@erase "$(INTDIR)\Preference.obj"
	-@erase "$(INTDIR)\Preference.sbr"
	-@erase "$(INTDIR)\PrefsManager.obj"
	-@erase "$(INTDIR)\PrefsManager.sbr"
	-@erase "$(INTDIR)\Profile.obj"
	-@erase "$(INTDIR)\Profile.sbr"
	-@erase "$(INTDIR)\ProfileManager.obj"
	-@erase "$(INTDIR)\ProfileManager.sbr"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\pubkey.sbr"
	-@erase "$(INTDIR)\Quad.obj"
	-@erase "$(INTDIR)\Quad.sbr"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\queue.sbr"
	-@erase "$(INTDIR)\RadarValues.obj"
	-@erase "$(INTDIR)\RadarValues.sbr"
	-@erase "$(INTDIR)\RageBitmapTexture.obj"
	-@erase "$(INTDIR)\RageBitmapTexture.sbr"
	-@erase "$(INTDIR)\RageDisplay.obj"
	-@erase "$(INTDIR)\RageDisplay.sbr"
	-@erase "$(INTDIR)\RageDisplay_D3D.obj"
	-@erase "$(INTDIR)\RageDisplay_D3D.sbr"
	-@erase "$(INTDIR)\RageDisplay_Null.obj"
	-@erase "$(INTDIR)\RageDisplay_Null.sbr"
	-@erase "$(INTDIR)\RageDisplay_OGL.obj"
	-@erase "$(INTDIR)\RageDisplay_OGL.sbr"
	-@erase "$(INTDIR)\RageDisplay_OGL_Extensions.obj"
	-@erase "$(INTDIR)\RageDisplay_OGL_Extensions.sbr"
	-@erase "$(INTDIR)\RageException.obj"
	-@erase "$(INTDIR)\RageException.sbr"
	-@erase "$(INTDIR)\RageFile.obj"
	-@erase "$(INTDIR)\RageFile.sbr"
	-@erase "$(INTDIR)\RageFileBasic.obj"
	-@erase "$(INTDIR)\RageFileBasic.sbr"
	-@erase "$(INTDIR)\RageFileDriver.obj"
	-@erase "$(INTDIR)\RageFileDriver.sbr"
	-@erase "$(INTDIR)\RageFileDriverDeflate.obj"
	-@erase "$(INTDIR)\RageFileDriverDeflate.sbr"
	-@erase "$(INTDIR)\RageFileDriverDirect.obj"
	-@erase "$(INTDIR)\RageFileDriverDirect.sbr"
	-@erase "$(INTDIR)\RageFileDriverDirectHelpers.obj"
	-@erase "$(INTDIR)\RageFileDriverDirectHelpers.sbr"
	-@erase "$(INTDIR)\RageFileDriverMemory.obj"
	-@erase "$(INTDIR)\RageFileDriverMemory.sbr"
	-@erase "$(INTDIR)\RageFileDriverSlice.obj"
	-@erase "$(INTDIR)\RageFileDriverSlice.sbr"
	-@erase "$(INTDIR)\RageFileDriverTimeout.obj"
	-@erase "$(INTDIR)\RageFileDriverTimeout.sbr"
	-@erase "$(INTDIR)\RageFileDriverZip.obj"
	-@erase "$(INTDIR)\RageFileDriverZip.sbr"
	-@erase "$(INTDIR)\RageFileManager.obj"
	-@erase "$(INTDIR)\RageFileManager.sbr"
	-@erase "$(INTDIR)\RageInput.obj"
	-@erase "$(INTDIR)\RageInput.sbr"
	-@erase "$(INTDIR)\RageInputDevice.obj"
	-@erase "$(INTDIR)\RageInputDevice.sbr"
	-@erase "$(INTDIR)\RageLog.obj"
	-@erase "$(INTDIR)\RageLog.sbr"
	-@erase "$(INTDIR)\RageMath.obj"
	-@erase "$(INTDIR)\RageMath.sbr"
	-@erase "$(INTDIR)\RageModelGeometry.obj"
	-@erase "$(INTDIR)\RageModelGeometry.sbr"
	-@erase "$(INTDIR)\RageSound.obj"
	-@erase "$(INTDIR)\RageSound.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_DSound.obj"
	-@erase "$(INTDIR)\RageSoundDriver_DSound.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_DSound_Software.obj"
	-@erase "$(INTDIR)\RageSoundDriver_DSound_Software.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_Generic_Software.obj"
	-@erase "$(INTDIR)\RageSoundDriver_Generic_Software.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_Null.obj"
	-@erase "$(INTDIR)\RageSoundDriver_Null.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_WaveOut.obj"
	-@erase "$(INTDIR)\RageSoundDriver_WaveOut.sbr"
	-@erase "$(INTDIR)\RageSoundManager.obj"
	-@erase "$(INTDIR)\RageSoundManager.sbr"
	-@erase "$(INTDIR)\RageSoundMixBuffer.obj"
	-@erase "$(INTDIR)\RageSoundMixBuffer.sbr"
	-@erase "$(INTDIR)\RageSoundPosMap.obj"
	-@erase "$(INTDIR)\RageSoundPosMap.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Chain.obj"
	-@erase "$(INTDIR)\RageSoundReader_Chain.sbr"
	-@erase "$(INTDIR)\RageSoundReader_FileReader.obj"
	-@erase "$(INTDIR)\RageSoundReader_FileReader.sbr"
	-@erase "$(INTDIR)\RageSoundReader_MP3.obj"
	-@erase "$(INTDIR)\RageSoundReader_MP3.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Preload.obj"
	-@erase "$(INTDIR)\RageSoundReader_Preload.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Fast.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Fast.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Good.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Good.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Vorbisfile.obj"
	-@erase "$(INTDIR)\RageSoundReader_Vorbisfile.sbr"
	-@erase "$(INTDIR)\RageSoundReader_WAV.obj"
	-@erase "$(INTDIR)\RageSoundReader_WAV.sbr"
	-@erase "$(INTDIR)\RageSoundResampler.obj"
	-@erase "$(INTDIR)\RageSoundResampler.sbr"
	-@erase "$(INTDIR)\RageSoundUtil.obj"
	-@erase "$(INTDIR)\RageSoundUtil.sbr"
	-@erase "$(INTDIR)\RageSurface.obj"
	-@erase "$(INTDIR)\RageSurface.sbr"
	-@erase "$(INTDIR)\RageSurface_Load.obj"
	-@erase "$(INTDIR)\RageSurface_Load.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_BMP.obj"
	-@erase "$(INTDIR)\RageSurface_Load_BMP.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_GIF.obj"
	-@erase "$(INTDIR)\RageSurface_Load_GIF.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_JPEG.obj"
	-@erase "$(INTDIR)\RageSurface_Load_JPEG.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_PNG.obj"
	-@erase "$(INTDIR)\RageSurface_Load_PNG.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_XPM.obj"
	-@erase "$(INTDIR)\RageSurface_Load_XPM.sbr"
	-@erase "$(INTDIR)\RageSurface_Save_BMP.obj"
	-@erase "$(INTDIR)\RageSurface_Save_BMP.sbr"
	-@erase "$(INTDIR)\RageSurface_Save_JPEG.obj"
	-@erase "$(INTDIR)\RageSurface_Save_JPEG.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Dither.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Dither.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Palettize.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Palettize.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Zoom.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Zoom.sbr"
	-@erase "$(INTDIR)\RageTexture.obj"
	-@erase "$(INTDIR)\RageTexture.sbr"
	-@erase "$(INTDIR)\RageTextureID.obj"
	-@erase "$(INTDIR)\RageTextureID.sbr"
	-@erase "$(INTDIR)\RageTextureManager.obj"
	-@erase "$(INTDIR)\RageTextureManager.sbr"
	-@erase "$(INTDIR)\RageThreads.obj"
	-@erase "$(INTDIR)\RageThreads.sbr"
	-@erase "$(INTDIR)\RageTimer.obj"
	-@erase "$(INTDIR)\RageTimer.sbr"
	-@erase "$(INTDIR)\RageUtil.obj"
	-@erase "$(INTDIR)\RageUtil.sbr"
	-@erase "$(INTDIR)\RageUtil_BackgroundLoader.obj"
	-@erase "$(INTDIR)\RageUtil_BackgroundLoader.sbr"
	-@erase "$(INTDIR)\RageUtil_CharConversions.obj"
	-@erase "$(INTDIR)\RageUtil_CharConversions.sbr"
	-@erase "$(INTDIR)\RageUtil_FileDB.obj"
	-@erase "$(INTDIR)\RageUtil_FileDB.sbr"
	-@erase "$(INTDIR)\RageUtil_WorkerThread.obj"
	-@erase "$(INTDIR)\RageUtil_WorkerThread.sbr"
	-@erase "$(INTDIR)\RandomSample.obj"
	-@erase "$(INTDIR)\RandomSample.sbr"
	-@erase "$(INTDIR)\ReceptorArrow.obj"
	-@erase "$(INTDIR)\ReceptorArrow.sbr"
	-@erase "$(INTDIR)\ReceptorArrowRow.obj"
	-@erase "$(INTDIR)\ReceptorArrowRow.sbr"
	-@erase "$(INTDIR)\RegistryAccess.obj"
	-@erase "$(INTDIR)\RegistryAccess.sbr"
	-@erase "$(INTDIR)\RestartProgram.obj"
	-@erase "$(INTDIR)\RestartProgram.sbr"
	-@erase "$(INTDIR)\RollingNumbers.obj"
	-@erase "$(INTDIR)\RollingNumbers.sbr"
	-@erase "$(INTDIR)\RoomWheel.obj"
	-@erase "$(INTDIR)\RoomWheel.sbr"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rsa.sbr"
	-@erase "$(INTDIR)\ScoreDisplay.obj"
	-@erase "$(INTDIR)\ScoreDisplay.sbr"
	-@erase "$(INTDIR)\ScoreDisplayAliveTime.obj"
	-@erase "$(INTDIR)\ScoreDisplayAliveTime.sbr"
	-@erase "$(INTDIR)\ScoreDisplayBattle.obj"
	-@erase "$(INTDIR)\ScoreDisplayBattle.sbr"
	-@erase "$(INTDIR)\ScoreDisplayCalories.obj"
	-@erase "$(INTDIR)\ScoreDisplayCalories.sbr"
	-@erase "$(INTDIR)\ScoreDisplayLifeTime.obj"
	-@erase "$(INTDIR)\ScoreDisplayLifeTime.sbr"
	-@erase "$(INTDIR)\ScoreDisplayNormal.obj"
	-@erase "$(INTDIR)\ScoreDisplayNormal.sbr"
	-@erase "$(INTDIR)\ScoreDisplayOni.obj"
	-@erase "$(INTDIR)\ScoreDisplayOni.sbr"
	-@erase "$(INTDIR)\ScoreDisplayPercentage.obj"
	-@erase "$(INTDIR)\ScoreDisplayPercentage.sbr"
	-@erase "$(INTDIR)\ScoreDisplayRave.obj"
	-@erase "$(INTDIR)\ScoreDisplayRave.sbr"
	-@erase "$(INTDIR)\ScoreKeeperMAX2.obj"
	-@erase "$(INTDIR)\ScoreKeeperMAX2.sbr"
	-@erase "$(INTDIR)\ScoreKeeperRave.obj"
	-@erase "$(INTDIR)\ScoreKeeperRave.sbr"
	-@erase "$(INTDIR)\Screen.obj"
	-@erase "$(INTDIR)\Screen.sbr"
	-@erase "$(INTDIR)\ScreenAttract.obj"
	-@erase "$(INTDIR)\ScreenAttract.sbr"
	-@erase "$(INTDIR)\ScreenBookkeeping.obj"
	-@erase "$(INTDIR)\ScreenBookkeeping.sbr"
	-@erase "$(INTDIR)\ScreenBranch.obj"
	-@erase "$(INTDIR)\ScreenBranch.sbr"
	-@erase "$(INTDIR)\ScreenCenterImage.obj"
	-@erase "$(INTDIR)\ScreenCenterImage.sbr"
	-@erase "$(INTDIR)\ScreenCredits.obj"
	-@erase "$(INTDIR)\ScreenCredits.sbr"
	-@erase "$(INTDIR)\ScreenDebugOverlay.obj"
	-@erase "$(INTDIR)\ScreenDebugOverlay.sbr"
	-@erase "$(INTDIR)\ScreenDemonstration.obj"
	-@erase "$(INTDIR)\ScreenDemonstration.sbr"
	-@erase "$(INTDIR)\ScreenDimensions.obj"
	-@erase "$(INTDIR)\ScreenDimensions.sbr"
	-@erase "$(INTDIR)\ScreenEdit.obj"
	-@erase "$(INTDIR)\ScreenEdit.sbr"
	-@erase "$(INTDIR)\ScreenEditCoursesMenu.obj"
	-@erase "$(INTDIR)\ScreenEditCoursesMenu.sbr"
	-@erase "$(INTDIR)\ScreenEditMenu.obj"
	-@erase "$(INTDIR)\ScreenEditMenu.sbr"
	-@erase "$(INTDIR)\ScreenEnding.obj"
	-@erase "$(INTDIR)\ScreenEnding.sbr"
	-@erase "$(INTDIR)\ScreenEndlessBreak.obj"
	-@erase "$(INTDIR)\ScreenEndlessBreak.sbr"
	-@erase "$(INTDIR)\ScreenEvaluation.obj"
	-@erase "$(INTDIR)\ScreenEvaluation.sbr"
	-@erase "$(INTDIR)\ScreenExit.obj"
	-@erase "$(INTDIR)\ScreenExit.sbr"
	-@erase "$(INTDIR)\ScreenEz2SelectMusic.obj"
	-@erase "$(INTDIR)\ScreenEz2SelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenEz2SelectPlayer.obj"
	-@erase "$(INTDIR)\ScreenEz2SelectPlayer.sbr"
	-@erase "$(INTDIR)\ScreenGameplay.obj"
	-@erase "$(INTDIR)\ScreenGameplay.sbr"
	-@erase "$(INTDIR)\ScreenGameplayMultiplayer.obj"
	-@erase "$(INTDIR)\ScreenGameplayMultiplayer.sbr"
	-@erase "$(INTDIR)\ScreenHowToPlay.obj"
	-@erase "$(INTDIR)\ScreenHowToPlay.sbr"
	-@erase "$(INTDIR)\ScreenInstructions.obj"
	-@erase "$(INTDIR)\ScreenInstructions.sbr"
	-@erase "$(INTDIR)\ScreenJukebox.obj"
	-@erase "$(INTDIR)\ScreenJukebox.sbr"
	-@erase "$(INTDIR)\ScreenLogo.obj"
	-@erase "$(INTDIR)\ScreenLogo.sbr"
	-@erase "$(INTDIR)\ScreenManager.obj"
	-@erase "$(INTDIR)\ScreenManager.sbr"
	-@erase "$(INTDIR)\ScreenMapControllers.obj"
	-@erase "$(INTDIR)\ScreenMapControllers.sbr"
	-@erase "$(INTDIR)\ScreenMessage.obj"
	-@erase "$(INTDIR)\ScreenMessage.sbr"
	-@erase "$(INTDIR)\ScreenMiniMenu.obj"
	-@erase "$(INTDIR)\ScreenMiniMenu.sbr"
	-@erase "$(INTDIR)\ScreenMusicScroll.obj"
	-@erase "$(INTDIR)\ScreenMusicScroll.sbr"
	-@erase "$(INTDIR)\ScreenNameEntry.obj"
	-@erase "$(INTDIR)\ScreenNameEntry.sbr"
	-@erase "$(INTDIR)\ScreenNameEntryTraditional.obj"
	-@erase "$(INTDIR)\ScreenNameEntryTraditional.sbr"
	-@erase "$(INTDIR)\ScreenNetEvaluation.obj"
	-@erase "$(INTDIR)\ScreenNetEvaluation.sbr"
	-@erase "$(INTDIR)\ScreenNetRoom.obj"
	-@erase "$(INTDIR)\ScreenNetRoom.sbr"
	-@erase "$(INTDIR)\ScreenNetSelectBase.obj"
	-@erase "$(INTDIR)\ScreenNetSelectBase.sbr"
	-@erase "$(INTDIR)\ScreenNetSelectMusic.obj"
	-@erase "$(INTDIR)\ScreenNetSelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenNetworkOptions.obj"
	-@erase "$(INTDIR)\ScreenNetworkOptions.sbr"
	-@erase "$(INTDIR)\ScreenOptions.obj"
	-@erase "$(INTDIR)\ScreenOptions.sbr"
	-@erase "$(INTDIR)\ScreenOptionsMaster.obj"
	-@erase "$(INTDIR)\ScreenOptionsMaster.sbr"
	-@erase "$(INTDIR)\ScreenOptionsMasterPrefs.obj"
	-@erase "$(INTDIR)\ScreenOptionsMasterPrefs.sbr"
	-@erase "$(INTDIR)\ScreenPackages.obj"
	-@erase "$(INTDIR)\ScreenPackages.sbr"
	-@erase "$(INTDIR)\ScreenPlayerOptions.obj"
	-@erase "$(INTDIR)\ScreenPlayerOptions.sbr"
	-@erase "$(INTDIR)\ScreenProfileOptions.obj"
	-@erase "$(INTDIR)\ScreenProfileOptions.sbr"
	-@erase "$(INTDIR)\ScreenPrompt.obj"
	-@erase "$(INTDIR)\ScreenPrompt.sbr"
	-@erase "$(INTDIR)\ScreenRanking.obj"
	-@erase "$(INTDIR)\ScreenRanking.sbr"
	-@erase "$(INTDIR)\ScreenReloadSongs.obj"
	-@erase "$(INTDIR)\ScreenReloadSongs.sbr"
	-@erase "$(INTDIR)\ScreenSandbox.obj"
	-@erase "$(INTDIR)\ScreenSandbox.sbr"
	-@erase "$(INTDIR)\ScreenSelect.obj"
	-@erase "$(INTDIR)\ScreenSelect.sbr"
	-@erase "$(INTDIR)\ScreenSelectCharacter.obj"
	-@erase "$(INTDIR)\ScreenSelectCharacter.sbr"
	-@erase "$(INTDIR)\ScreenSelectDifficulty.obj"
	-@erase "$(INTDIR)\ScreenSelectDifficulty.sbr"
	-@erase "$(INTDIR)\ScreenSelectGroup.obj"
	-@erase "$(INTDIR)\ScreenSelectGroup.sbr"
	-@erase "$(INTDIR)\ScreenSelectMaster.obj"
	-@erase "$(INTDIR)\ScreenSelectMaster.sbr"
	-@erase "$(INTDIR)\ScreenSelectMode.obj"
	-@erase "$(INTDIR)\ScreenSelectMode.sbr"
	-@erase "$(INTDIR)\ScreenSelectMusic.obj"
	-@erase "$(INTDIR)\ScreenSelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenSelectStyle.obj"
	-@erase "$(INTDIR)\ScreenSelectStyle.sbr"
	-@erase "$(INTDIR)\ScreenSetTime.obj"
	-@erase "$(INTDIR)\ScreenSetTime.sbr"
	-@erase "$(INTDIR)\ScreenSMOnlineLogin.obj"
	-@erase "$(INTDIR)\ScreenSMOnlineLogin.sbr"
	-@erase "$(INTDIR)\ScreenSongOptions.obj"
	-@erase "$(INTDIR)\ScreenSongOptions.sbr"
	-@erase "$(INTDIR)\ScreenSplash.obj"
	-@erase "$(INTDIR)\ScreenSplash.sbr"
	-@erase "$(INTDIR)\ScreenStage.obj"
	-@erase "$(INTDIR)\ScreenStage.sbr"
	-@erase "$(INTDIR)\ScreenStyleSplash.obj"
	-@erase "$(INTDIR)\ScreenStyleSplash.sbr"
	-@erase "$(INTDIR)\ScreenSystemLayer.obj"
	-@erase "$(INTDIR)\ScreenSystemLayer.sbr"
	-@erase "$(INTDIR)\ScreenTest.obj"
	-@erase "$(INTDIR)\ScreenTest.sbr"
	-@erase "$(INTDIR)\ScreenTestFonts.obj"
	-@erase "$(INTDIR)\ScreenTestFonts.sbr"
	-@erase "$(INTDIR)\ScreenTestInput.obj"
	-@erase "$(INTDIR)\ScreenTestInput.sbr"
	-@erase "$(INTDIR)\ScreenTestLights.obj"
	-@erase "$(INTDIR)\ScreenTestLights.sbr"
	-@erase "$(INTDIR)\ScreenTestSound.obj"
	-@erase "$(INTDIR)\ScreenTestSound.sbr"
	-@erase "$(INTDIR)\ScreenTextEntry.obj"
	-@erase "$(INTDIR)\ScreenTextEntry.sbr"
	-@erase "$(INTDIR)\ScreenTitleMenu.obj"
	-@erase "$(INTDIR)\ScreenTitleMenu.sbr"
	-@erase "$(INTDIR)\ScreenUnlock.obj"
	-@erase "$(INTDIR)\ScreenUnlock.sbr"
	-@erase "$(INTDIR)\ScreenWithMenuElements.obj"
	-@erase "$(INTDIR)\ScreenWithMenuElements.sbr"
	-@erase "$(INTDIR)\ScrollBar.obj"
	-@erase "$(INTDIR)\ScrollBar.sbr"
	-@erase "$(INTDIR)\ScrollingList.obj"
	-@erase "$(INTDIR)\ScrollingList.sbr"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\sha.sbr"
	-@erase "$(INTDIR)\SnapDisplay.obj"
	-@erase "$(INTDIR)\SnapDisplay.sbr"
	-@erase "$(INTDIR)\Song.obj"
	-@erase "$(INTDIR)\Song.sbr"
	-@erase "$(INTDIR)\SongCacheIndex.obj"
	-@erase "$(INTDIR)\SongCacheIndex.sbr"
	-@erase "$(INTDIR)\SongManager.obj"
	-@erase "$(INTDIR)\SongManager.sbr"
	-@erase "$(INTDIR)\SongOptions.obj"
	-@erase "$(INTDIR)\SongOptions.sbr"
	-@erase "$(INTDIR)\SongUtil.obj"
	-@erase "$(INTDIR)\SongUtil.sbr"
	-@erase "$(INTDIR)\Sprite.obj"
	-@erase "$(INTDIR)\Sprite.sbr"
	-@erase "$(INTDIR)\StageStats.obj"
	-@erase "$(INTDIR)\StageStats.sbr"
	-@erase "$(INTDIR)\StatsManager.obj"
	-@erase "$(INTDIR)\StatsManager.sbr"
	-@erase "$(INTDIR)\StepMania.obj"
	-@erase "$(INTDIR)\StepMania.pch"
	-@erase "$(INTDIR)\StepMania.sbr"
	-@erase "$(INTDIR)\Steps.obj"
	-@erase "$(INTDIR)\Steps.sbr"
	-@erase "$(INTDIR)\StepsUtil.obj"
	-@erase "$(INTDIR)\StepsUtil.sbr"
	-@erase "$(INTDIR)\StreamDisplay.obj"
	-@erase "$(INTDIR)\StreamDisplay.sbr"
	-@erase "$(INTDIR)\study.obj"
	-@erase "$(INTDIR)\study.sbr"
	-@erase "$(INTDIR)\Style.obj"
	-@erase "$(INTDIR)\Style.sbr"
	-@erase "$(INTDIR)\StyleUtil.obj"
	-@erase "$(INTDIR)\StyleUtil.sbr"
	-@erase "$(INTDIR)\TextBanner.obj"
	-@erase "$(INTDIR)\TextBanner.sbr"
	-@erase "$(INTDIR)\ThemeManager.obj"
	-@erase "$(INTDIR)\ThemeManager.sbr"
	-@erase "$(INTDIR)\Threads_Win32.obj"
	-@erase "$(INTDIR)\Threads_Win32.sbr"
	-@erase "$(INTDIR)\TimingData.obj"
	-@erase "$(INTDIR)\TimingData.sbr"
	-@erase "$(INTDIR)\TitleSubstitution.obj"
	-@erase "$(INTDIR)\TitleSubstitution.sbr"
	-@erase "$(INTDIR)\Trail.obj"
	-@erase "$(INTDIR)\Trail.sbr"
	-@erase "$(INTDIR)\TrailUtil.obj"
	-@erase "$(INTDIR)\TrailUtil.sbr"
	-@erase "$(INTDIR)\Transition.obj"
	-@erase "$(INTDIR)\Transition.sbr"
	-@erase "$(INTDIR)\UnlockManager.obj"
	-@erase "$(INTDIR)\UnlockManager.sbr"
	-@erase "$(INTDIR)\USB.obj"
	-@erase "$(INTDIR)\USB.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VideoDriverInfo.obj"
	-@erase "$(INTDIR)\VideoDriverInfo.sbr"
	-@erase "$(INTDIR)\WheelBase.obj"
	-@erase "$(INTDIR)\WheelBase.sbr"
	-@erase "$(INTDIR)\WheelItemBase.obj"
	-@erase "$(INTDIR)\WheelItemBase.sbr"
	-@erase "$(INTDIR)\WheelNotifyIcon.obj"
	-@erase "$(INTDIR)\WheelNotifyIcon.sbr"
	-@erase "$(INTDIR)\WindowIcon.obj"
	-@erase "$(INTDIR)\WindowIcon.sbr"
	-@erase "$(INTDIR)\WindowsResources.res"
	-@erase "$(INTDIR)\XmlFile.obj"
	-@erase "$(INTDIR)\XmlFile.sbr"
	-@erase "$(OUTDIR)\StepMania-debug.map"
	-@erase "$(OUTDIR)\StepMania-debug.pdb"
	-@erase "$(OUTDIR)\StepMania.bsc"
	-@erase "..\Program\StepMania-debug.exe"
	-@erase "..\Program\StepMania-debug.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX"global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WindowsResources.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\StepMania.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\get.sbr" \
	"$(INTDIR)\maketables.sbr" \
	"$(INTDIR)\pcre.sbr" \
	"$(INTDIR)\study.sbr" \
	"$(INTDIR)\RageBitmapTexture.sbr" \
	"$(INTDIR)\RageDisplay.sbr" \
	"$(INTDIR)\RageDisplay_D3D.sbr" \
	"$(INTDIR)\RageDisplay_Null.sbr" \
	"$(INTDIR)\RageDisplay_OGL.sbr" \
	"$(INTDIR)\RageDisplay_OGL_Extensions.sbr" \
	"$(INTDIR)\RageException.sbr" \
	"$(INTDIR)\RageFile.sbr" \
	"$(INTDIR)\RageFileBasic.sbr" \
	"$(INTDIR)\RageFileDriver.sbr" \
	"$(INTDIR)\RageFileDriverDeflate.sbr" \
	"$(INTDIR)\RageFileDriverDirect.sbr" \
	"$(INTDIR)\RageFileDriverDirectHelpers.sbr" \
	"$(INTDIR)\RageFileDriverMemory.sbr" \
	"$(INTDIR)\RageFileDriverSlice.sbr" \
	"$(INTDIR)\RageFileDriverTimeout.sbr" \
	"$(INTDIR)\RageFileDriverZip.sbr" \
	"$(INTDIR)\RageFileManager.sbr" \
	"$(INTDIR)\RageInput.sbr" \
	"$(INTDIR)\RageInputDevice.sbr" \
	"$(INTDIR)\RageLog.sbr" \
	"$(INTDIR)\RageMath.sbr" \
	"$(INTDIR)\RageModelGeometry.sbr" \
	"$(INTDIR)\RageSound.sbr" \
	"$(INTDIR)\RageSoundManager.sbr" \
	"$(INTDIR)\RageSoundMixBuffer.sbr" \
	"$(INTDIR)\RageSoundPosMap.sbr" \
	"$(INTDIR)\RageSoundReader_Chain.sbr" \
	"$(INTDIR)\RageSoundReader_FileReader.sbr" \
	"$(INTDIR)\RageSoundReader_MP3.sbr" \
	"$(INTDIR)\RageSoundReader_Preload.sbr" \
	"$(INTDIR)\RageSoundReader_Resample.sbr" \
	"$(INTDIR)\RageSoundReader_Resample_Fast.sbr" \
	"$(INTDIR)\RageSoundReader_Resample_Good.sbr" \
	"$(INTDIR)\RageSoundReader_Vorbisfile.sbr" \
	"$(INTDIR)\RageSoundReader_WAV.sbr" \
	"$(INTDIR)\RageSoundResampler.sbr" \
	"$(INTDIR)\RageSoundUtil.sbr" \
	"$(INTDIR)\RageSurface.sbr" \
	"$(INTDIR)\RageSurface_Load.sbr" \
	"$(INTDIR)\RageSurface_Load_BMP.sbr" \
	"$(INTDIR)\RageSurface_Load_GIF.sbr" \
	"$(INTDIR)\RageSurface_Load_JPEG.sbr" \
	"$(INTDIR)\RageSurface_Load_PNG.sbr" \
	"$(INTDIR)\RageSurface_Load_XPM.sbr" \
	"$(INTDIR)\RageSurface_Save_BMP.sbr" \
	"$(INTDIR)\RageSurface_Save_JPEG.sbr" \
	"$(INTDIR)\RageSurfaceUtils.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Dither.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Palettize.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Zoom.sbr" \
	"$(INTDIR)\RageTexture.sbr" \
	"$(INTDIR)\RageTextureID.sbr" \
	"$(INTDIR)\RageTextureManager.sbr" \
	"$(INTDIR)\RageThreads.sbr" \
	"$(INTDIR)\RageTimer.sbr" \
	"$(INTDIR)\RageUtil.sbr" \
	"$(INTDIR)\RageUtil_BackgroundLoader.sbr" \
	"$(INTDIR)\RageUtil_CharConversions.sbr" \
	"$(INTDIR)\RageUtil_FileDB.sbr" \
	"$(INTDIR)\RageUtil_WorkerThread.sbr" \
	"$(INTDIR)\ActorCommands.sbr" \
	"$(INTDIR)\Attack.sbr" \
	"$(INTDIR)\AutoKeysounds.sbr" \
	"$(INTDIR)\BannerCache.sbr" \
	"$(INTDIR)\CatalogXml.sbr" \
	"$(INTDIR)\Character.sbr" \
	"$(INTDIR)\CodeDetector.sbr" \
	"$(INTDIR)\Command.sbr" \
	"$(INTDIR)\CommonMetrics.sbr" \
	"$(INTDIR)\Course.sbr" \
	"$(INTDIR)\CourseUtil.sbr" \
	"$(INTDIR)\DateTime.sbr" \
	"$(INTDIR)\Difficulty.sbr" \
	"$(INTDIR)\EnumHelper.sbr" \
	"$(INTDIR)\Font.sbr" \
	"$(INTDIR)\FontCharAliases.sbr" \
	"$(INTDIR)\FontCharmaps.sbr" \
	"$(INTDIR)\Game.sbr" \
	"$(INTDIR)\GameCommand.sbr" \
	"$(INTDIR)\GameConstantsAndTypes.sbr" \
	"$(INTDIR)\GameInput.sbr" \
	"$(INTDIR)\Grade.sbr" \
	"$(INTDIR)\HighScore.sbr" \
	"$(INTDIR)\Inventory.sbr" \
	"$(INTDIR)\LuaReference.sbr" \
	"$(INTDIR)\LyricsLoader.sbr" \
	"$(INTDIR)\NoteData.sbr" \
	"$(INTDIR)\NoteDataUtil.sbr" \
	"$(INTDIR)\NoteDataWithScoring.sbr" \
	"$(INTDIR)\NoteFieldPositioning.sbr" \
	"$(INTDIR)\NotesLoader.sbr" \
	"$(INTDIR)\NotesLoaderBMS.sbr" \
	"$(INTDIR)\NotesLoaderDWI.sbr" \
	"$(INTDIR)\NotesLoaderKSF.sbr" \
	"$(INTDIR)\NotesLoaderSM.sbr" \
	"$(INTDIR)\NotesWriterDWI.sbr" \
	"$(INTDIR)\NotesWriterSM.sbr" \
	"$(INTDIR)\NoteTypes.sbr" \
	"$(INTDIR)\OptionRowHandler.sbr" \
	"$(INTDIR)\PlayerAI.sbr" \
	"$(INTDIR)\PlayerNumber.sbr" \
	"$(INTDIR)\PlayerOptions.sbr" \
	"$(INTDIR)\PlayerStageStats.sbr" \
	"$(INTDIR)\PlayerState.sbr" \
	"$(INTDIR)\Preference.sbr" \
	"$(INTDIR)\Profile.sbr" \
	"$(INTDIR)\RadarValues.sbr" \
	"$(INTDIR)\RandomSample.sbr" \
	"$(INTDIR)\ScoreKeeperMAX2.sbr" \
	"$(INTDIR)\ScoreKeeperRave.sbr" \
	"$(INTDIR)\ScreenDimensions.sbr" \
	"$(INTDIR)\Song.sbr" \
	"$(INTDIR)\SongCacheIndex.sbr" \
	"$(INTDIR)\SongOptions.sbr" \
	"$(INTDIR)\SongUtil.sbr" \
	"$(INTDIR)\StageStats.sbr" \
	"$(INTDIR)\Steps.sbr" \
	"$(INTDIR)\StepsUtil.sbr" \
	"$(INTDIR)\Style.sbr" \
	"$(INTDIR)\StyleUtil.sbr" \
	"$(INTDIR)\TimingData.sbr" \
	"$(INTDIR)\TitleSubstitution.sbr" \
	"$(INTDIR)\Trail.sbr" \
	"$(INTDIR)\TrailUtil.sbr" \
	"$(INTDIR)\IniFile.sbr" \
	"$(INTDIR)\MsdFile.sbr" \
	"$(INTDIR)\XmlFile.sbr" \
	"$(INTDIR)\LoadingWindow_Win32.sbr" \
	"$(INTDIR)\DSoundHelpers.sbr" \
	"$(INTDIR)\RageSoundDriver_DSound.sbr" \
	"$(INTDIR)\RageSoundDriver_DSound_Software.sbr" \
	"$(INTDIR)\RageSoundDriver_Generic_Software.sbr" \
	"$(INTDIR)\RageSoundDriver_Null.sbr" \
	"$(INTDIR)\RageSoundDriver_WaveOut.sbr" \
	"$(INTDIR)\ArchHooks.sbr" \
	"$(INTDIR)\ArchHooks_Win32.sbr" \
	"$(INTDIR)\InputHandler.sbr" \
	"$(INTDIR)\InputHandler_DirectInput.sbr" \
	"$(INTDIR)\InputHandler_DirectInputHelper.sbr" \
	"$(INTDIR)\InputHandler_MonkeyKeyboard.sbr" \
	"$(INTDIR)\InputHandler_Win32_Para.sbr" \
	"$(INTDIR)\InputHandler_Win32_Pump.sbr" \
	"$(INTDIR)\MovieTexture.sbr" \
	"$(INTDIR)\MovieTexture_DShow.sbr" \
	"$(INTDIR)\MovieTexture_DShowHelper.sbr" \
	"$(INTDIR)\MovieTexture_FFMpeg.sbr" \
	"$(INTDIR)\MovieTexture_Null.sbr" \
	"$(INTDIR)\LowLevelWindow_Win32.sbr" \
	"$(INTDIR)\LightsDriver_SystemMessage.sbr" \
	"$(INTDIR)\LightsDriver_Win32Parallel.sbr" \
	"$(INTDIR)\MemoryCardDriver.sbr" \
	"$(INTDIR)\MemoryCardDriverThreaded_Windows.sbr" \
	"$(INTDIR)\Dialog.sbr" \
	"$(INTDIR)\DialogDriver_Win32.sbr" \
	"$(INTDIR)\Threads_Win32.sbr" \
	"$(INTDIR)\arch.sbr" \
	"$(INTDIR)\AppInstance.sbr" \
	"$(INTDIR)\arch_setup.sbr" \
	"$(INTDIR)\Crash.sbr" \
	"$(INTDIR)\DebugInfoHunt.sbr" \
	"$(INTDIR)\GetFileInformation.sbr" \
	"$(INTDIR)\GotoURL.sbr" \
	"$(INTDIR)\GraphicsWindow.sbr" \
	"$(INTDIR)\RegistryAccess.sbr" \
	"$(INTDIR)\RestartProgram.sbr" \
	"$(INTDIR)\USB.sbr" \
	"$(INTDIR)\VideoDriverInfo.sbr" \
	"$(INTDIR)\WindowIcon.sbr" \
	"$(INTDIR)\global.sbr" \
	"$(INTDIR)\StepMania.sbr" \
	"$(INTDIR)\Actor.sbr" \
	"$(INTDIR)\ActorFrame.sbr" \
	"$(INTDIR)\ActorScroller.sbr" \
	"$(INTDIR)\ActorUtil.sbr" \
	"$(INTDIR)\AutoActor.sbr" \
	"$(INTDIR)\BitmapText.sbr" \
	"$(INTDIR)\Model.sbr" \
	"$(INTDIR)\ModelManager.sbr" \
	"$(INTDIR)\ModelTypes.sbr" \
	"$(INTDIR)\Quad.sbr" \
	"$(INTDIR)\RollingNumbers.sbr" \
	"$(INTDIR)\Sprite.sbr" \
	"$(INTDIR)\Banner.sbr" \
	"$(INTDIR)\BGAnimation.sbr" \
	"$(INTDIR)\BGAnimationLayer.sbr" \
	"$(INTDIR)\DifficultyIcon.sbr" \
	"$(INTDIR)\FadingBanner.sbr" \
	"$(INTDIR)\MeterDisplay.sbr" \
	"$(INTDIR)\StreamDisplay.sbr" \
	"$(INTDIR)\Transition.sbr" \
	"$(INTDIR)\ActiveAttackList.sbr" \
	"$(INTDIR)\ArrowEffects.sbr" \
	"$(INTDIR)\AttackDisplay.sbr" \
	"$(INTDIR)\Background.sbr" \
	"$(INTDIR)\BeginnerHelper.sbr" \
	"$(INTDIR)\CharacterHead.sbr" \
	"$(INTDIR)\CombinedLifeMeterTug.sbr" \
	"$(INTDIR)\Combo.sbr" \
	"$(INTDIR)\DancingCharacters.sbr" \
	"$(INTDIR)\Foreground.sbr" \
	"$(INTDIR)\GhostArrow.sbr" \
	"$(INTDIR)\GhostArrowRow.sbr" \
	"$(INTDIR)\HoldGhostArrow.sbr" \
	"$(INTDIR)\HoldJudgment.sbr" \
	"$(INTDIR)\Judgment.sbr" \
	"$(INTDIR)\LifeMeterBar.sbr" \
	"$(INTDIR)\LifeMeterBattery.sbr" \
	"$(INTDIR)\LifeMeterTime.sbr" \
	"$(INTDIR)\LyricDisplay.sbr" \
	"$(INTDIR)\NoteDisplay.sbr" \
	"$(INTDIR)\NoteField.sbr" \
	"$(INTDIR)\PercentageDisplay.sbr" \
	"$(INTDIR)\Player.sbr" \
	"$(INTDIR)\ReceptorArrow.sbr" \
	"$(INTDIR)\ReceptorArrowRow.sbr" \
	"$(INTDIR)\ScoreDisplay.sbr" \
	"$(INTDIR)\ScoreDisplayAliveTime.sbr" \
	"$(INTDIR)\ScoreDisplayBattle.sbr" \
	"$(INTDIR)\ScoreDisplayCalories.sbr" \
	"$(INTDIR)\ScoreDisplayLifeTime.sbr" \
	"$(INTDIR)\ScoreDisplayNormal.sbr" \
	"$(INTDIR)\ScoreDisplayOni.sbr" \
	"$(INTDIR)\ScoreDisplayPercentage.sbr" \
	"$(INTDIR)\ScoreDisplayRave.sbr" \
	"$(INTDIR)\BPMDisplay.sbr" \
	"$(INTDIR)\ComboGraph.sbr" \
	"$(INTDIR)\CourseContentsList.sbr" \
	"$(INTDIR)\CourseEntryDisplay.sbr" \
	"$(INTDIR)\DifficultyDisplay.sbr" \
	"$(INTDIR)\DifficultyList.sbr" \
	"$(INTDIR)\DifficultyMeter.sbr" \
	"$(INTDIR)\DifficultyRating.sbr" \
	"$(INTDIR)\DualScrollBar.sbr" \
	"$(INTDIR)\EditCoursesMenu.sbr" \
	"$(INTDIR)\EditCoursesSongMenu.sbr" \
	"$(INTDIR)\EditMenu.sbr" \
	"$(INTDIR)\GradeDisplay.sbr" \
	"$(INTDIR)\GraphDisplay.sbr" \
	"$(INTDIR)\GrooveGraph.sbr" \
	"$(INTDIR)\GrooveRadar.sbr" \
	"$(INTDIR)\GroupList.sbr" \
	"$(INTDIR)\HelpDisplay.sbr" \
	"$(INTDIR)\MemoryCardDisplay.sbr" \
	"$(INTDIR)\MenuTimer.sbr" \
	"$(INTDIR)\ModeSwitcher.sbr" \
	"$(INTDIR)\MusicBannerWheel.sbr" \
	"$(INTDIR)\MusicList.sbr" \
	"$(INTDIR)\MusicSortDisplay.sbr" \
	"$(INTDIR)\MusicWheel.sbr" \
	"$(INTDIR)\MusicWheelItem.sbr" \
	"$(INTDIR)\OptionIcon.sbr" \
	"$(INTDIR)\OptionIconRow.sbr" \
	"$(INTDIR)\OptionRow.sbr" \
	"$(INTDIR)\OptionsCursor.sbr" \
	"$(INTDIR)\PaneDisplay.sbr" \
	"$(INTDIR)\RoomWheel.sbr" \
	"$(INTDIR)\ScrollBar.sbr" \
	"$(INTDIR)\ScrollingList.sbr" \
	"$(INTDIR)\SnapDisplay.sbr" \
	"$(INTDIR)\TextBanner.sbr" \
	"$(INTDIR)\WheelBase.sbr" \
	"$(INTDIR)\WheelItemBase.sbr" \
	"$(INTDIR)\WheelNotifyIcon.sbr" \
	"$(INTDIR)\Screen.sbr" \
	"$(INTDIR)\ScreenAttract.sbr" \
	"$(INTDIR)\ScreenBookkeeping.sbr" \
	"$(INTDIR)\ScreenBranch.sbr" \
	"$(INTDIR)\ScreenCenterImage.sbr" \
	"$(INTDIR)\ScreenCredits.sbr" \
	"$(INTDIR)\ScreenDebugOverlay.sbr" \
	"$(INTDIR)\ScreenDemonstration.sbr" \
	"$(INTDIR)\ScreenEdit.sbr" \
	"$(INTDIR)\ScreenEditCoursesMenu.sbr" \
	"$(INTDIR)\ScreenEditMenu.sbr" \
	"$(INTDIR)\ScreenEnding.sbr" \
	"$(INTDIR)\ScreenEndlessBreak.sbr" \
	"$(INTDIR)\ScreenEvaluation.sbr" \
	"$(INTDIR)\ScreenExit.sbr" \
	"$(INTDIR)\ScreenEz2SelectMusic.sbr" \
	"$(INTDIR)\ScreenEz2SelectPlayer.sbr" \
	"$(INTDIR)\ScreenGameplay.sbr" \
	"$(INTDIR)\ScreenGameplayMultiplayer.sbr" \
	"$(INTDIR)\ScreenHowToPlay.sbr" \
	"$(INTDIR)\ScreenInstructions.sbr" \
	"$(INTDIR)\ScreenJukebox.sbr" \
	"$(INTDIR)\ScreenLogo.sbr" \
	"$(INTDIR)\ScreenMapControllers.sbr" \
	"$(INTDIR)\ScreenMessage.sbr" \
	"$(INTDIR)\ScreenMiniMenu.sbr" \
	"$(INTDIR)\ScreenMusicScroll.sbr" \
	"$(INTDIR)\ScreenNameEntry.sbr" \
	"$(INTDIR)\ScreenNameEntryTraditional.sbr" \
	"$(INTDIR)\ScreenNetEvaluation.sbr" \
	"$(INTDIR)\ScreenNetRoom.sbr" \
	"$(INTDIR)\ScreenNetSelectBase.sbr" \
	"$(INTDIR)\ScreenNetSelectMusic.sbr" \
	"$(INTDIR)\ScreenNetworkOptions.sbr" \
	"$(INTDIR)\ScreenOptions.sbr" \
	"$(INTDIR)\ScreenOptionsMaster.sbr" \
	"$(INTDIR)\ScreenOptionsMasterPrefs.sbr" \
	"$(INTDIR)\ScreenPackages.sbr" \
	"$(INTDIR)\ScreenPlayerOptions.sbr" \
	"$(INTDIR)\ScreenProfileOptions.sbr" \
	"$(INTDIR)\ScreenPrompt.sbr" \
	"$(INTDIR)\ScreenRanking.sbr" \
	"$(INTDIR)\ScreenReloadSongs.sbr" \
	"$(INTDIR)\ScreenSandbox.sbr" \
	"$(INTDIR)\ScreenSelect.sbr" \
	"$(INTDIR)\ScreenSelectCharacter.sbr" \
	"$(INTDIR)\ScreenSelectDifficulty.sbr" \
	"$(INTDIR)\ScreenSelectGroup.sbr" \
	"$(INTDIR)\ScreenSelectMaster.sbr" \
	"$(INTDIR)\ScreenSelectMode.sbr" \
	"$(INTDIR)\ScreenSelectMusic.sbr" \
	"$(INTDIR)\ScreenSelectStyle.sbr" \
	"$(INTDIR)\ScreenSetTime.sbr" \
	"$(INTDIR)\ScreenSMOnlineLogin.sbr" \
	"$(INTDIR)\ScreenSongOptions.sbr" \
	"$(INTDIR)\ScreenSplash.sbr" \
	"$(INTDIR)\ScreenStage.sbr" \
	"$(INTDIR)\ScreenStyleSplash.sbr" \
	"$(INTDIR)\ScreenSystemLayer.sbr" \
	"$(INTDIR)\ScreenTest.sbr" \
	"$(INTDIR)\ScreenTestFonts.sbr" \
	"$(INTDIR)\ScreenTestInput.sbr" \
	"$(INTDIR)\ScreenTestLights.sbr" \
	"$(INTDIR)\ScreenTestSound.sbr" \
	"$(INTDIR)\ScreenTextEntry.sbr" \
	"$(INTDIR)\ScreenTitleMenu.sbr" \
	"$(INTDIR)\ScreenUnlock.sbr" \
	"$(INTDIR)\ScreenWithMenuElements.sbr" \
	"$(INTDIR)\AnnouncerManager.sbr" \
	"$(INTDIR)\Bookkeeper.sbr" \
	"$(INTDIR)\CryptManager.sbr" \
	"$(INTDIR)\ezsockets.sbr" \
	"$(INTDIR)\FontManager.sbr" \
	"$(INTDIR)\GameManager.sbr" \
	"$(INTDIR)\GameSoundManager.sbr" \
	"$(INTDIR)\GameState.sbr" \
	"$(INTDIR)\InputFilter.sbr" \
	"$(INTDIR)\InputMapper.sbr" \
	"$(INTDIR)\InputQueue.sbr" \
	"$(INTDIR)\LightsManager.sbr" \
	"$(INTDIR)\LuaManager.sbr" \
	"$(INTDIR)\MemoryCardManager.sbr" \
	"$(INTDIR)\MessageManager.sbr" \
	"$(INTDIR)\NetworkSyncManager.sbr" \
	"$(INTDIR)\NetworkSyncServer.sbr" \
	"$(INTDIR)\NoteSkinManager.sbr" \
	"$(INTDIR)\PrefsManager.sbr" \
	"$(INTDIR)\ProfileManager.sbr" \
	"$(INTDIR)\ScreenManager.sbr" \
	"$(INTDIR)\SongManager.sbr" \
	"$(INTDIR)\StatsManager.sbr" \
	"$(INTDIR)\ThemeManager.sbr" \
	"$(INTDIR)\UnlockManager.sbr" \
	"$(INTDIR)\algebra.sbr" \
	"$(INTDIR)\algparam.sbr" \
	"$(INTDIR)\asn.sbr" \
	"$(INTDIR)\cryptlib.sbr" \
	"$(INTDIR)\files.sbr" \
	"$(INTDIR)\filters.sbr" \
	"$(INTDIR)\integer.sbr" \
	"$(INTDIR)\iterhash.sbr" \
	"$(INTDIR)\misc.sbr" \
	"$(INTDIR)\mqueue.sbr" \
	"$(INTDIR)\nbtheory.sbr" \
	"$(INTDIR)\osrng.sbr" \
	"$(INTDIR)\pkcspad.sbr" \
	"$(INTDIR)\pubkey.sbr" \
	"$(INTDIR)\queue.sbr" \
	"$(INTDIR)\rsa.sbr" \
	"$(INTDIR)\sha.sbr" \
	"$(INTDIR)\CryptBn.sbr" \
	"$(INTDIR)\CryptHelpers.sbr" \
	"$(INTDIR)\CryptMD5.sbr" \
	"$(INTDIR)\CryptNoise.sbr" \
	"$(INTDIR)\CryptPrime.sbr" \
	"$(INTDIR)\CryptRand.sbr" \
	"$(INTDIR)\CryptRSA.sbr" \
	"$(INTDIR)\CryptSH512.sbr" \
	"$(INTDIR)\CryptSHA.sbr"

"$(OUTDIR)\StepMania.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\StepMania-debug.pdb" /map:"$(INTDIR)\StepMania-debug.map" /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /out:"../Program/StepMania-debug.exe" 
LINK32_OBJS= \
	"$(INTDIR)\get.obj" \
	"$(INTDIR)\maketables.obj" \
	"$(INTDIR)\pcre.obj" \
	"$(INTDIR)\study.obj" \
	"$(INTDIR)\RageBitmapTexture.obj" \
	"$(INTDIR)\RageDisplay.obj" \
	"$(INTDIR)\RageDisplay_D3D.obj" \
	"$(INTDIR)\RageDisplay_Null.obj" \
	"$(INTDIR)\RageDisplay_OGL.obj" \
	"$(INTDIR)\RageDisplay_OGL_Extensions.obj" \
	"$(INTDIR)\RageException.obj" \
	"$(INTDIR)\RageFile.obj" \
	"$(INTDIR)\RageFileBasic.obj" \
	"$(INTDIR)\RageFileDriver.obj" \
	"$(INTDIR)\RageFileDriverDeflate.obj" \
	"$(INTDIR)\RageFileDriverDirect.obj" \
	"$(INTDIR)\RageFileDriverDirectHelpers.obj" \
	"$(INTDIR)\RageFileDriverMemory.obj" \
	"$(INTDIR)\RageFileDriverSlice.obj" \
	"$(INTDIR)\RageFileDriverTimeout.obj" \
	"$(INTDIR)\RageFileDriverZip.obj" \
	"$(INTDIR)\RageFileManager.obj" \
	"$(INTDIR)\RageInput.obj" \
	"$(INTDIR)\RageInputDevice.obj" \
	"$(INTDIR)\RageLog.obj" \
	"$(INTDIR)\RageMath.obj" \
	"$(INTDIR)\RageModelGeometry.obj" \
	"$(INTDIR)\RageSound.obj" \
	"$(INTDIR)\RageSoundManager.obj" \
	"$(INTDIR)\RageSoundMixBuffer.obj" \
	"$(INTDIR)\RageSoundPosMap.obj" \
	"$(INTDIR)\RageSoundReader_Chain.obj" \
	"$(INTDIR)\RageSoundReader_FileReader.obj" \
	"$(INTDIR)\RageSoundReader_MP3.obj" \
	"$(INTDIR)\RageSoundReader_Preload.obj" \
	"$(INTDIR)\RageSoundReader_Resample.obj" \
	"$(INTDIR)\RageSoundReader_Resample_Fast.obj" \
	"$(INTDIR)\RageSoundReader_Resample_Good.obj" \
	"$(INTDIR)\RageSoundReader_Vorbisfile.obj" \
	"$(INTDIR)\RageSoundReader_WAV.obj" \
	"$(INTDIR)\RageSoundResampler.obj" \
	"$(INTDIR)\RageSoundUtil.obj" \
	"$(INTDIR)\RageSurface.obj" \
	"$(INTDIR)\RageSurface_Load.obj" \
	"$(INTDIR)\RageSurface_Load_BMP.obj" \
	"$(INTDIR)\RageSurface_Load_GIF.obj" \
	"$(INTDIR)\RageSurface_Load_JPEG.obj" \
	"$(INTDIR)\RageSurface_Load_PNG.obj" \
	"$(INTDIR)\RageSurface_Load_XPM.obj" \
	"$(INTDIR)\RageSurface_Save_BMP.obj" \
	"$(INTDIR)\RageSurface_Save_JPEG.obj" \
	"$(INTDIR)\RageSurfaceUtils.obj" \
	"$(INTDIR)\RageSurfaceUtils_Dither.obj" \
	"$(INTDIR)\RageSurfaceUtils_Palettize.obj" \
	"$(INTDIR)\RageSurfaceUtils_Zoom.obj" \
	"$(INTDIR)\RageTexture.obj" \
	"$(INTDIR)\RageTextureID.obj" \
	"$(INTDIR)\RageTextureManager.obj" \
	"$(INTDIR)\RageThreads.obj" \
	"$(INTDIR)\RageTimer.obj" \
	"$(INTDIR)\RageUtil.obj" \
	"$(INTDIR)\RageUtil_BackgroundLoader.obj" \
	"$(INTDIR)\RageUtil_CharConversions.obj" \
	"$(INTDIR)\RageUtil_FileDB.obj" \
	"$(INTDIR)\RageUtil_WorkerThread.obj" \
	"$(INTDIR)\ActorCommands.obj" \
	"$(INTDIR)\Attack.obj" \
	"$(INTDIR)\AutoKeysounds.obj" \
	"$(INTDIR)\BannerCache.obj" \
	"$(INTDIR)\CatalogXml.obj" \
	"$(INTDIR)\Character.obj" \
	"$(INTDIR)\CodeDetector.obj" \
	"$(INTDIR)\Command.obj" \
	"$(INTDIR)\CommonMetrics.obj" \
	"$(INTDIR)\Course.obj" \
	"$(INTDIR)\CourseUtil.obj" \
	"$(INTDIR)\DateTime.obj" \
	"$(INTDIR)\Difficulty.obj" \
	"$(INTDIR)\EnumHelper.obj" \
	"$(INTDIR)\Font.obj" \
	"$(INTDIR)\FontCharAliases.obj" \
	"$(INTDIR)\FontCharmaps.obj" \
	"$(INTDIR)\Game.obj" \
	"$(INTDIR)\GameCommand.obj" \
	"$(INTDIR)\GameConstantsAndTypes.obj" \
	"$(INTDIR)\GameInput.obj" \
	"$(INTDIR)\Grade.obj" \
	"$(INTDIR)\HighScore.obj" \
	"$(INTDIR)\Inventory.obj" \
	"$(INTDIR)\LuaReference.obj" \
	"$(INTDIR)\LyricsLoader.obj" \
	"$(INTDIR)\NoteData.obj" \
	"$(INTDIR)\NoteDataUtil.obj" \
	"$(INTDIR)\NoteDataWithScoring.obj" \
	"$(INTDIR)\NoteFieldPositioning.obj" \
	"$(INTDIR)\NotesLoader.obj" \
	"$(INTDIR)\NotesLoaderBMS.obj" \
	"$(INTDIR)\NotesLoaderDWI.obj" \
	"$(INTDIR)\NotesLoaderKSF.obj" \
	"$(INTDIR)\NotesLoaderSM.obj" \
	"$(INTDIR)\NotesWriterDWI.obj" \
	"$(INTDIR)\NotesWriterSM.obj" \
	"$(INTDIR)\NoteTypes.obj" \
	"$(INTDIR)\OptionRowHandler.obj" \
	"$(INTDIR)\PlayerAI.obj" \
	"$(INTDIR)\PlayerNumber.obj" \
	"$(INTDIR)\PlayerOptions.obj" \
	"$(INTDIR)\PlayerStageStats.obj" \
	"$(INTDIR)\PlayerState.obj" \
	"$(INTDIR)\Preference.obj" \
	"$(INTDIR)\Profile.obj" \
	"$(INTDIR)\RadarValues.obj" \
	"$(INTDIR)\RandomSample.obj" \
	"$(INTDIR)\ScoreKeeperMAX2.obj" \
	"$(INTDIR)\ScoreKeeperRave.obj" \
	"$(INTDIR)\ScreenDimensions.obj" \
	"$(INTDIR)\Song.obj" \
	"$(INTDIR)\SongCacheIndex.obj" \
	"$(INTDIR)\SongOptions.obj" \
	"$(INTDIR)\SongUtil.obj" \
	"$(INTDIR)\StageStats.obj" \
	"$(INTDIR)\Steps.obj" \
	"$(INTDIR)\StepsUtil.obj" \
	"$(INTDIR)\Style.obj" \
	"$(INTDIR)\StyleUtil.obj" \
	"$(INTDIR)\TimingData.obj" \
	"$(INTDIR)\TitleSubstitution.obj" \
	"$(INTDIR)\Trail.obj" \
	"$(INTDIR)\TrailUtil.obj" \
	"$(INTDIR)\IniFile.obj" \
	"$(INTDIR)\MsdFile.obj" \
	"$(INTDIR)\XmlFile.obj" \
	"$(INTDIR)\LoadingWindow_Win32.obj" \
	"$(INTDIR)\DSoundHelpers.obj" \
	"$(INTDIR)\RageSoundDriver_DSound.obj" \
	"$(INTDIR)\RageSoundDriver_DSound_Software.obj" \
	"$(INTDIR)\RageSoundDriver_Generic_Software.obj" \
	"$(INTDIR)\RageSoundDriver_Null.obj" \
	"$(INTDIR)\RageSoundDriver_WaveOut.obj" \
	"$(INTDIR)\ArchHooks.obj" \
	"$(INTDIR)\ArchHooks_Win32.obj" \
	"$(INTDIR)\InputHandler.obj" \
	"$(INTDIR)\InputHandler_DirectInput.obj" \
	"$(INTDIR)\InputHandler_DirectInputHelper.obj" \
	"$(INTDIR)\InputHandler_MonkeyKeyboard.obj" \
	"$(INTDIR)\InputHandler_Win32_Para.obj" \
	"$(INTDIR)\InputHandler_Win32_Pump.obj" \
	"$(INTDIR)\MovieTexture.obj" \
	"$(INTDIR)\MovieTexture_DShow.obj" \
	"$(INTDIR)\MovieTexture_DShowHelper.obj" \
	"$(INTDIR)\MovieTexture_FFMpeg.obj" \
	"$(INTDIR)\MovieTexture_Null.obj" \
	"$(INTDIR)\LowLevelWindow_Win32.obj" \
	"$(INTDIR)\LightsDriver_SystemMessage.obj" \
	"$(INTDIR)\LightsDriver_Win32Parallel.obj" \
	"$(INTDIR)\MemoryCardDriver.obj" \
	"$(INTDIR)\MemoryCardDriverThreaded_Windows.obj" \
	"$(INTDIR)\Dialog.obj" \
	"$(INTDIR)\DialogDriver_Win32.obj" \
	"$(INTDIR)\Threads_Win32.obj" \
	"$(INTDIR)\arch.obj" \
	"$(INTDIR)\AppInstance.obj" \
	"$(INTDIR)\arch_setup.obj" \
	"$(INTDIR)\Crash.obj" \
	"$(INTDIR)\DebugInfoHunt.obj" \
	"$(INTDIR)\GetFileInformation.obj" \
	"$(INTDIR)\GotoURL.obj" \
	"$(INTDIR)\GraphicsWindow.obj" \
	"$(INTDIR)\RegistryAccess.obj" \
	"$(INTDIR)\RestartProgram.obj" \
	"$(INTDIR)\USB.obj" \
	"$(INTDIR)\VideoDriverInfo.obj" \
	"$(INTDIR)\WindowIcon.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\StepMania.obj" \
	"$(INTDIR)\Actor.obj" \
	"$(INTDIR)\ActorFrame.obj" \
	"$(INTDIR)\ActorScroller.obj" \
	"$(INTDIR)\ActorUtil.obj" \
	"$(INTDIR)\AutoActor.obj" \
	"$(INTDIR)\BitmapText.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\ModelManager.obj" \
	"$(INTDIR)\ModelTypes.obj" \
	"$(INTDIR)\Quad.obj" \
	"$(INTDIR)\RollingNumbers.obj" \
	"$(INTDIR)\Sprite.obj" \
	"$(INTDIR)\Banner.obj" \
	"$(INTDIR)\BGAnimation.obj" \
	"$(INTDIR)\BGAnimationLayer.obj" \
	"$(INTDIR)\DifficultyIcon.obj" \
	"$(INTDIR)\FadingBanner.obj" \
	"$(INTDIR)\MeterDisplay.obj" \
	"$(INTDIR)\StreamDisplay.obj" \
	"$(INTDIR)\Transition.obj" \
	"$(INTDIR)\ActiveAttackList.obj" \
	"$(INTDIR)\ArrowEffects.obj" \
	"$(INTDIR)\AttackDisplay.obj" \
	"$(INTDIR)\Background.obj" \
	"$(INTDIR)\BeginnerHelper.obj" \
	"$(INTDIR)\CharacterHead.obj" \
	"$(INTDIR)\CombinedLifeMeterTug.obj" \
	"$(INTDIR)\Combo.obj" \
	"$(INTDIR)\DancingCharacters.obj" \
	"$(INTDIR)\Foreground.obj" \
	"$(INTDIR)\GhostArrow.obj" \
	"$(INTDIR)\GhostArrowRow.obj" \
	"$(INTDIR)\HoldGhostArrow.obj" \
	"$(INTDIR)\HoldJudgment.obj" \
	"$(INTDIR)\Judgment.obj" \
	"$(INTDIR)\LifeMeterBar.obj" \
	"$(INTDIR)\LifeMeterBattery.obj" \
	"$(INTDIR)\LifeMeterTime.obj" \
	"$(INTDIR)\LyricDisplay.obj" \
	"$(INTDIR)\NoteDisplay.obj" \
	"$(INTDIR)\NoteField.obj" \
	"$(INTDIR)\PercentageDisplay.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\ReceptorArrow.obj" \
	"$(INTDIR)\ReceptorArrowRow.obj" \
	"$(INTDIR)\ScoreDisplay.obj" \
	"$(INTDIR)\ScoreDisplayAliveTime.obj" \
	"$(INTDIR)\ScoreDisplayBattle.obj" \
	"$(INTDIR)\ScoreDisplayCalories.obj" \
	"$(INTDIR)\ScoreDisplayLifeTime.obj" \
	"$(INTDIR)\ScoreDisplayNormal.obj" \
	"$(INTDIR)\ScoreDisplayOni.obj" \
	"$(INTDIR)\ScoreDisplayPercentage.obj" \
	"$(INTDIR)\ScoreDisplayRave.obj" \
	"$(INTDIR)\BPMDisplay.obj" \
	"$(INTDIR)\ComboGraph.obj" \
	"$(INTDIR)\CourseContentsList.obj" \
	"$(INTDIR)\CourseEntryDisplay.obj" \
	"$(INTDIR)\DifficultyDisplay.obj" \
	"$(INTDIR)\DifficultyList.obj" \
	"$(INTDIR)\DifficultyMeter.obj" \
	"$(INTDIR)\DifficultyRating.obj" \
	"$(INTDIR)\DualScrollBar.obj" \
	"$(INTDIR)\EditCoursesMenu.obj" \
	"$(INTDIR)\EditCoursesSongMenu.obj" \
	"$(INTDIR)\EditMenu.obj" \
	"$(INTDIR)\GradeDisplay.obj" \
	"$(INTDIR)\GraphDisplay.obj" \
	"$(INTDIR)\GrooveGraph.obj" \
	"$(INTDIR)\GrooveRadar.obj" \
	"$(INTDIR)\GroupList.obj" \
	"$(INTDIR)\HelpDisplay.obj" \
	"$(INTDIR)\MemoryCardDisplay.obj" \
	"$(INTDIR)\MenuTimer.obj" \
	"$(INTDIR)\ModeSwitcher.obj" \
	"$(INTDIR)\MusicBannerWheel.obj" \
	"$(INTDIR)\MusicList.obj" \
	"$(INTDIR)\MusicSortDisplay.obj" \
	"$(INTDIR)\MusicWheel.obj" \
	"$(INTDIR)\MusicWheelItem.obj" \
	"$(INTDIR)\OptionIcon.obj" \
	"$(INTDIR)\OptionIconRow.obj" \
	"$(INTDIR)\OptionRow.obj" \
	"$(INTDIR)\OptionsCursor.obj" \
	"$(INTDIR)\PaneDisplay.obj" \
	"$(INTDIR)\RoomWheel.obj" \
	"$(INTDIR)\ScrollBar.obj" \
	"$(INTDIR)\ScrollingList.obj" \
	"$(INTDIR)\SnapDisplay.obj" \
	"$(INTDIR)\TextBanner.obj" \
	"$(INTDIR)\WheelBase.obj" \
	"$(INTDIR)\WheelItemBase.obj" \
	"$(INTDIR)\WheelNotifyIcon.obj" \
	"$(INTDIR)\Screen.obj" \
	"$(INTDIR)\ScreenAttract.obj" \
	"$(INTDIR)\ScreenBookkeeping.obj" \
	"$(INTDIR)\ScreenBranch.obj" \
	"$(INTDIR)\ScreenCenterImage.obj" \
	"$(INTDIR)\ScreenCredits.obj" \
	"$(INTDIR)\ScreenDebugOverlay.obj" \
	"$(INTDIR)\ScreenDemonstration.obj" \
	"$(INTDIR)\ScreenEdit.obj" \
	"$(INTDIR)\ScreenEditCoursesMenu.obj" \
	"$(INTDIR)\ScreenEditMenu.obj" \
	"$(INTDIR)\ScreenEnding.obj" \
	"$(INTDIR)\ScreenEndlessBreak.obj" \
	"$(INTDIR)\ScreenEvaluation.obj" \
	"$(INTDIR)\ScreenExit.obj" \
	"$(INTDIR)\ScreenEz2SelectMusic.obj" \
	"$(INTDIR)\ScreenEz2SelectPlayer.obj" \
	"$(INTDIR)\ScreenGameplay.obj" \
	"$(INTDIR)\ScreenGameplayMultiplayer.obj" \
	"$(INTDIR)\ScreenHowToPlay.obj" \
	"$(INTDIR)\ScreenInstructions.obj" \
	"$(INTDIR)\ScreenJukebox.obj" \
	"$(INTDIR)\ScreenLogo.obj" \
	"$(INTDIR)\ScreenMapControllers.obj" \
	"$(INTDIR)\ScreenMessage.obj" \
	"$(INTDIR)\ScreenMiniMenu.obj" \
	"$(INTDIR)\ScreenMusicScroll.obj" \
	"$(INTDIR)\ScreenNameEntry.obj" \
	"$(INTDIR)\ScreenNameEntryTraditional.obj" \
	"$(INTDIR)\ScreenNetEvaluation.obj" \
	"$(INTDIR)\ScreenNetRoom.obj" \
	"$(INTDIR)\ScreenNetSelectBase.obj" \
	"$(INTDIR)\ScreenNetSelectMusic.obj" \
	"$(INTDIR)\ScreenNetworkOptions.obj" \
	"$(INTDIR)\ScreenOptions.obj" \
	"$(INTDIR)\ScreenOptionsMaster.obj" \
	"$(INTDIR)\ScreenOptionsMasterPrefs.obj" \
	"$(INTDIR)\ScreenPackages.obj" \
	"$(INTDIR)\ScreenPlayerOptions.obj" \
	"$(INTDIR)\ScreenProfileOptions.obj" \
	"$(INTDIR)\ScreenPrompt.obj" \
	"$(INTDIR)\ScreenRanking.obj" \
	"$(INTDIR)\ScreenReloadSongs.obj" \
	"$(INTDIR)\ScreenSandbox.obj" \
	"$(INTDIR)\ScreenSelect.obj" \
	"$(INTDIR)\ScreenSelectCharacter.obj" \
	"$(INTDIR)\ScreenSelectDifficulty.obj" \
	"$(INTDIR)\ScreenSelectGroup.obj" \
	"$(INTDIR)\ScreenSelectMaster.obj" \
	"$(INTDIR)\ScreenSelectMode.obj" \
	"$(INTDIR)\ScreenSelectMusic.obj" \
	"$(INTDIR)\ScreenSelectStyle.obj" \
	"$(INTDIR)\ScreenSetTime.obj" \
	"$(INTDIR)\ScreenSMOnlineLogin.obj" \
	"$(INTDIR)\ScreenSongOptions.obj" \
	"$(INTDIR)\ScreenSplash.obj" \
	"$(INTDIR)\ScreenStage.obj" \
	"$(INTDIR)\ScreenStyleSplash.obj" \
	"$(INTDIR)\ScreenSystemLayer.obj" \
	"$(INTDIR)\ScreenTest.obj" \
	"$(INTDIR)\ScreenTestFonts.obj" \
	"$(INTDIR)\ScreenTestInput.obj" \
	"$(INTDIR)\ScreenTestLights.obj" \
	"$(INTDIR)\ScreenTestSound.obj" \
	"$(INTDIR)\ScreenTextEntry.obj" \
	"$(INTDIR)\ScreenTitleMenu.obj" \
	"$(INTDIR)\ScreenUnlock.obj" \
	"$(INTDIR)\ScreenWithMenuElements.obj" \
	"$(INTDIR)\AnnouncerManager.obj" \
	"$(INTDIR)\Bookkeeper.obj" \
	"$(INTDIR)\CryptManager.obj" \
	"$(INTDIR)\ezsockets.obj" \
	"$(INTDIR)\FontManager.obj" \
	"$(INTDIR)\GameManager.obj" \
	"$(INTDIR)\GameSoundManager.obj" \
	"$(INTDIR)\GameState.obj" \
	"$(INTDIR)\InputFilter.obj" \
	"$(INTDIR)\InputMapper.obj" \
	"$(INTDIR)\InputQueue.obj" \
	"$(INTDIR)\LightsManager.obj" \
	"$(INTDIR)\LuaManager.obj" \
	"$(INTDIR)\MemoryCardManager.obj" \
	"$(INTDIR)\MessageManager.obj" \
	"$(INTDIR)\NetworkSyncManager.obj" \
	"$(INTDIR)\NetworkSyncServer.obj" \
	"$(INTDIR)\NoteSkinManager.obj" \
	"$(INTDIR)\PrefsManager.obj" \
	"$(INTDIR)\ProfileManager.obj" \
	"$(INTDIR)\ScreenManager.obj" \
	"$(INTDIR)\SongManager.obj" \
	"$(INTDIR)\StatsManager.obj" \
	"$(INTDIR)\ThemeManager.obj" \
	"$(INTDIR)\UnlockManager.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\CryptBn.obj" \
	"$(INTDIR)\CryptHelpers.obj" \
	"$(INTDIR)\CryptMD5.obj" \
	"$(INTDIR)\CryptNoise.obj" \
	"$(INTDIR)\CryptPrime.obj" \
	"$(INTDIR)\CryptRand.obj" \
	"$(INTDIR)\CryptRSA.obj" \
	"$(INTDIR)\CryptSH512.obj" \
	"$(INTDIR)\CryptSHA.obj" \
	"$(INTDIR)\WindowsResources.res"

"..\Program\StepMania-debug.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   archutils\Win32\verinc
	cl /Zl /nologo /c verstub.cpp  /Fo.\..\Debug6\


	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

IntDir=.\../Debug6
TargetDir=\stepmania\Program
TargetName=StepMania-debug
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

# Begin Custom Macros
OutDir=.\../Debug6
# End Custom Macros

$(DS_POSTBUILD_DEP) : "..\Program\StepMania-debug.exe" "$(OUTDIR)\StepMania.pch" "$(OUTDIR)\StepMania.bsc"
   archutils\Win32\mapconv .\../Debug6\StepMania-debug.map \stepmania\Program\StepMania.vdi
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

OUTDIR=.\..\Release6
INTDIR=.\..\Release6

ALL : "..\Program\StepMania.exe" "..\StepMania-StackTrace.bsc"


CLEAN :
	-@erase "$(INTDIR)\ActiveAttackList.obj"
	-@erase "$(INTDIR)\ActiveAttackList.sbr"
	-@erase "$(INTDIR)\Actor.obj"
	-@erase "$(INTDIR)\Actor.sbr"
	-@erase "$(INTDIR)\ActorCommands.obj"
	-@erase "$(INTDIR)\ActorCommands.sbr"
	-@erase "$(INTDIR)\ActorFrame.obj"
	-@erase "$(INTDIR)\ActorFrame.sbr"
	-@erase "$(INTDIR)\ActorScroller.obj"
	-@erase "$(INTDIR)\ActorScroller.sbr"
	-@erase "$(INTDIR)\ActorUtil.obj"
	-@erase "$(INTDIR)\ActorUtil.sbr"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algebra.sbr"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\algparam.sbr"
	-@erase "$(INTDIR)\AnnouncerManager.obj"
	-@erase "$(INTDIR)\AnnouncerManager.sbr"
	-@erase "$(INTDIR)\AppInstance.obj"
	-@erase "$(INTDIR)\AppInstance.sbr"
	-@erase "$(INTDIR)\arch.obj"
	-@erase "$(INTDIR)\arch.sbr"
	-@erase "$(INTDIR)\arch_setup.obj"
	-@erase "$(INTDIR)\arch_setup.sbr"
	-@erase "$(INTDIR)\ArchHooks.obj"
	-@erase "$(INTDIR)\ArchHooks.sbr"
	-@erase "$(INTDIR)\ArchHooks_Win32.obj"
	-@erase "$(INTDIR)\ArchHooks_Win32.sbr"
	-@erase "$(INTDIR)\ArrowEffects.obj"
	-@erase "$(INTDIR)\ArrowEffects.sbr"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\asn.sbr"
	-@erase "$(INTDIR)\Attack.obj"
	-@erase "$(INTDIR)\Attack.sbr"
	-@erase "$(INTDIR)\AttackDisplay.obj"
	-@erase "$(INTDIR)\AttackDisplay.sbr"
	-@erase "$(INTDIR)\AutoActor.obj"
	-@erase "$(INTDIR)\AutoActor.sbr"
	-@erase "$(INTDIR)\AutoKeysounds.obj"
	-@erase "$(INTDIR)\AutoKeysounds.sbr"
	-@erase "$(INTDIR)\Background.obj"
	-@erase "$(INTDIR)\Background.sbr"
	-@erase "$(INTDIR)\Banner.obj"
	-@erase "$(INTDIR)\Banner.sbr"
	-@erase "$(INTDIR)\BannerCache.obj"
	-@erase "$(INTDIR)\BannerCache.sbr"
	-@erase "$(INTDIR)\BeginnerHelper.obj"
	-@erase "$(INTDIR)\BeginnerHelper.sbr"
	-@erase "$(INTDIR)\BGAnimation.obj"
	-@erase "$(INTDIR)\BGAnimation.sbr"
	-@erase "$(INTDIR)\BGAnimationLayer.obj"
	-@erase "$(INTDIR)\BGAnimationLayer.sbr"
	-@erase "$(INTDIR)\BitmapText.obj"
	-@erase "$(INTDIR)\BitmapText.sbr"
	-@erase "$(INTDIR)\Bookkeeper.obj"
	-@erase "$(INTDIR)\Bookkeeper.sbr"
	-@erase "$(INTDIR)\BPMDisplay.obj"
	-@erase "$(INTDIR)\BPMDisplay.sbr"
	-@erase "$(INTDIR)\CatalogXml.obj"
	-@erase "$(INTDIR)\CatalogXml.sbr"
	-@erase "$(INTDIR)\Character.obj"
	-@erase "$(INTDIR)\Character.sbr"
	-@erase "$(INTDIR)\CharacterHead.obj"
	-@erase "$(INTDIR)\CharacterHead.sbr"
	-@erase "$(INTDIR)\CodeDetector.obj"
	-@erase "$(INTDIR)\CodeDetector.sbr"
	-@erase "$(INTDIR)\CombinedLifeMeterTug.obj"
	-@erase "$(INTDIR)\CombinedLifeMeterTug.sbr"
	-@erase "$(INTDIR)\Combo.obj"
	-@erase "$(INTDIR)\Combo.sbr"
	-@erase "$(INTDIR)\ComboGraph.obj"
	-@erase "$(INTDIR)\ComboGraph.sbr"
	-@erase "$(INTDIR)\Command.obj"
	-@erase "$(INTDIR)\Command.sbr"
	-@erase "$(INTDIR)\CommonMetrics.obj"
	-@erase "$(INTDIR)\CommonMetrics.sbr"
	-@erase "$(INTDIR)\Course.obj"
	-@erase "$(INTDIR)\Course.sbr"
	-@erase "$(INTDIR)\CourseContentsList.obj"
	-@erase "$(INTDIR)\CourseContentsList.sbr"
	-@erase "$(INTDIR)\CourseEntryDisplay.obj"
	-@erase "$(INTDIR)\CourseEntryDisplay.sbr"
	-@erase "$(INTDIR)\CourseUtil.obj"
	-@erase "$(INTDIR)\CourseUtil.sbr"
	-@erase "$(INTDIR)\Crash.obj"
	-@erase "$(INTDIR)\Crash.sbr"
	-@erase "$(INTDIR)\CryptBn.obj"
	-@erase "$(INTDIR)\CryptBn.sbr"
	-@erase "$(INTDIR)\CryptHelpers.obj"
	-@erase "$(INTDIR)\CryptHelpers.sbr"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.sbr"
	-@erase "$(INTDIR)\CryptManager.obj"
	-@erase "$(INTDIR)\CryptManager.sbr"
	-@erase "$(INTDIR)\CryptMD5.obj"
	-@erase "$(INTDIR)\CryptMD5.sbr"
	-@erase "$(INTDIR)\CryptNoise.obj"
	-@erase "$(INTDIR)\CryptNoise.sbr"
	-@erase "$(INTDIR)\CryptPrime.obj"
	-@erase "$(INTDIR)\CryptPrime.sbr"
	-@erase "$(INTDIR)\CryptRand.obj"
	-@erase "$(INTDIR)\CryptRand.sbr"
	-@erase "$(INTDIR)\CryptRSA.obj"
	-@erase "$(INTDIR)\CryptRSA.sbr"
	-@erase "$(INTDIR)\CryptSH512.obj"
	-@erase "$(INTDIR)\CryptSH512.sbr"
	-@erase "$(INTDIR)\CryptSHA.obj"
	-@erase "$(INTDIR)\CryptSHA.sbr"
	-@erase "$(INTDIR)\DancingCharacters.obj"
	-@erase "$(INTDIR)\DancingCharacters.sbr"
	-@erase "$(INTDIR)\DateTime.obj"
	-@erase "$(INTDIR)\DateTime.sbr"
	-@erase "$(INTDIR)\DebugInfoHunt.obj"
	-@erase "$(INTDIR)\DebugInfoHunt.sbr"
	-@erase "$(INTDIR)\Dialog.obj"
	-@erase "$(INTDIR)\Dialog.sbr"
	-@erase "$(INTDIR)\DialogDriver_Win32.obj"
	-@erase "$(INTDIR)\DialogDriver_Win32.sbr"
	-@erase "$(INTDIR)\Difficulty.obj"
	-@erase "$(INTDIR)\Difficulty.sbr"
	-@erase "$(INTDIR)\DifficultyDisplay.obj"
	-@erase "$(INTDIR)\DifficultyDisplay.sbr"
	-@erase "$(INTDIR)\DifficultyIcon.obj"
	-@erase "$(INTDIR)\DifficultyIcon.sbr"
	-@erase "$(INTDIR)\DifficultyList.obj"
	-@erase "$(INTDIR)\DifficultyList.sbr"
	-@erase "$(INTDIR)\DifficultyMeter.obj"
	-@erase "$(INTDIR)\DifficultyMeter.sbr"
	-@erase "$(INTDIR)\DifficultyRating.obj"
	-@erase "$(INTDIR)\DifficultyRating.sbr"
	-@erase "$(INTDIR)\DSoundHelpers.obj"
	-@erase "$(INTDIR)\DSoundHelpers.sbr"
	-@erase "$(INTDIR)\DualScrollBar.obj"
	-@erase "$(INTDIR)\DualScrollBar.sbr"
	-@erase "$(INTDIR)\EditCoursesMenu.obj"
	-@erase "$(INTDIR)\EditCoursesMenu.sbr"
	-@erase "$(INTDIR)\EditCoursesSongMenu.obj"
	-@erase "$(INTDIR)\EditCoursesSongMenu.sbr"
	-@erase "$(INTDIR)\EditMenu.obj"
	-@erase "$(INTDIR)\EditMenu.sbr"
	-@erase "$(INTDIR)\EnumHelper.obj"
	-@erase "$(INTDIR)\EnumHelper.sbr"
	-@erase "$(INTDIR)\ezsockets.obj"
	-@erase "$(INTDIR)\ezsockets.sbr"
	-@erase "$(INTDIR)\FadingBanner.obj"
	-@erase "$(INTDIR)\FadingBanner.sbr"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\files.sbr"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\filters.sbr"
	-@erase "$(INTDIR)\Font.obj"
	-@erase "$(INTDIR)\Font.sbr"
	-@erase "$(INTDIR)\FontCharAliases.obj"
	-@erase "$(INTDIR)\FontCharAliases.sbr"
	-@erase "$(INTDIR)\FontCharmaps.obj"
	-@erase "$(INTDIR)\FontCharmaps.sbr"
	-@erase "$(INTDIR)\FontManager.obj"
	-@erase "$(INTDIR)\FontManager.sbr"
	-@erase "$(INTDIR)\Foreground.obj"
	-@erase "$(INTDIR)\Foreground.sbr"
	-@erase "$(INTDIR)\Game.obj"
	-@erase "$(INTDIR)\Game.sbr"
	-@erase "$(INTDIR)\GameCommand.obj"
	-@erase "$(INTDIR)\GameCommand.sbr"
	-@erase "$(INTDIR)\GameConstantsAndTypes.obj"
	-@erase "$(INTDIR)\GameConstantsAndTypes.sbr"
	-@erase "$(INTDIR)\GameInput.obj"
	-@erase "$(INTDIR)\GameInput.sbr"
	-@erase "$(INTDIR)\GameManager.obj"
	-@erase "$(INTDIR)\GameManager.sbr"
	-@erase "$(INTDIR)\GameSoundManager.obj"
	-@erase "$(INTDIR)\GameSoundManager.sbr"
	-@erase "$(INTDIR)\GameState.obj"
	-@erase "$(INTDIR)\GameState.sbr"
	-@erase "$(INTDIR)\get.obj"
	-@erase "$(INTDIR)\get.sbr"
	-@erase "$(INTDIR)\GetFileInformation.obj"
	-@erase "$(INTDIR)\GetFileInformation.sbr"
	-@erase "$(INTDIR)\GhostArrow.obj"
	-@erase "$(INTDIR)\GhostArrow.sbr"
	-@erase "$(INTDIR)\GhostArrowRow.obj"
	-@erase "$(INTDIR)\GhostArrowRow.sbr"
	-@erase "$(INTDIR)\global.obj"
	-@erase "$(INTDIR)\global.sbr"
	-@erase "$(INTDIR)\GotoURL.obj"
	-@erase "$(INTDIR)\GotoURL.sbr"
	-@erase "$(INTDIR)\Grade.obj"
	-@erase "$(INTDIR)\Grade.sbr"
	-@erase "$(INTDIR)\GradeDisplay.obj"
	-@erase "$(INTDIR)\GradeDisplay.sbr"
	-@erase "$(INTDIR)\GraphDisplay.obj"
	-@erase "$(INTDIR)\GraphDisplay.sbr"
	-@erase "$(INTDIR)\GraphicsWindow.obj"
	-@erase "$(INTDIR)\GraphicsWindow.sbr"
	-@erase "$(INTDIR)\GrooveGraph.obj"
	-@erase "$(INTDIR)\GrooveGraph.sbr"
	-@erase "$(INTDIR)\GrooveRadar.obj"
	-@erase "$(INTDIR)\GrooveRadar.sbr"
	-@erase "$(INTDIR)\GroupList.obj"
	-@erase "$(INTDIR)\GroupList.sbr"
	-@erase "$(INTDIR)\HelpDisplay.obj"
	-@erase "$(INTDIR)\HelpDisplay.sbr"
	-@erase "$(INTDIR)\HighScore.obj"
	-@erase "$(INTDIR)\HighScore.sbr"
	-@erase "$(INTDIR)\HoldGhostArrow.obj"
	-@erase "$(INTDIR)\HoldGhostArrow.sbr"
	-@erase "$(INTDIR)\HoldJudgment.obj"
	-@erase "$(INTDIR)\HoldJudgment.sbr"
	-@erase "$(INTDIR)\IniFile.obj"
	-@erase "$(INTDIR)\IniFile.sbr"
	-@erase "$(INTDIR)\InputFilter.obj"
	-@erase "$(INTDIR)\InputFilter.sbr"
	-@erase "$(INTDIR)\InputHandler.obj"
	-@erase "$(INTDIR)\InputHandler.sbr"
	-@erase "$(INTDIR)\InputHandler_DirectInput.obj"
	-@erase "$(INTDIR)\InputHandler_DirectInput.sbr"
	-@erase "$(INTDIR)\InputHandler_DirectInputHelper.obj"
	-@erase "$(INTDIR)\InputHandler_DirectInputHelper.sbr"
	-@erase "$(INTDIR)\InputHandler_MonkeyKeyboard.obj"
	-@erase "$(INTDIR)\InputHandler_MonkeyKeyboard.sbr"
	-@erase "$(INTDIR)\InputHandler_Win32_Para.obj"
	-@erase "$(INTDIR)\InputHandler_Win32_Para.sbr"
	-@erase "$(INTDIR)\InputHandler_Win32_Pump.obj"
	-@erase "$(INTDIR)\InputHandler_Win32_Pump.sbr"
	-@erase "$(INTDIR)\InputMapper.obj"
	-@erase "$(INTDIR)\InputMapper.sbr"
	-@erase "$(INTDIR)\InputQueue.obj"
	-@erase "$(INTDIR)\InputQueue.sbr"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\integer.sbr"
	-@erase "$(INTDIR)\Inventory.obj"
	-@erase "$(INTDIR)\Inventory.sbr"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\iterhash.sbr"
	-@erase "$(INTDIR)\Judgment.obj"
	-@erase "$(INTDIR)\Judgment.sbr"
	-@erase "$(INTDIR)\LifeMeterBar.obj"
	-@erase "$(INTDIR)\LifeMeterBar.sbr"
	-@erase "$(INTDIR)\LifeMeterBattery.obj"
	-@erase "$(INTDIR)\LifeMeterBattery.sbr"
	-@erase "$(INTDIR)\LifeMeterTime.obj"
	-@erase "$(INTDIR)\LifeMeterTime.sbr"
	-@erase "$(INTDIR)\LightsDriver_SystemMessage.obj"
	-@erase "$(INTDIR)\LightsDriver_SystemMessage.sbr"
	-@erase "$(INTDIR)\LightsDriver_Win32Parallel.obj"
	-@erase "$(INTDIR)\LightsDriver_Win32Parallel.sbr"
	-@erase "$(INTDIR)\LightsManager.obj"
	-@erase "$(INTDIR)\LightsManager.sbr"
	-@erase "$(INTDIR)\LoadingWindow_Win32.obj"
	-@erase "$(INTDIR)\LoadingWindow_Win32.sbr"
	-@erase "$(INTDIR)\LowLevelWindow_Win32.obj"
	-@erase "$(INTDIR)\LowLevelWindow_Win32.sbr"
	-@erase "$(INTDIR)\LuaManager.obj"
	-@erase "$(INTDIR)\LuaManager.sbr"
	-@erase "$(INTDIR)\LuaReference.obj"
	-@erase "$(INTDIR)\LuaReference.sbr"
	-@erase "$(INTDIR)\LyricDisplay.obj"
	-@erase "$(INTDIR)\LyricDisplay.sbr"
	-@erase "$(INTDIR)\LyricsLoader.obj"
	-@erase "$(INTDIR)\LyricsLoader.sbr"
	-@erase "$(INTDIR)\maketables.obj"
	-@erase "$(INTDIR)\maketables.sbr"
	-@erase "$(INTDIR)\MemoryCardDisplay.obj"
	-@erase "$(INTDIR)\MemoryCardDisplay.sbr"
	-@erase "$(INTDIR)\MemoryCardDriver.obj"
	-@erase "$(INTDIR)\MemoryCardDriver.sbr"
	-@erase "$(INTDIR)\MemoryCardDriverThreaded_Windows.obj"
	-@erase "$(INTDIR)\MemoryCardDriverThreaded_Windows.sbr"
	-@erase "$(INTDIR)\MemoryCardManager.obj"
	-@erase "$(INTDIR)\MemoryCardManager.sbr"
	-@erase "$(INTDIR)\MenuTimer.obj"
	-@erase "$(INTDIR)\MenuTimer.sbr"
	-@erase "$(INTDIR)\MessageManager.obj"
	-@erase "$(INTDIR)\MessageManager.sbr"
	-@erase "$(INTDIR)\MeterDisplay.obj"
	-@erase "$(INTDIR)\MeterDisplay.sbr"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\misc.sbr"
	-@erase "$(INTDIR)\Model.obj"
	-@erase "$(INTDIR)\Model.sbr"
	-@erase "$(INTDIR)\ModelManager.obj"
	-@erase "$(INTDIR)\ModelManager.sbr"
	-@erase "$(INTDIR)\ModelTypes.obj"
	-@erase "$(INTDIR)\ModelTypes.sbr"
	-@erase "$(INTDIR)\ModeSwitcher.obj"
	-@erase "$(INTDIR)\ModeSwitcher.sbr"
	-@erase "$(INTDIR)\MovieTexture.obj"
	-@erase "$(INTDIR)\MovieTexture.sbr"
	-@erase "$(INTDIR)\MovieTexture_DShow.obj"
	-@erase "$(INTDIR)\MovieTexture_DShow.sbr"
	-@erase "$(INTDIR)\MovieTexture_DShowHelper.obj"
	-@erase "$(INTDIR)\MovieTexture_DShowHelper.sbr"
	-@erase "$(INTDIR)\MovieTexture_FFMpeg.obj"
	-@erase "$(INTDIR)\MovieTexture_FFMpeg.sbr"
	-@erase "$(INTDIR)\MovieTexture_Null.obj"
	-@erase "$(INTDIR)\MovieTexture_Null.sbr"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqueue.sbr"
	-@erase "$(INTDIR)\MsdFile.obj"
	-@erase "$(INTDIR)\MsdFile.sbr"
	-@erase "$(INTDIR)\MusicBannerWheel.obj"
	-@erase "$(INTDIR)\MusicBannerWheel.sbr"
	-@erase "$(INTDIR)\MusicList.obj"
	-@erase "$(INTDIR)\MusicList.sbr"
	-@erase "$(INTDIR)\MusicSortDisplay.obj"
	-@erase "$(INTDIR)\MusicSortDisplay.sbr"
	-@erase "$(INTDIR)\MusicWheel.obj"
	-@erase "$(INTDIR)\MusicWheel.sbr"
	-@erase "$(INTDIR)\MusicWheelItem.obj"
	-@erase "$(INTDIR)\MusicWheelItem.sbr"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\nbtheory.sbr"
	-@erase "$(INTDIR)\NetworkSyncManager.obj"
	-@erase "$(INTDIR)\NetworkSyncManager.sbr"
	-@erase "$(INTDIR)\NetworkSyncServer.obj"
	-@erase "$(INTDIR)\NetworkSyncServer.sbr"
	-@erase "$(INTDIR)\NoteData.obj"
	-@erase "$(INTDIR)\NoteData.sbr"
	-@erase "$(INTDIR)\NoteDataUtil.obj"
	-@erase "$(INTDIR)\NoteDataUtil.sbr"
	-@erase "$(INTDIR)\NoteDataWithScoring.obj"
	-@erase "$(INTDIR)\NoteDataWithScoring.sbr"
	-@erase "$(INTDIR)\NoteDisplay.obj"
	-@erase "$(INTDIR)\NoteDisplay.sbr"
	-@erase "$(INTDIR)\NoteField.obj"
	-@erase "$(INTDIR)\NoteField.sbr"
	-@erase "$(INTDIR)\NoteFieldPositioning.obj"
	-@erase "$(INTDIR)\NoteFieldPositioning.sbr"
	-@erase "$(INTDIR)\NoteSkinManager.obj"
	-@erase "$(INTDIR)\NoteSkinManager.sbr"
	-@erase "$(INTDIR)\NotesLoader.obj"
	-@erase "$(INTDIR)\NotesLoader.sbr"
	-@erase "$(INTDIR)\NotesLoaderBMS.obj"
	-@erase "$(INTDIR)\NotesLoaderBMS.sbr"
	-@erase "$(INTDIR)\NotesLoaderDWI.obj"
	-@erase "$(INTDIR)\NotesLoaderDWI.sbr"
	-@erase "$(INTDIR)\NotesLoaderKSF.obj"
	-@erase "$(INTDIR)\NotesLoaderKSF.sbr"
	-@erase "$(INTDIR)\NotesLoaderSM.obj"
	-@erase "$(INTDIR)\NotesLoaderSM.sbr"
	-@erase "$(INTDIR)\NotesWriterDWI.obj"
	-@erase "$(INTDIR)\NotesWriterDWI.sbr"
	-@erase "$(INTDIR)\NotesWriterSM.obj"
	-@erase "$(INTDIR)\NotesWriterSM.sbr"
	-@erase "$(INTDIR)\NoteTypes.obj"
	-@erase "$(INTDIR)\NoteTypes.sbr"
	-@erase "$(INTDIR)\OptionIcon.obj"
	-@erase "$(INTDIR)\OptionIcon.sbr"
	-@erase "$(INTDIR)\OptionIconRow.obj"
	-@erase "$(INTDIR)\OptionIconRow.sbr"
	-@erase "$(INTDIR)\OptionRow.obj"
	-@erase "$(INTDIR)\OptionRow.sbr"
	-@erase "$(INTDIR)\OptionRowHandler.obj"
	-@erase "$(INTDIR)\OptionRowHandler.sbr"
	-@erase "$(INTDIR)\OptionsCursor.obj"
	-@erase "$(INTDIR)\OptionsCursor.sbr"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\osrng.sbr"
	-@erase "$(INTDIR)\PaneDisplay.obj"
	-@erase "$(INTDIR)\PaneDisplay.sbr"
	-@erase "$(INTDIR)\pcre.obj"
	-@erase "$(INTDIR)\pcre.sbr"
	-@erase "$(INTDIR)\PercentageDisplay.obj"
	-@erase "$(INTDIR)\PercentageDisplay.sbr"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\pkcspad.sbr"
	-@erase "$(INTDIR)\Player.obj"
	-@erase "$(INTDIR)\Player.sbr"
	-@erase "$(INTDIR)\PlayerAI.obj"
	-@erase "$(INTDIR)\PlayerAI.sbr"
	-@erase "$(INTDIR)\PlayerNumber.obj"
	-@erase "$(INTDIR)\PlayerNumber.sbr"
	-@erase "$(INTDIR)\PlayerOptions.obj"
	-@erase "$(INTDIR)\PlayerOptions.sbr"
	-@erase "$(INTDIR)\PlayerStageStats.obj"
	-@erase "$(INTDIR)\PlayerStageStats.sbr"
	-@erase "$(INTDIR)\PlayerState.obj"
	-@erase "$(INTDIR)\PlayerState.sbr"
	-@erase "$(INTDIR)\Preference.obj"
	-@erase "$(INTDIR)\Preference.sbr"
	-@erase "$(INTDIR)\PrefsManager.obj"
	-@erase "$(INTDIR)\PrefsManager.sbr"
	-@erase "$(INTDIR)\Profile.obj"
	-@erase "$(INTDIR)\Profile.sbr"
	-@erase "$(INTDIR)\ProfileManager.obj"
	-@erase "$(INTDIR)\ProfileManager.sbr"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\pubkey.sbr"
	-@erase "$(INTDIR)\Quad.obj"
	-@erase "$(INTDIR)\Quad.sbr"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\queue.sbr"
	-@erase "$(INTDIR)\RadarValues.obj"
	-@erase "$(INTDIR)\RadarValues.sbr"
	-@erase "$(INTDIR)\RageBitmapTexture.obj"
	-@erase "$(INTDIR)\RageBitmapTexture.sbr"
	-@erase "$(INTDIR)\RageDisplay.obj"
	-@erase "$(INTDIR)\RageDisplay.sbr"
	-@erase "$(INTDIR)\RageDisplay_D3D.obj"
	-@erase "$(INTDIR)\RageDisplay_D3D.sbr"
	-@erase "$(INTDIR)\RageDisplay_Null.obj"
	-@erase "$(INTDIR)\RageDisplay_Null.sbr"
	-@erase "$(INTDIR)\RageDisplay_OGL.obj"
	-@erase "$(INTDIR)\RageDisplay_OGL.sbr"
	-@erase "$(INTDIR)\RageDisplay_OGL_Extensions.obj"
	-@erase "$(INTDIR)\RageDisplay_OGL_Extensions.sbr"
	-@erase "$(INTDIR)\RageException.obj"
	-@erase "$(INTDIR)\RageException.sbr"
	-@erase "$(INTDIR)\RageFile.obj"
	-@erase "$(INTDIR)\RageFile.sbr"
	-@erase "$(INTDIR)\RageFileBasic.obj"
	-@erase "$(INTDIR)\RageFileBasic.sbr"
	-@erase "$(INTDIR)\RageFileDriver.obj"
	-@erase "$(INTDIR)\RageFileDriver.sbr"
	-@erase "$(INTDIR)\RageFileDriverDeflate.obj"
	-@erase "$(INTDIR)\RageFileDriverDeflate.sbr"
	-@erase "$(INTDIR)\RageFileDriverDirect.obj"
	-@erase "$(INTDIR)\RageFileDriverDirect.sbr"
	-@erase "$(INTDIR)\RageFileDriverDirectHelpers.obj"
	-@erase "$(INTDIR)\RageFileDriverDirectHelpers.sbr"
	-@erase "$(INTDIR)\RageFileDriverMemory.obj"
	-@erase "$(INTDIR)\RageFileDriverMemory.sbr"
	-@erase "$(INTDIR)\RageFileDriverSlice.obj"
	-@erase "$(INTDIR)\RageFileDriverSlice.sbr"
	-@erase "$(INTDIR)\RageFileDriverTimeout.obj"
	-@erase "$(INTDIR)\RageFileDriverTimeout.sbr"
	-@erase "$(INTDIR)\RageFileDriverZip.obj"
	-@erase "$(INTDIR)\RageFileDriverZip.sbr"
	-@erase "$(INTDIR)\RageFileManager.obj"
	-@erase "$(INTDIR)\RageFileManager.sbr"
	-@erase "$(INTDIR)\RageInput.obj"
	-@erase "$(INTDIR)\RageInput.sbr"
	-@erase "$(INTDIR)\RageInputDevice.obj"
	-@erase "$(INTDIR)\RageInputDevice.sbr"
	-@erase "$(INTDIR)\RageLog.obj"
	-@erase "$(INTDIR)\RageLog.sbr"
	-@erase "$(INTDIR)\RageMath.obj"
	-@erase "$(INTDIR)\RageMath.sbr"
	-@erase "$(INTDIR)\RageModelGeometry.obj"
	-@erase "$(INTDIR)\RageModelGeometry.sbr"
	-@erase "$(INTDIR)\RageSound.obj"
	-@erase "$(INTDIR)\RageSound.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_DSound.obj"
	-@erase "$(INTDIR)\RageSoundDriver_DSound.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_DSound_Software.obj"
	-@erase "$(INTDIR)\RageSoundDriver_DSound_Software.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_Generic_Software.obj"
	-@erase "$(INTDIR)\RageSoundDriver_Generic_Software.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_Null.obj"
	-@erase "$(INTDIR)\RageSoundDriver_Null.sbr"
	-@erase "$(INTDIR)\RageSoundDriver_WaveOut.obj"
	-@erase "$(INTDIR)\RageSoundDriver_WaveOut.sbr"
	-@erase "$(INTDIR)\RageSoundManager.obj"
	-@erase "$(INTDIR)\RageSoundManager.sbr"
	-@erase "$(INTDIR)\RageSoundMixBuffer.obj"
	-@erase "$(INTDIR)\RageSoundMixBuffer.sbr"
	-@erase "$(INTDIR)\RageSoundPosMap.obj"
	-@erase "$(INTDIR)\RageSoundPosMap.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Chain.obj"
	-@erase "$(INTDIR)\RageSoundReader_Chain.sbr"
	-@erase "$(INTDIR)\RageSoundReader_FileReader.obj"
	-@erase "$(INTDIR)\RageSoundReader_FileReader.sbr"
	-@erase "$(INTDIR)\RageSoundReader_MP3.obj"
	-@erase "$(INTDIR)\RageSoundReader_MP3.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Preload.obj"
	-@erase "$(INTDIR)\RageSoundReader_Preload.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Fast.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Fast.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Good.obj"
	-@erase "$(INTDIR)\RageSoundReader_Resample_Good.sbr"
	-@erase "$(INTDIR)\RageSoundReader_Vorbisfile.obj"
	-@erase "$(INTDIR)\RageSoundReader_Vorbisfile.sbr"
	-@erase "$(INTDIR)\RageSoundReader_WAV.obj"
	-@erase "$(INTDIR)\RageSoundReader_WAV.sbr"
	-@erase "$(INTDIR)\RageSoundResampler.obj"
	-@erase "$(INTDIR)\RageSoundResampler.sbr"
	-@erase "$(INTDIR)\RageSoundUtil.obj"
	-@erase "$(INTDIR)\RageSoundUtil.sbr"
	-@erase "$(INTDIR)\RageSurface.obj"
	-@erase "$(INTDIR)\RageSurface.sbr"
	-@erase "$(INTDIR)\RageSurface_Load.obj"
	-@erase "$(INTDIR)\RageSurface_Load.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_BMP.obj"
	-@erase "$(INTDIR)\RageSurface_Load_BMP.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_GIF.obj"
	-@erase "$(INTDIR)\RageSurface_Load_GIF.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_JPEG.obj"
	-@erase "$(INTDIR)\RageSurface_Load_JPEG.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_PNG.obj"
	-@erase "$(INTDIR)\RageSurface_Load_PNG.sbr"
	-@erase "$(INTDIR)\RageSurface_Load_XPM.obj"
	-@erase "$(INTDIR)\RageSurface_Load_XPM.sbr"
	-@erase "$(INTDIR)\RageSurface_Save_BMP.obj"
	-@erase "$(INTDIR)\RageSurface_Save_BMP.sbr"
	-@erase "$(INTDIR)\RageSurface_Save_JPEG.obj"
	-@erase "$(INTDIR)\RageSurface_Save_JPEG.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Dither.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Dither.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Palettize.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Palettize.sbr"
	-@erase "$(INTDIR)\RageSurfaceUtils_Zoom.obj"
	-@erase "$(INTDIR)\RageSurfaceUtils_Zoom.sbr"
	-@erase "$(INTDIR)\RageTexture.obj"
	-@erase "$(INTDIR)\RageTexture.sbr"
	-@erase "$(INTDIR)\RageTextureID.obj"
	-@erase "$(INTDIR)\RageTextureID.sbr"
	-@erase "$(INTDIR)\RageTextureManager.obj"
	-@erase "$(INTDIR)\RageTextureManager.sbr"
	-@erase "$(INTDIR)\RageThreads.obj"
	-@erase "$(INTDIR)\RageThreads.sbr"
	-@erase "$(INTDIR)\RageTimer.obj"
	-@erase "$(INTDIR)\RageTimer.sbr"
	-@erase "$(INTDIR)\RageUtil.obj"
	-@erase "$(INTDIR)\RageUtil.sbr"
	-@erase "$(INTDIR)\RageUtil_BackgroundLoader.obj"
	-@erase "$(INTDIR)\RageUtil_BackgroundLoader.sbr"
	-@erase "$(INTDIR)\RageUtil_CharConversions.obj"
	-@erase "$(INTDIR)\RageUtil_CharConversions.sbr"
	-@erase "$(INTDIR)\RageUtil_FileDB.obj"
	-@erase "$(INTDIR)\RageUtil_FileDB.sbr"
	-@erase "$(INTDIR)\RageUtil_WorkerThread.obj"
	-@erase "$(INTDIR)\RageUtil_WorkerThread.sbr"
	-@erase "$(INTDIR)\RandomSample.obj"
	-@erase "$(INTDIR)\RandomSample.sbr"
	-@erase "$(INTDIR)\ReceptorArrow.obj"
	-@erase "$(INTDIR)\ReceptorArrow.sbr"
	-@erase "$(INTDIR)\ReceptorArrowRow.obj"
	-@erase "$(INTDIR)\ReceptorArrowRow.sbr"
	-@erase "$(INTDIR)\RegistryAccess.obj"
	-@erase "$(INTDIR)\RegistryAccess.sbr"
	-@erase "$(INTDIR)\RestartProgram.obj"
	-@erase "$(INTDIR)\RestartProgram.sbr"
	-@erase "$(INTDIR)\RollingNumbers.obj"
	-@erase "$(INTDIR)\RollingNumbers.sbr"
	-@erase "$(INTDIR)\RoomWheel.obj"
	-@erase "$(INTDIR)\RoomWheel.sbr"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rsa.sbr"
	-@erase "$(INTDIR)\ScoreDisplay.obj"
	-@erase "$(INTDIR)\ScoreDisplay.sbr"
	-@erase "$(INTDIR)\ScoreDisplayAliveTime.obj"
	-@erase "$(INTDIR)\ScoreDisplayAliveTime.sbr"
	-@erase "$(INTDIR)\ScoreDisplayBattle.obj"
	-@erase "$(INTDIR)\ScoreDisplayBattle.sbr"
	-@erase "$(INTDIR)\ScoreDisplayCalories.obj"
	-@erase "$(INTDIR)\ScoreDisplayCalories.sbr"
	-@erase "$(INTDIR)\ScoreDisplayLifeTime.obj"
	-@erase "$(INTDIR)\ScoreDisplayLifeTime.sbr"
	-@erase "$(INTDIR)\ScoreDisplayNormal.obj"
	-@erase "$(INTDIR)\ScoreDisplayNormal.sbr"
	-@erase "$(INTDIR)\ScoreDisplayOni.obj"
	-@erase "$(INTDIR)\ScoreDisplayOni.sbr"
	-@erase "$(INTDIR)\ScoreDisplayPercentage.obj"
	-@erase "$(INTDIR)\ScoreDisplayPercentage.sbr"
	-@erase "$(INTDIR)\ScoreDisplayRave.obj"
	-@erase "$(INTDIR)\ScoreDisplayRave.sbr"
	-@erase "$(INTDIR)\ScoreKeeperMAX2.obj"
	-@erase "$(INTDIR)\ScoreKeeperMAX2.sbr"
	-@erase "$(INTDIR)\ScoreKeeperRave.obj"
	-@erase "$(INTDIR)\ScoreKeeperRave.sbr"
	-@erase "$(INTDIR)\Screen.obj"
	-@erase "$(INTDIR)\Screen.sbr"
	-@erase "$(INTDIR)\ScreenAttract.obj"
	-@erase "$(INTDIR)\ScreenAttract.sbr"
	-@erase "$(INTDIR)\ScreenBookkeeping.obj"
	-@erase "$(INTDIR)\ScreenBookkeeping.sbr"
	-@erase "$(INTDIR)\ScreenBranch.obj"
	-@erase "$(INTDIR)\ScreenBranch.sbr"
	-@erase "$(INTDIR)\ScreenCenterImage.obj"
	-@erase "$(INTDIR)\ScreenCenterImage.sbr"
	-@erase "$(INTDIR)\ScreenCredits.obj"
	-@erase "$(INTDIR)\ScreenCredits.sbr"
	-@erase "$(INTDIR)\ScreenDebugOverlay.obj"
	-@erase "$(INTDIR)\ScreenDebugOverlay.sbr"
	-@erase "$(INTDIR)\ScreenDemonstration.obj"
	-@erase "$(INTDIR)\ScreenDemonstration.sbr"
	-@erase "$(INTDIR)\ScreenDimensions.obj"
	-@erase "$(INTDIR)\ScreenDimensions.sbr"
	-@erase "$(INTDIR)\ScreenEdit.obj"
	-@erase "$(INTDIR)\ScreenEdit.sbr"
	-@erase "$(INTDIR)\ScreenEditCoursesMenu.obj"
	-@erase "$(INTDIR)\ScreenEditCoursesMenu.sbr"
	-@erase "$(INTDIR)\ScreenEditMenu.obj"
	-@erase "$(INTDIR)\ScreenEditMenu.sbr"
	-@erase "$(INTDIR)\ScreenEnding.obj"
	-@erase "$(INTDIR)\ScreenEnding.sbr"
	-@erase "$(INTDIR)\ScreenEndlessBreak.obj"
	-@erase "$(INTDIR)\ScreenEndlessBreak.sbr"
	-@erase "$(INTDIR)\ScreenEvaluation.obj"
	-@erase "$(INTDIR)\ScreenEvaluation.sbr"
	-@erase "$(INTDIR)\ScreenExit.obj"
	-@erase "$(INTDIR)\ScreenExit.sbr"
	-@erase "$(INTDIR)\ScreenEz2SelectMusic.obj"
	-@erase "$(INTDIR)\ScreenEz2SelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenEz2SelectPlayer.obj"
	-@erase "$(INTDIR)\ScreenEz2SelectPlayer.sbr"
	-@erase "$(INTDIR)\ScreenGameplay.obj"
	-@erase "$(INTDIR)\ScreenGameplay.sbr"
	-@erase "$(INTDIR)\ScreenGameplayMultiplayer.obj"
	-@erase "$(INTDIR)\ScreenGameplayMultiplayer.sbr"
	-@erase "$(INTDIR)\ScreenHowToPlay.obj"
	-@erase "$(INTDIR)\ScreenHowToPlay.sbr"
	-@erase "$(INTDIR)\ScreenInstructions.obj"
	-@erase "$(INTDIR)\ScreenInstructions.sbr"
	-@erase "$(INTDIR)\ScreenJukebox.obj"
	-@erase "$(INTDIR)\ScreenJukebox.sbr"
	-@erase "$(INTDIR)\ScreenLogo.obj"
	-@erase "$(INTDIR)\ScreenLogo.sbr"
	-@erase "$(INTDIR)\ScreenManager.obj"
	-@erase "$(INTDIR)\ScreenManager.sbr"
	-@erase "$(INTDIR)\ScreenMapControllers.obj"
	-@erase "$(INTDIR)\ScreenMapControllers.sbr"
	-@erase "$(INTDIR)\ScreenMessage.obj"
	-@erase "$(INTDIR)\ScreenMessage.sbr"
	-@erase "$(INTDIR)\ScreenMiniMenu.obj"
	-@erase "$(INTDIR)\ScreenMiniMenu.sbr"
	-@erase "$(INTDIR)\ScreenMusicScroll.obj"
	-@erase "$(INTDIR)\ScreenMusicScroll.sbr"
	-@erase "$(INTDIR)\ScreenNameEntry.obj"
	-@erase "$(INTDIR)\ScreenNameEntry.sbr"
	-@erase "$(INTDIR)\ScreenNameEntryTraditional.obj"
	-@erase "$(INTDIR)\ScreenNameEntryTraditional.sbr"
	-@erase "$(INTDIR)\ScreenNetEvaluation.obj"
	-@erase "$(INTDIR)\ScreenNetEvaluation.sbr"
	-@erase "$(INTDIR)\ScreenNetRoom.obj"
	-@erase "$(INTDIR)\ScreenNetRoom.sbr"
	-@erase "$(INTDIR)\ScreenNetSelectBase.obj"
	-@erase "$(INTDIR)\ScreenNetSelectBase.sbr"
	-@erase "$(INTDIR)\ScreenNetSelectMusic.obj"
	-@erase "$(INTDIR)\ScreenNetSelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenNetworkOptions.obj"
	-@erase "$(INTDIR)\ScreenNetworkOptions.sbr"
	-@erase "$(INTDIR)\ScreenOptions.obj"
	-@erase "$(INTDIR)\ScreenOptions.sbr"
	-@erase "$(INTDIR)\ScreenOptionsMaster.obj"
	-@erase "$(INTDIR)\ScreenOptionsMaster.sbr"
	-@erase "$(INTDIR)\ScreenOptionsMasterPrefs.obj"
	-@erase "$(INTDIR)\ScreenOptionsMasterPrefs.sbr"
	-@erase "$(INTDIR)\ScreenPackages.obj"
	-@erase "$(INTDIR)\ScreenPackages.sbr"
	-@erase "$(INTDIR)\ScreenPlayerOptions.obj"
	-@erase "$(INTDIR)\ScreenPlayerOptions.sbr"
	-@erase "$(INTDIR)\ScreenProfileOptions.obj"
	-@erase "$(INTDIR)\ScreenProfileOptions.sbr"
	-@erase "$(INTDIR)\ScreenPrompt.obj"
	-@erase "$(INTDIR)\ScreenPrompt.sbr"
	-@erase "$(INTDIR)\ScreenRanking.obj"
	-@erase "$(INTDIR)\ScreenRanking.sbr"
	-@erase "$(INTDIR)\ScreenReloadSongs.obj"
	-@erase "$(INTDIR)\ScreenReloadSongs.sbr"
	-@erase "$(INTDIR)\ScreenSandbox.obj"
	-@erase "$(INTDIR)\ScreenSandbox.sbr"
	-@erase "$(INTDIR)\ScreenSelect.obj"
	-@erase "$(INTDIR)\ScreenSelect.sbr"
	-@erase "$(INTDIR)\ScreenSelectCharacter.obj"
	-@erase "$(INTDIR)\ScreenSelectCharacter.sbr"
	-@erase "$(INTDIR)\ScreenSelectDifficulty.obj"
	-@erase "$(INTDIR)\ScreenSelectDifficulty.sbr"
	-@erase "$(INTDIR)\ScreenSelectGroup.obj"
	-@erase "$(INTDIR)\ScreenSelectGroup.sbr"
	-@erase "$(INTDIR)\ScreenSelectMaster.obj"
	-@erase "$(INTDIR)\ScreenSelectMaster.sbr"
	-@erase "$(INTDIR)\ScreenSelectMode.obj"
	-@erase "$(INTDIR)\ScreenSelectMode.sbr"
	-@erase "$(INTDIR)\ScreenSelectMusic.obj"
	-@erase "$(INTDIR)\ScreenSelectMusic.sbr"
	-@erase "$(INTDIR)\ScreenSelectStyle.obj"
	-@erase "$(INTDIR)\ScreenSelectStyle.sbr"
	-@erase "$(INTDIR)\ScreenSetTime.obj"
	-@erase "$(INTDIR)\ScreenSetTime.sbr"
	-@erase "$(INTDIR)\ScreenSMOnlineLogin.obj"
	-@erase "$(INTDIR)\ScreenSMOnlineLogin.sbr"
	-@erase "$(INTDIR)\ScreenSongOptions.obj"
	-@erase "$(INTDIR)\ScreenSongOptions.sbr"
	-@erase "$(INTDIR)\ScreenSplash.obj"
	-@erase "$(INTDIR)\ScreenSplash.sbr"
	-@erase "$(INTDIR)\ScreenStage.obj"
	-@erase "$(INTDIR)\ScreenStage.sbr"
	-@erase "$(INTDIR)\ScreenStyleSplash.obj"
	-@erase "$(INTDIR)\ScreenStyleSplash.sbr"
	-@erase "$(INTDIR)\ScreenSystemLayer.obj"
	-@erase "$(INTDIR)\ScreenSystemLayer.sbr"
	-@erase "$(INTDIR)\ScreenTest.obj"
	-@erase "$(INTDIR)\ScreenTest.sbr"
	-@erase "$(INTDIR)\ScreenTestFonts.obj"
	-@erase "$(INTDIR)\ScreenTestFonts.sbr"
	-@erase "$(INTDIR)\ScreenTestInput.obj"
	-@erase "$(INTDIR)\ScreenTestInput.sbr"
	-@erase "$(INTDIR)\ScreenTestLights.obj"
	-@erase "$(INTDIR)\ScreenTestLights.sbr"
	-@erase "$(INTDIR)\ScreenTestSound.obj"
	-@erase "$(INTDIR)\ScreenTestSound.sbr"
	-@erase "$(INTDIR)\ScreenTextEntry.obj"
	-@erase "$(INTDIR)\ScreenTextEntry.sbr"
	-@erase "$(INTDIR)\ScreenTitleMenu.obj"
	-@erase "$(INTDIR)\ScreenTitleMenu.sbr"
	-@erase "$(INTDIR)\ScreenUnlock.obj"
	-@erase "$(INTDIR)\ScreenUnlock.sbr"
	-@erase "$(INTDIR)\ScreenWithMenuElements.obj"
	-@erase "$(INTDIR)\ScreenWithMenuElements.sbr"
	-@erase "$(INTDIR)\ScrollBar.obj"
	-@erase "$(INTDIR)\ScrollBar.sbr"
	-@erase "$(INTDIR)\ScrollingList.obj"
	-@erase "$(INTDIR)\ScrollingList.sbr"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\sha.sbr"
	-@erase "$(INTDIR)\SnapDisplay.obj"
	-@erase "$(INTDIR)\SnapDisplay.sbr"
	-@erase "$(INTDIR)\Song.obj"
	-@erase "$(INTDIR)\Song.sbr"
	-@erase "$(INTDIR)\SongCacheIndex.obj"
	-@erase "$(INTDIR)\SongCacheIndex.sbr"
	-@erase "$(INTDIR)\SongManager.obj"
	-@erase "$(INTDIR)\SongManager.sbr"
	-@erase "$(INTDIR)\SongOptions.obj"
	-@erase "$(INTDIR)\SongOptions.sbr"
	-@erase "$(INTDIR)\SongUtil.obj"
	-@erase "$(INTDIR)\SongUtil.sbr"
	-@erase "$(INTDIR)\Sprite.obj"
	-@erase "$(INTDIR)\Sprite.sbr"
	-@erase "$(INTDIR)\StageStats.obj"
	-@erase "$(INTDIR)\StageStats.sbr"
	-@erase "$(INTDIR)\StatsManager.obj"
	-@erase "$(INTDIR)\StatsManager.sbr"
	-@erase "$(INTDIR)\StepMania.obj"
	-@erase "$(INTDIR)\StepMania.sbr"
	-@erase "$(INTDIR)\Steps.obj"
	-@erase "$(INTDIR)\Steps.sbr"
	-@erase "$(INTDIR)\StepsUtil.obj"
	-@erase "$(INTDIR)\StepsUtil.sbr"
	-@erase "$(INTDIR)\StreamDisplay.obj"
	-@erase "$(INTDIR)\StreamDisplay.sbr"
	-@erase "$(INTDIR)\study.obj"
	-@erase "$(INTDIR)\study.sbr"
	-@erase "$(INTDIR)\Style.obj"
	-@erase "$(INTDIR)\Style.sbr"
	-@erase "$(INTDIR)\StyleUtil.obj"
	-@erase "$(INTDIR)\StyleUtil.sbr"
	-@erase "$(INTDIR)\TextBanner.obj"
	-@erase "$(INTDIR)\TextBanner.sbr"
	-@erase "$(INTDIR)\ThemeManager.obj"
	-@erase "$(INTDIR)\ThemeManager.sbr"
	-@erase "$(INTDIR)\Threads_Win32.obj"
	-@erase "$(INTDIR)\Threads_Win32.sbr"
	-@erase "$(INTDIR)\TimingData.obj"
	-@erase "$(INTDIR)\TimingData.sbr"
	-@erase "$(INTDIR)\TitleSubstitution.obj"
	-@erase "$(INTDIR)\TitleSubstitution.sbr"
	-@erase "$(INTDIR)\Trail.obj"
	-@erase "$(INTDIR)\Trail.sbr"
	-@erase "$(INTDIR)\TrailUtil.obj"
	-@erase "$(INTDIR)\TrailUtil.sbr"
	-@erase "$(INTDIR)\Transition.obj"
	-@erase "$(INTDIR)\Transition.sbr"
	-@erase "$(INTDIR)\UnlockManager.obj"
	-@erase "$(INTDIR)\UnlockManager.sbr"
	-@erase "$(INTDIR)\USB.obj"
	-@erase "$(INTDIR)\USB.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VideoDriverInfo.obj"
	-@erase "$(INTDIR)\VideoDriverInfo.sbr"
	-@erase "$(INTDIR)\WheelBase.obj"
	-@erase "$(INTDIR)\WheelBase.sbr"
	-@erase "$(INTDIR)\WheelItemBase.obj"
	-@erase "$(INTDIR)\WheelItemBase.sbr"
	-@erase "$(INTDIR)\WheelNotifyIcon.obj"
	-@erase "$(INTDIR)\WheelNotifyIcon.sbr"
	-@erase "$(INTDIR)\WindowIcon.obj"
	-@erase "$(INTDIR)\WindowIcon.sbr"
	-@erase "$(INTDIR)\WindowsResources.res"
	-@erase "$(INTDIR)\XmlFile.obj"
	-@erase "$(INTDIR)\XmlFile.sbr"
	-@erase "$(OUTDIR)\StepMania.map"
	-@erase "..\Program\StepMania.exe"
	-@erase "..\StepMania-StackTrace.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WindowsResources.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"../StepMania-StackTrace.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\get.sbr" \
	"$(INTDIR)\maketables.sbr" \
	"$(INTDIR)\pcre.sbr" \
	"$(INTDIR)\study.sbr" \
	"$(INTDIR)\RageBitmapTexture.sbr" \
	"$(INTDIR)\RageDisplay.sbr" \
	"$(INTDIR)\RageDisplay_D3D.sbr" \
	"$(INTDIR)\RageDisplay_Null.sbr" \
	"$(INTDIR)\RageDisplay_OGL.sbr" \
	"$(INTDIR)\RageDisplay_OGL_Extensions.sbr" \
	"$(INTDIR)\RageException.sbr" \
	"$(INTDIR)\RageFile.sbr" \
	"$(INTDIR)\RageFileBasic.sbr" \
	"$(INTDIR)\RageFileDriver.sbr" \
	"$(INTDIR)\RageFileDriverDeflate.sbr" \
	"$(INTDIR)\RageFileDriverDirect.sbr" \
	"$(INTDIR)\RageFileDriverDirectHelpers.sbr" \
	"$(INTDIR)\RageFileDriverMemory.sbr" \
	"$(INTDIR)\RageFileDriverSlice.sbr" \
	"$(INTDIR)\RageFileDriverTimeout.sbr" \
	"$(INTDIR)\RageFileDriverZip.sbr" \
	"$(INTDIR)\RageFileManager.sbr" \
	"$(INTDIR)\RageInput.sbr" \
	"$(INTDIR)\RageInputDevice.sbr" \
	"$(INTDIR)\RageLog.sbr" \
	"$(INTDIR)\RageMath.sbr" \
	"$(INTDIR)\RageModelGeometry.sbr" \
	"$(INTDIR)\RageSound.sbr" \
	"$(INTDIR)\RageSoundManager.sbr" \
	"$(INTDIR)\RageSoundMixBuffer.sbr" \
	"$(INTDIR)\RageSoundPosMap.sbr" \
	"$(INTDIR)\RageSoundReader_Chain.sbr" \
	"$(INTDIR)\RageSoundReader_FileReader.sbr" \
	"$(INTDIR)\RageSoundReader_MP3.sbr" \
	"$(INTDIR)\RageSoundReader_Preload.sbr" \
	"$(INTDIR)\RageSoundReader_Resample.sbr" \
	"$(INTDIR)\RageSoundReader_Resample_Fast.sbr" \
	"$(INTDIR)\RageSoundReader_Resample_Good.sbr" \
	"$(INTDIR)\RageSoundReader_Vorbisfile.sbr" \
	"$(INTDIR)\RageSoundReader_WAV.sbr" \
	"$(INTDIR)\RageSoundResampler.sbr" \
	"$(INTDIR)\RageSoundUtil.sbr" \
	"$(INTDIR)\RageSurface.sbr" \
	"$(INTDIR)\RageSurface_Load.sbr" \
	"$(INTDIR)\RageSurface_Load_BMP.sbr" \
	"$(INTDIR)\RageSurface_Load_GIF.sbr" \
	"$(INTDIR)\RageSurface_Load_JPEG.sbr" \
	"$(INTDIR)\RageSurface_Load_PNG.sbr" \
	"$(INTDIR)\RageSurface_Load_XPM.sbr" \
	"$(INTDIR)\RageSurface_Save_BMP.sbr" \
	"$(INTDIR)\RageSurface_Save_JPEG.sbr" \
	"$(INTDIR)\RageSurfaceUtils.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Dither.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Palettize.sbr" \
	"$(INTDIR)\RageSurfaceUtils_Zoom.sbr" \
	"$(INTDIR)\RageTexture.sbr" \
	"$(INTDIR)\RageTextureID.sbr" \
	"$(INTDIR)\RageTextureManager.sbr" \
	"$(INTDIR)\RageThreads.sbr" \
	"$(INTDIR)\RageTimer.sbr" \
	"$(INTDIR)\RageUtil.sbr" \
	"$(INTDIR)\RageUtil_BackgroundLoader.sbr" \
	"$(INTDIR)\RageUtil_CharConversions.sbr" \
	"$(INTDIR)\RageUtil_FileDB.sbr" \
	"$(INTDIR)\RageUtil_WorkerThread.sbr" \
	"$(INTDIR)\ActorCommands.sbr" \
	"$(INTDIR)\Attack.sbr" \
	"$(INTDIR)\AutoKeysounds.sbr" \
	"$(INTDIR)\BannerCache.sbr" \
	"$(INTDIR)\CatalogXml.sbr" \
	"$(INTDIR)\Character.sbr" \
	"$(INTDIR)\CodeDetector.sbr" \
	"$(INTDIR)\Command.sbr" \
	"$(INTDIR)\CommonMetrics.sbr" \
	"$(INTDIR)\Course.sbr" \
	"$(INTDIR)\CourseUtil.sbr" \
	"$(INTDIR)\DateTime.sbr" \
	"$(INTDIR)\Difficulty.sbr" \
	"$(INTDIR)\EnumHelper.sbr" \
	"$(INTDIR)\Font.sbr" \
	"$(INTDIR)\FontCharAliases.sbr" \
	"$(INTDIR)\FontCharmaps.sbr" \
	"$(INTDIR)\Game.sbr" \
	"$(INTDIR)\GameCommand.sbr" \
	"$(INTDIR)\GameConstantsAndTypes.sbr" \
	"$(INTDIR)\GameInput.sbr" \
	"$(INTDIR)\Grade.sbr" \
	"$(INTDIR)\HighScore.sbr" \
	"$(INTDIR)\Inventory.sbr" \
	"$(INTDIR)\LuaReference.sbr" \
	"$(INTDIR)\LyricsLoader.sbr" \
	"$(INTDIR)\NoteData.sbr" \
	"$(INTDIR)\NoteDataUtil.sbr" \
	"$(INTDIR)\NoteDataWithScoring.sbr" \
	"$(INTDIR)\NoteFieldPositioning.sbr" \
	"$(INTDIR)\NotesLoader.sbr" \
	"$(INTDIR)\NotesLoaderBMS.sbr" \
	"$(INTDIR)\NotesLoaderDWI.sbr" \
	"$(INTDIR)\NotesLoaderKSF.sbr" \
	"$(INTDIR)\NotesLoaderSM.sbr" \
	"$(INTDIR)\NotesWriterDWI.sbr" \
	"$(INTDIR)\NotesWriterSM.sbr" \
	"$(INTDIR)\NoteTypes.sbr" \
	"$(INTDIR)\OptionRowHandler.sbr" \
	"$(INTDIR)\PlayerAI.sbr" \
	"$(INTDIR)\PlayerNumber.sbr" \
	"$(INTDIR)\PlayerOptions.sbr" \
	"$(INTDIR)\PlayerStageStats.sbr" \
	"$(INTDIR)\PlayerState.sbr" \
	"$(INTDIR)\Preference.sbr" \
	"$(INTDIR)\Profile.sbr" \
	"$(INTDIR)\RadarValues.sbr" \
	"$(INTDIR)\RandomSample.sbr" \
	"$(INTDIR)\ScoreKeeperMAX2.sbr" \
	"$(INTDIR)\ScoreKeeperRave.sbr" \
	"$(INTDIR)\ScreenDimensions.sbr" \
	"$(INTDIR)\Song.sbr" \
	"$(INTDIR)\SongCacheIndex.sbr" \
	"$(INTDIR)\SongOptions.sbr" \
	"$(INTDIR)\SongUtil.sbr" \
	"$(INTDIR)\StageStats.sbr" \
	"$(INTDIR)\Steps.sbr" \
	"$(INTDIR)\StepsUtil.sbr" \
	"$(INTDIR)\Style.sbr" \
	"$(INTDIR)\StyleUtil.sbr" \
	"$(INTDIR)\TimingData.sbr" \
	"$(INTDIR)\TitleSubstitution.sbr" \
	"$(INTDIR)\Trail.sbr" \
	"$(INTDIR)\TrailUtil.sbr" \
	"$(INTDIR)\IniFile.sbr" \
	"$(INTDIR)\MsdFile.sbr" \
	"$(INTDIR)\XmlFile.sbr" \
	"$(INTDIR)\LoadingWindow_Win32.sbr" \
	"$(INTDIR)\DSoundHelpers.sbr" \
	"$(INTDIR)\RageSoundDriver_DSound.sbr" \
	"$(INTDIR)\RageSoundDriver_DSound_Software.sbr" \
	"$(INTDIR)\RageSoundDriver_Generic_Software.sbr" \
	"$(INTDIR)\RageSoundDriver_Null.sbr" \
	"$(INTDIR)\RageSoundDriver_WaveOut.sbr" \
	"$(INTDIR)\ArchHooks.sbr" \
	"$(INTDIR)\ArchHooks_Win32.sbr" \
	"$(INTDIR)\InputHandler.sbr" \
	"$(INTDIR)\InputHandler_DirectInput.sbr" \
	"$(INTDIR)\InputHandler_DirectInputHelper.sbr" \
	"$(INTDIR)\InputHandler_MonkeyKeyboard.sbr" \
	"$(INTDIR)\InputHandler_Win32_Para.sbr" \
	"$(INTDIR)\InputHandler_Win32_Pump.sbr" \
	"$(INTDIR)\MovieTexture.sbr" \
	"$(INTDIR)\MovieTexture_DShow.sbr" \
	"$(INTDIR)\MovieTexture_DShowHelper.sbr" \
	"$(INTDIR)\MovieTexture_FFMpeg.sbr" \
	"$(INTDIR)\MovieTexture_Null.sbr" \
	"$(INTDIR)\LowLevelWindow_Win32.sbr" \
	"$(INTDIR)\LightsDriver_SystemMessage.sbr" \
	"$(INTDIR)\LightsDriver_Win32Parallel.sbr" \
	"$(INTDIR)\MemoryCardDriver.sbr" \
	"$(INTDIR)\MemoryCardDriverThreaded_Windows.sbr" \
	"$(INTDIR)\Dialog.sbr" \
	"$(INTDIR)\DialogDriver_Win32.sbr" \
	"$(INTDIR)\Threads_Win32.sbr" \
	"$(INTDIR)\arch.sbr" \
	"$(INTDIR)\AppInstance.sbr" \
	"$(INTDIR)\arch_setup.sbr" \
	"$(INTDIR)\Crash.sbr" \
	"$(INTDIR)\DebugInfoHunt.sbr" \
	"$(INTDIR)\GetFileInformation.sbr" \
	"$(INTDIR)\GotoURL.sbr" \
	"$(INTDIR)\GraphicsWindow.sbr" \
	"$(INTDIR)\RegistryAccess.sbr" \
	"$(INTDIR)\RestartProgram.sbr" \
	"$(INTDIR)\USB.sbr" \
	"$(INTDIR)\VideoDriverInfo.sbr" \
	"$(INTDIR)\WindowIcon.sbr" \
	"$(INTDIR)\global.sbr" \
	"$(INTDIR)\StepMania.sbr" \
	"$(INTDIR)\Actor.sbr" \
	"$(INTDIR)\ActorFrame.sbr" \
	"$(INTDIR)\ActorScroller.sbr" \
	"$(INTDIR)\ActorUtil.sbr" \
	"$(INTDIR)\AutoActor.sbr" \
	"$(INTDIR)\BitmapText.sbr" \
	"$(INTDIR)\Model.sbr" \
	"$(INTDIR)\ModelManager.sbr" \
	"$(INTDIR)\ModelTypes.sbr" \
	"$(INTDIR)\Quad.sbr" \
	"$(INTDIR)\RollingNumbers.sbr" \
	"$(INTDIR)\Sprite.sbr" \
	"$(INTDIR)\Banner.sbr" \
	"$(INTDIR)\BGAnimation.sbr" \
	"$(INTDIR)\BGAnimationLayer.sbr" \
	"$(INTDIR)\DifficultyIcon.sbr" \
	"$(INTDIR)\FadingBanner.sbr" \
	"$(INTDIR)\MeterDisplay.sbr" \
	"$(INTDIR)\StreamDisplay.sbr" \
	"$(INTDIR)\Transition.sbr" \
	"$(INTDIR)\ActiveAttackList.sbr" \
	"$(INTDIR)\ArrowEffects.sbr" \
	"$(INTDIR)\AttackDisplay.sbr" \
	"$(INTDIR)\Background.sbr" \
	"$(INTDIR)\BeginnerHelper.sbr" \
	"$(INTDIR)\CharacterHead.sbr" \
	"$(INTDIR)\CombinedLifeMeterTug.sbr" \
	"$(INTDIR)\Combo.sbr" \
	"$(INTDIR)\DancingCharacters.sbr" \
	"$(INTDIR)\Foreground.sbr" \
	"$(INTDIR)\GhostArrow.sbr" \
	"$(INTDIR)\GhostArrowRow.sbr" \
	"$(INTDIR)\HoldGhostArrow.sbr" \
	"$(INTDIR)\HoldJudgment.sbr" \
	"$(INTDIR)\Judgment.sbr" \
	"$(INTDIR)\LifeMeterBar.sbr" \
	"$(INTDIR)\LifeMeterBattery.sbr" \
	"$(INTDIR)\LifeMeterTime.sbr" \
	"$(INTDIR)\LyricDisplay.sbr" \
	"$(INTDIR)\NoteDisplay.sbr" \
	"$(INTDIR)\NoteField.sbr" \
	"$(INTDIR)\PercentageDisplay.sbr" \
	"$(INTDIR)\Player.sbr" \
	"$(INTDIR)\ReceptorArrow.sbr" \
	"$(INTDIR)\ReceptorArrowRow.sbr" \
	"$(INTDIR)\ScoreDisplay.sbr" \
	"$(INTDIR)\ScoreDisplayAliveTime.sbr" \
	"$(INTDIR)\ScoreDisplayBattle.sbr" \
	"$(INTDIR)\ScoreDisplayCalories.sbr" \
	"$(INTDIR)\ScoreDisplayLifeTime.sbr" \
	"$(INTDIR)\ScoreDisplayNormal.sbr" \
	"$(INTDIR)\ScoreDisplayOni.sbr" \
	"$(INTDIR)\ScoreDisplayPercentage.sbr" \
	"$(INTDIR)\ScoreDisplayRave.sbr" \
	"$(INTDIR)\BPMDisplay.sbr" \
	"$(INTDIR)\ComboGraph.sbr" \
	"$(INTDIR)\CourseContentsList.sbr" \
	"$(INTDIR)\CourseEntryDisplay.sbr" \
	"$(INTDIR)\DifficultyDisplay.sbr" \
	"$(INTDIR)\DifficultyList.sbr" \
	"$(INTDIR)\DifficultyMeter.sbr" \
	"$(INTDIR)\DifficultyRating.sbr" \
	"$(INTDIR)\DualScrollBar.sbr" \
	"$(INTDIR)\EditCoursesMenu.sbr" \
	"$(INTDIR)\EditCoursesSongMenu.sbr" \
	"$(INTDIR)\EditMenu.sbr" \
	"$(INTDIR)\GradeDisplay.sbr" \
	"$(INTDIR)\GraphDisplay.sbr" \
	"$(INTDIR)\GrooveGraph.sbr" \
	"$(INTDIR)\GrooveRadar.sbr" \
	"$(INTDIR)\GroupList.sbr" \
	"$(INTDIR)\HelpDisplay.sbr" \
	"$(INTDIR)\MemoryCardDisplay.sbr" \
	"$(INTDIR)\MenuTimer.sbr" \
	"$(INTDIR)\ModeSwitcher.sbr" \
	"$(INTDIR)\MusicBannerWheel.sbr" \
	"$(INTDIR)\MusicList.sbr" \
	"$(INTDIR)\MusicSortDisplay.sbr" \
	"$(INTDIR)\MusicWheel.sbr" \
	"$(INTDIR)\MusicWheelItem.sbr" \
	"$(INTDIR)\OptionIcon.sbr" \
	"$(INTDIR)\OptionIconRow.sbr" \
	"$(INTDIR)\OptionRow.sbr" \
	"$(INTDIR)\OptionsCursor.sbr" \
	"$(INTDIR)\PaneDisplay.sbr" \
	"$(INTDIR)\RoomWheel.sbr" \
	"$(INTDIR)\ScrollBar.sbr" \
	"$(INTDIR)\ScrollingList.sbr" \
	"$(INTDIR)\SnapDisplay.sbr" \
	"$(INTDIR)\TextBanner.sbr" \
	"$(INTDIR)\WheelBase.sbr" \
	"$(INTDIR)\WheelItemBase.sbr" \
	"$(INTDIR)\WheelNotifyIcon.sbr" \
	"$(INTDIR)\Screen.sbr" \
	"$(INTDIR)\ScreenAttract.sbr" \
	"$(INTDIR)\ScreenBookkeeping.sbr" \
	"$(INTDIR)\ScreenBranch.sbr" \
	"$(INTDIR)\ScreenCenterImage.sbr" \
	"$(INTDIR)\ScreenCredits.sbr" \
	"$(INTDIR)\ScreenDebugOverlay.sbr" \
	"$(INTDIR)\ScreenDemonstration.sbr" \
	"$(INTDIR)\ScreenEdit.sbr" \
	"$(INTDIR)\ScreenEditCoursesMenu.sbr" \
	"$(INTDIR)\ScreenEditMenu.sbr" \
	"$(INTDIR)\ScreenEnding.sbr" \
	"$(INTDIR)\ScreenEndlessBreak.sbr" \
	"$(INTDIR)\ScreenEvaluation.sbr" \
	"$(INTDIR)\ScreenExit.sbr" \
	"$(INTDIR)\ScreenEz2SelectMusic.sbr" \
	"$(INTDIR)\ScreenEz2SelectPlayer.sbr" \
	"$(INTDIR)\ScreenGameplay.sbr" \
	"$(INTDIR)\ScreenGameplayMultiplayer.sbr" \
	"$(INTDIR)\ScreenHowToPlay.sbr" \
	"$(INTDIR)\ScreenInstructions.sbr" \
	"$(INTDIR)\ScreenJukebox.sbr" \
	"$(INTDIR)\ScreenLogo.sbr" \
	"$(INTDIR)\ScreenMapControllers.sbr" \
	"$(INTDIR)\ScreenMessage.sbr" \
	"$(INTDIR)\ScreenMiniMenu.sbr" \
	"$(INTDIR)\ScreenMusicScroll.sbr" \
	"$(INTDIR)\ScreenNameEntry.sbr" \
	"$(INTDIR)\ScreenNameEntryTraditional.sbr" \
	"$(INTDIR)\ScreenNetEvaluation.sbr" \
	"$(INTDIR)\ScreenNetRoom.sbr" \
	"$(INTDIR)\ScreenNetSelectBase.sbr" \
	"$(INTDIR)\ScreenNetSelectMusic.sbr" \
	"$(INTDIR)\ScreenNetworkOptions.sbr" \
	"$(INTDIR)\ScreenOptions.sbr" \
	"$(INTDIR)\ScreenOptionsMaster.sbr" \
	"$(INTDIR)\ScreenOptionsMasterPrefs.sbr" \
	"$(INTDIR)\ScreenPackages.sbr" \
	"$(INTDIR)\ScreenPlayerOptions.sbr" \
	"$(INTDIR)\ScreenProfileOptions.sbr" \
	"$(INTDIR)\ScreenPrompt.sbr" \
	"$(INTDIR)\ScreenRanking.sbr" \
	"$(INTDIR)\ScreenReloadSongs.sbr" \
	"$(INTDIR)\ScreenSandbox.sbr" \
	"$(INTDIR)\ScreenSelect.sbr" \
	"$(INTDIR)\ScreenSelectCharacter.sbr" \
	"$(INTDIR)\ScreenSelectDifficulty.sbr" \
	"$(INTDIR)\ScreenSelectGroup.sbr" \
	"$(INTDIR)\ScreenSelectMaster.sbr" \
	"$(INTDIR)\ScreenSelectMode.sbr" \
	"$(INTDIR)\ScreenSelectMusic.sbr" \
	"$(INTDIR)\ScreenSelectStyle.sbr" \
	"$(INTDIR)\ScreenSetTime.sbr" \
	"$(INTDIR)\ScreenSMOnlineLogin.sbr" \
	"$(INTDIR)\ScreenSongOptions.sbr" \
	"$(INTDIR)\ScreenSplash.sbr" \
	"$(INTDIR)\ScreenStage.sbr" \
	"$(INTDIR)\ScreenStyleSplash.sbr" \
	"$(INTDIR)\ScreenSystemLayer.sbr" \
	"$(INTDIR)\ScreenTest.sbr" \
	"$(INTDIR)\ScreenTestFonts.sbr" \
	"$(INTDIR)\ScreenTestInput.sbr" \
	"$(INTDIR)\ScreenTestLights.sbr" \
	"$(INTDIR)\ScreenTestSound.sbr" \
	"$(INTDIR)\ScreenTextEntry.sbr" \
	"$(INTDIR)\ScreenTitleMenu.sbr" \
	"$(INTDIR)\ScreenUnlock.sbr" \
	"$(INTDIR)\ScreenWithMenuElements.sbr" \
	"$(INTDIR)\AnnouncerManager.sbr" \
	"$(INTDIR)\Bookkeeper.sbr" \
	"$(INTDIR)\CryptManager.sbr" \
	"$(INTDIR)\ezsockets.sbr" \
	"$(INTDIR)\FontManager.sbr" \
	"$(INTDIR)\GameManager.sbr" \
	"$(INTDIR)\GameSoundManager.sbr" \
	"$(INTDIR)\GameState.sbr" \
	"$(INTDIR)\InputFilter.sbr" \
	"$(INTDIR)\InputMapper.sbr" \
	"$(INTDIR)\InputQueue.sbr" \
	"$(INTDIR)\LightsManager.sbr" \
	"$(INTDIR)\LuaManager.sbr" \
	"$(INTDIR)\MemoryCardManager.sbr" \
	"$(INTDIR)\MessageManager.sbr" \
	"$(INTDIR)\NetworkSyncManager.sbr" \
	"$(INTDIR)\NetworkSyncServer.sbr" \
	"$(INTDIR)\NoteSkinManager.sbr" \
	"$(INTDIR)\PrefsManager.sbr" \
	"$(INTDIR)\ProfileManager.sbr" \
	"$(INTDIR)\ScreenManager.sbr" \
	"$(INTDIR)\SongManager.sbr" \
	"$(INTDIR)\StatsManager.sbr" \
	"$(INTDIR)\ThemeManager.sbr" \
	"$(INTDIR)\UnlockManager.sbr" \
	"$(INTDIR)\algebra.sbr" \
	"$(INTDIR)\algparam.sbr" \
	"$(INTDIR)\asn.sbr" \
	"$(INTDIR)\cryptlib.sbr" \
	"$(INTDIR)\files.sbr" \
	"$(INTDIR)\filters.sbr" \
	"$(INTDIR)\integer.sbr" \
	"$(INTDIR)\iterhash.sbr" \
	"$(INTDIR)\misc.sbr" \
	"$(INTDIR)\mqueue.sbr" \
	"$(INTDIR)\nbtheory.sbr" \
	"$(INTDIR)\osrng.sbr" \
	"$(INTDIR)\pkcspad.sbr" \
	"$(INTDIR)\pubkey.sbr" \
	"$(INTDIR)\queue.sbr" \
	"$(INTDIR)\rsa.sbr" \
	"$(INTDIR)\sha.sbr" \
	"$(INTDIR)\CryptBn.sbr" \
	"$(INTDIR)\CryptHelpers.sbr" \
	"$(INTDIR)\CryptMD5.sbr" \
	"$(INTDIR)\CryptNoise.sbr" \
	"$(INTDIR)\CryptPrime.sbr" \
	"$(INTDIR)\CryptRand.sbr" \
	"$(INTDIR)\CryptRSA.sbr" \
	"$(INTDIR)\CryptSH512.sbr" \
	"$(INTDIR)\CryptSHA.sbr"

"..\StepMania-StackTrace.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=$(intdir)\verstub.obj kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib winmm.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\StepMania.pdb" /map:"$(INTDIR)\StepMania.map" /machine:I386 /out:"../Program/StepMania.exe" 
LINK32_OBJS= \
	"$(INTDIR)\get.obj" \
	"$(INTDIR)\maketables.obj" \
	"$(INTDIR)\pcre.obj" \
	"$(INTDIR)\study.obj" \
	"$(INTDIR)\RageBitmapTexture.obj" \
	"$(INTDIR)\RageDisplay.obj" \
	"$(INTDIR)\RageDisplay_D3D.obj" \
	"$(INTDIR)\RageDisplay_Null.obj" \
	"$(INTDIR)\RageDisplay_OGL.obj" \
	"$(INTDIR)\RageDisplay_OGL_Extensions.obj" \
	"$(INTDIR)\RageException.obj" \
	"$(INTDIR)\RageFile.obj" \
	"$(INTDIR)\RageFileBasic.obj" \
	"$(INTDIR)\RageFileDriver.obj" \
	"$(INTDIR)\RageFileDriverDeflate.obj" \
	"$(INTDIR)\RageFileDriverDirect.obj" \
	"$(INTDIR)\RageFileDriverDirectHelpers.obj" \
	"$(INTDIR)\RageFileDriverMemory.obj" \
	"$(INTDIR)\RageFileDriverSlice.obj" \
	"$(INTDIR)\RageFileDriverTimeout.obj" \
	"$(INTDIR)\RageFileDriverZip.obj" \
	"$(INTDIR)\RageFileManager.obj" \
	"$(INTDIR)\RageInput.obj" \
	"$(INTDIR)\RageInputDevice.obj" \
	"$(INTDIR)\RageLog.obj" \
	"$(INTDIR)\RageMath.obj" \
	"$(INTDIR)\RageModelGeometry.obj" \
	"$(INTDIR)\RageSound.obj" \
	"$(INTDIR)\RageSoundManager.obj" \
	"$(INTDIR)\RageSoundMixBuffer.obj" \
	"$(INTDIR)\RageSoundPosMap.obj" \
	"$(INTDIR)\RageSoundReader_Chain.obj" \
	"$(INTDIR)\RageSoundReader_FileReader.obj" \
	"$(INTDIR)\RageSoundReader_MP3.obj" \
	"$(INTDIR)\RageSoundReader_Preload.obj" \
	"$(INTDIR)\RageSoundReader_Resample.obj" \
	"$(INTDIR)\RageSoundReader_Resample_Fast.obj" \
	"$(INTDIR)\RageSoundReader_Resample_Good.obj" \
	"$(INTDIR)\RageSoundReader_Vorbisfile.obj" \
	"$(INTDIR)\RageSoundReader_WAV.obj" \
	"$(INTDIR)\RageSoundResampler.obj" \
	"$(INTDIR)\RageSoundUtil.obj" \
	"$(INTDIR)\RageSurface.obj" \
	"$(INTDIR)\RageSurface_Load.obj" \
	"$(INTDIR)\RageSurface_Load_BMP.obj" \
	"$(INTDIR)\RageSurface_Load_GIF.obj" \
	"$(INTDIR)\RageSurface_Load_JPEG.obj" \
	"$(INTDIR)\RageSurface_Load_PNG.obj" \
	"$(INTDIR)\RageSurface_Load_XPM.obj" \
	"$(INTDIR)\RageSurface_Save_BMP.obj" \
	"$(INTDIR)\RageSurface_Save_JPEG.obj" \
	"$(INTDIR)\RageSurfaceUtils.obj" \
	"$(INTDIR)\RageSurfaceUtils_Dither.obj" \
	"$(INTDIR)\RageSurfaceUtils_Palettize.obj" \
	"$(INTDIR)\RageSurfaceUtils_Zoom.obj" \
	"$(INTDIR)\RageTexture.obj" \
	"$(INTDIR)\RageTextureID.obj" \
	"$(INTDIR)\RageTextureManager.obj" \
	"$(INTDIR)\RageThreads.obj" \
	"$(INTDIR)\RageTimer.obj" \
	"$(INTDIR)\RageUtil.obj" \
	"$(INTDIR)\RageUtil_BackgroundLoader.obj" \
	"$(INTDIR)\RageUtil_CharConversions.obj" \
	"$(INTDIR)\RageUtil_FileDB.obj" \
	"$(INTDIR)\RageUtil_WorkerThread.obj" \
	"$(INTDIR)\ActorCommands.obj" \
	"$(INTDIR)\Attack.obj" \
	"$(INTDIR)\AutoKeysounds.obj" \
	"$(INTDIR)\BannerCache.obj" \
	"$(INTDIR)\CatalogXml.obj" \
	"$(INTDIR)\Character.obj" \
	"$(INTDIR)\CodeDetector.obj" \
	"$(INTDIR)\Command.obj" \
	"$(INTDIR)\CommonMetrics.obj" \
	"$(INTDIR)\Course.obj" \
	"$(INTDIR)\CourseUtil.obj" \
	"$(INTDIR)\DateTime.obj" \
	"$(INTDIR)\Difficulty.obj" \
	"$(INTDIR)\EnumHelper.obj" \
	"$(INTDIR)\Font.obj" \
	"$(INTDIR)\FontCharAliases.obj" \
	"$(INTDIR)\FontCharmaps.obj" \
	"$(INTDIR)\Game.obj" \
	"$(INTDIR)\GameCommand.obj" \
	"$(INTDIR)\GameConstantsAndTypes.obj" \
	"$(INTDIR)\GameInput.obj" \
	"$(INTDIR)\Grade.obj" \
	"$(INTDIR)\HighScore.obj" \
	"$(INTDIR)\Inventory.obj" \
	"$(INTDIR)\LuaReference.obj" \
	"$(INTDIR)\LyricsLoader.obj" \
	"$(INTDIR)\NoteData.obj" \
	"$(INTDIR)\NoteDataUtil.obj" \
	"$(INTDIR)\NoteDataWithScoring.obj" \
	"$(INTDIR)\NoteFieldPositioning.obj" \
	"$(INTDIR)\NotesLoader.obj" \
	"$(INTDIR)\NotesLoaderBMS.obj" \
	"$(INTDIR)\NotesLoaderDWI.obj" \
	"$(INTDIR)\NotesLoaderKSF.obj" \
	"$(INTDIR)\NotesLoaderSM.obj" \
	"$(INTDIR)\NotesWriterDWI.obj" \
	"$(INTDIR)\NotesWriterSM.obj" \
	"$(INTDIR)\NoteTypes.obj" \
	"$(INTDIR)\OptionRowHandler.obj" \
	"$(INTDIR)\PlayerAI.obj" \
	"$(INTDIR)\PlayerNumber.obj" \
	"$(INTDIR)\PlayerOptions.obj" \
	"$(INTDIR)\PlayerStageStats.obj" \
	"$(INTDIR)\PlayerState.obj" \
	"$(INTDIR)\Preference.obj" \
	"$(INTDIR)\Profile.obj" \
	"$(INTDIR)\RadarValues.obj" \
	"$(INTDIR)\RandomSample.obj" \
	"$(INTDIR)\ScoreKeeperMAX2.obj" \
	"$(INTDIR)\ScoreKeeperRave.obj" \
	"$(INTDIR)\ScreenDimensions.obj" \
	"$(INTDIR)\Song.obj" \
	"$(INTDIR)\SongCacheIndex.obj" \
	"$(INTDIR)\SongOptions.obj" \
	"$(INTDIR)\SongUtil.obj" \
	"$(INTDIR)\StageStats.obj" \
	"$(INTDIR)\Steps.obj" \
	"$(INTDIR)\StepsUtil.obj" \
	"$(INTDIR)\Style.obj" \
	"$(INTDIR)\StyleUtil.obj" \
	"$(INTDIR)\TimingData.obj" \
	"$(INTDIR)\TitleSubstitution.obj" \
	"$(INTDIR)\Trail.obj" \
	"$(INTDIR)\TrailUtil.obj" \
	"$(INTDIR)\IniFile.obj" \
	"$(INTDIR)\MsdFile.obj" \
	"$(INTDIR)\XmlFile.obj" \
	"$(INTDIR)\LoadingWindow_Win32.obj" \
	"$(INTDIR)\DSoundHelpers.obj" \
	"$(INTDIR)\RageSoundDriver_DSound.obj" \
	"$(INTDIR)\RageSoundDriver_DSound_Software.obj" \
	"$(INTDIR)\RageSoundDriver_Generic_Software.obj" \
	"$(INTDIR)\RageSoundDriver_Null.obj" \
	"$(INTDIR)\RageSoundDriver_WaveOut.obj" \
	"$(INTDIR)\ArchHooks.obj" \
	"$(INTDIR)\ArchHooks_Win32.obj" \
	"$(INTDIR)\InputHandler.obj" \
	"$(INTDIR)\InputHandler_DirectInput.obj" \
	"$(INTDIR)\InputHandler_DirectInputHelper.obj" \
	"$(INTDIR)\InputHandler_MonkeyKeyboard.obj" \
	"$(INTDIR)\InputHandler_Win32_Para.obj" \
	"$(INTDIR)\InputHandler_Win32_Pump.obj" \
	"$(INTDIR)\MovieTexture.obj" \
	"$(INTDIR)\MovieTexture_DShow.obj" \
	"$(INTDIR)\MovieTexture_DShowHelper.obj" \
	"$(INTDIR)\MovieTexture_FFMpeg.obj" \
	"$(INTDIR)\MovieTexture_Null.obj" \
	"$(INTDIR)\LowLevelWindow_Win32.obj" \
	"$(INTDIR)\LightsDriver_SystemMessage.obj" \
	"$(INTDIR)\LightsDriver_Win32Parallel.obj" \
	"$(INTDIR)\MemoryCardDriver.obj" \
	"$(INTDIR)\MemoryCardDriverThreaded_Windows.obj" \
	"$(INTDIR)\Dialog.obj" \
	"$(INTDIR)\DialogDriver_Win32.obj" \
	"$(INTDIR)\Threads_Win32.obj" \
	"$(INTDIR)\arch.obj" \
	"$(INTDIR)\AppInstance.obj" \
	"$(INTDIR)\arch_setup.obj" \
	"$(INTDIR)\Crash.obj" \
	"$(INTDIR)\DebugInfoHunt.obj" \
	"$(INTDIR)\GetFileInformation.obj" \
	"$(INTDIR)\GotoURL.obj" \
	"$(INTDIR)\GraphicsWindow.obj" \
	"$(INTDIR)\RegistryAccess.obj" \
	"$(INTDIR)\RestartProgram.obj" \
	"$(INTDIR)\USB.obj" \
	"$(INTDIR)\VideoDriverInfo.obj" \
	"$(INTDIR)\WindowIcon.obj" \
	"$(INTDIR)\global.obj" \
	"$(INTDIR)\StepMania.obj" \
	"$(INTDIR)\Actor.obj" \
	"$(INTDIR)\ActorFrame.obj" \
	"$(INTDIR)\ActorScroller.obj" \
	"$(INTDIR)\ActorUtil.obj" \
	"$(INTDIR)\AutoActor.obj" \
	"$(INTDIR)\BitmapText.obj" \
	"$(INTDIR)\Model.obj" \
	"$(INTDIR)\ModelManager.obj" \
	"$(INTDIR)\ModelTypes.obj" \
	"$(INTDIR)\Quad.obj" \
	"$(INTDIR)\RollingNumbers.obj" \
	"$(INTDIR)\Sprite.obj" \
	"$(INTDIR)\Banner.obj" \
	"$(INTDIR)\BGAnimation.obj" \
	"$(INTDIR)\BGAnimationLayer.obj" \
	"$(INTDIR)\DifficultyIcon.obj" \
	"$(INTDIR)\FadingBanner.obj" \
	"$(INTDIR)\MeterDisplay.obj" \
	"$(INTDIR)\StreamDisplay.obj" \
	"$(INTDIR)\Transition.obj" \
	"$(INTDIR)\ActiveAttackList.obj" \
	"$(INTDIR)\ArrowEffects.obj" \
	"$(INTDIR)\AttackDisplay.obj" \
	"$(INTDIR)\Background.obj" \
	"$(INTDIR)\BeginnerHelper.obj" \
	"$(INTDIR)\CharacterHead.obj" \
	"$(INTDIR)\CombinedLifeMeterTug.obj" \
	"$(INTDIR)\Combo.obj" \
	"$(INTDIR)\DancingCharacters.obj" \
	"$(INTDIR)\Foreground.obj" \
	"$(INTDIR)\GhostArrow.obj" \
	"$(INTDIR)\GhostArrowRow.obj" \
	"$(INTDIR)\HoldGhostArrow.obj" \
	"$(INTDIR)\HoldJudgment.obj" \
	"$(INTDIR)\Judgment.obj" \
	"$(INTDIR)\LifeMeterBar.obj" \
	"$(INTDIR)\LifeMeterBattery.obj" \
	"$(INTDIR)\LifeMeterTime.obj" \
	"$(INTDIR)\LyricDisplay.obj" \
	"$(INTDIR)\NoteDisplay.obj" \
	"$(INTDIR)\NoteField.obj" \
	"$(INTDIR)\PercentageDisplay.obj" \
	"$(INTDIR)\Player.obj" \
	"$(INTDIR)\ReceptorArrow.obj" \
	"$(INTDIR)\ReceptorArrowRow.obj" \
	"$(INTDIR)\ScoreDisplay.obj" \
	"$(INTDIR)\ScoreDisplayAliveTime.obj" \
	"$(INTDIR)\ScoreDisplayBattle.obj" \
	"$(INTDIR)\ScoreDisplayCalories.obj" \
	"$(INTDIR)\ScoreDisplayLifeTime.obj" \
	"$(INTDIR)\ScoreDisplayNormal.obj" \
	"$(INTDIR)\ScoreDisplayOni.obj" \
	"$(INTDIR)\ScoreDisplayPercentage.obj" \
	"$(INTDIR)\ScoreDisplayRave.obj" \
	"$(INTDIR)\BPMDisplay.obj" \
	"$(INTDIR)\ComboGraph.obj" \
	"$(INTDIR)\CourseContentsList.obj" \
	"$(INTDIR)\CourseEntryDisplay.obj" \
	"$(INTDIR)\DifficultyDisplay.obj" \
	"$(INTDIR)\DifficultyList.obj" \
	"$(INTDIR)\DifficultyMeter.obj" \
	"$(INTDIR)\DifficultyRating.obj" \
	"$(INTDIR)\DualScrollBar.obj" \
	"$(INTDIR)\EditCoursesMenu.obj" \
	"$(INTDIR)\EditCoursesSongMenu.obj" \
	"$(INTDIR)\EditMenu.obj" \
	"$(INTDIR)\GradeDisplay.obj" \
	"$(INTDIR)\GraphDisplay.obj" \
	"$(INTDIR)\GrooveGraph.obj" \
	"$(INTDIR)\GrooveRadar.obj" \
	"$(INTDIR)\GroupList.obj" \
	"$(INTDIR)\HelpDisplay.obj" \
	"$(INTDIR)\MemoryCardDisplay.obj" \
	"$(INTDIR)\MenuTimer.obj" \
	"$(INTDIR)\ModeSwitcher.obj" \
	"$(INTDIR)\MusicBannerWheel.obj" \
	"$(INTDIR)\MusicList.obj" \
	"$(INTDIR)\MusicSortDisplay.obj" \
	"$(INTDIR)\MusicWheel.obj" \
	"$(INTDIR)\MusicWheelItem.obj" \
	"$(INTDIR)\OptionIcon.obj" \
	"$(INTDIR)\OptionIconRow.obj" \
	"$(INTDIR)\OptionRow.obj" \
	"$(INTDIR)\OptionsCursor.obj" \
	"$(INTDIR)\PaneDisplay.obj" \
	"$(INTDIR)\RoomWheel.obj" \
	"$(INTDIR)\ScrollBar.obj" \
	"$(INTDIR)\ScrollingList.obj" \
	"$(INTDIR)\SnapDisplay.obj" \
	"$(INTDIR)\TextBanner.obj" \
	"$(INTDIR)\WheelBase.obj" \
	"$(INTDIR)\WheelItemBase.obj" \
	"$(INTDIR)\WheelNotifyIcon.obj" \
	"$(INTDIR)\Screen.obj" \
	"$(INTDIR)\ScreenAttract.obj" \
	"$(INTDIR)\ScreenBookkeeping.obj" \
	"$(INTDIR)\ScreenBranch.obj" \
	"$(INTDIR)\ScreenCenterImage.obj" \
	"$(INTDIR)\ScreenCredits.obj" \
	"$(INTDIR)\ScreenDebugOverlay.obj" \
	"$(INTDIR)\ScreenDemonstration.obj" \
	"$(INTDIR)\ScreenEdit.obj" \
	"$(INTDIR)\ScreenEditCoursesMenu.obj" \
	"$(INTDIR)\ScreenEditMenu.obj" \
	"$(INTDIR)\ScreenEnding.obj" \
	"$(INTDIR)\ScreenEndlessBreak.obj" \
	"$(INTDIR)\ScreenEvaluation.obj" \
	"$(INTDIR)\ScreenExit.obj" \
	"$(INTDIR)\ScreenEz2SelectMusic.obj" \
	"$(INTDIR)\ScreenEz2SelectPlayer.obj" \
	"$(INTDIR)\ScreenGameplay.obj" \
	"$(INTDIR)\ScreenGameplayMultiplayer.obj" \
	"$(INTDIR)\ScreenHowToPlay.obj" \
	"$(INTDIR)\ScreenInstructions.obj" \
	"$(INTDIR)\ScreenJukebox.obj" \
	"$(INTDIR)\ScreenLogo.obj" \
	"$(INTDIR)\ScreenMapControllers.obj" \
	"$(INTDIR)\ScreenMessage.obj" \
	"$(INTDIR)\ScreenMiniMenu.obj" \
	"$(INTDIR)\ScreenMusicScroll.obj" \
	"$(INTDIR)\ScreenNameEntry.obj" \
	"$(INTDIR)\ScreenNameEntryTraditional.obj" \
	"$(INTDIR)\ScreenNetEvaluation.obj" \
	"$(INTDIR)\ScreenNetRoom.obj" \
	"$(INTDIR)\ScreenNetSelectBase.obj" \
	"$(INTDIR)\ScreenNetSelectMusic.obj" \
	"$(INTDIR)\ScreenNetworkOptions.obj" \
	"$(INTDIR)\ScreenOptions.obj" \
	"$(INTDIR)\ScreenOptionsMaster.obj" \
	"$(INTDIR)\ScreenOptionsMasterPrefs.obj" \
	"$(INTDIR)\ScreenPackages.obj" \
	"$(INTDIR)\ScreenPlayerOptions.obj" \
	"$(INTDIR)\ScreenProfileOptions.obj" \
	"$(INTDIR)\ScreenPrompt.obj" \
	"$(INTDIR)\ScreenRanking.obj" \
	"$(INTDIR)\ScreenReloadSongs.obj" \
	"$(INTDIR)\ScreenSandbox.obj" \
	"$(INTDIR)\ScreenSelect.obj" \
	"$(INTDIR)\ScreenSelectCharacter.obj" \
	"$(INTDIR)\ScreenSelectDifficulty.obj" \
	"$(INTDIR)\ScreenSelectGroup.obj" \
	"$(INTDIR)\ScreenSelectMaster.obj" \
	"$(INTDIR)\ScreenSelectMode.obj" \
	"$(INTDIR)\ScreenSelectMusic.obj" \
	"$(INTDIR)\ScreenSelectStyle.obj" \
	"$(INTDIR)\ScreenSetTime.obj" \
	"$(INTDIR)\ScreenSMOnlineLogin.obj" \
	"$(INTDIR)\ScreenSongOptions.obj" \
	"$(INTDIR)\ScreenSplash.obj" \
	"$(INTDIR)\ScreenStage.obj" \
	"$(INTDIR)\ScreenStyleSplash.obj" \
	"$(INTDIR)\ScreenSystemLayer.obj" \
	"$(INTDIR)\ScreenTest.obj" \
	"$(INTDIR)\ScreenTestFonts.obj" \
	"$(INTDIR)\ScreenTestInput.obj" \
	"$(INTDIR)\ScreenTestLights.obj" \
	"$(INTDIR)\ScreenTestSound.obj" \
	"$(INTDIR)\ScreenTextEntry.obj" \
	"$(INTDIR)\ScreenTitleMenu.obj" \
	"$(INTDIR)\ScreenUnlock.obj" \
	"$(INTDIR)\ScreenWithMenuElements.obj" \
	"$(INTDIR)\AnnouncerManager.obj" \
	"$(INTDIR)\Bookkeeper.obj" \
	"$(INTDIR)\CryptManager.obj" \
	"$(INTDIR)\ezsockets.obj" \
	"$(INTDIR)\FontManager.obj" \
	"$(INTDIR)\GameManager.obj" \
	"$(INTDIR)\GameSoundManager.obj" \
	"$(INTDIR)\GameState.obj" \
	"$(INTDIR)\InputFilter.obj" \
	"$(INTDIR)\InputMapper.obj" \
	"$(INTDIR)\InputQueue.obj" \
	"$(INTDIR)\LightsManager.obj" \
	"$(INTDIR)\LuaManager.obj" \
	"$(INTDIR)\MemoryCardManager.obj" \
	"$(INTDIR)\MessageManager.obj" \
	"$(INTDIR)\NetworkSyncManager.obj" \
	"$(INTDIR)\NetworkSyncServer.obj" \
	"$(INTDIR)\NoteSkinManager.obj" \
	"$(INTDIR)\PrefsManager.obj" \
	"$(INTDIR)\ProfileManager.obj" \
	"$(INTDIR)\ScreenManager.obj" \
	"$(INTDIR)\SongManager.obj" \
	"$(INTDIR)\StatsManager.obj" \
	"$(INTDIR)\ThemeManager.obj" \
	"$(INTDIR)\UnlockManager.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\CryptBn.obj" \
	"$(INTDIR)\CryptHelpers.obj" \
	"$(INTDIR)\CryptMD5.obj" \
	"$(INTDIR)\CryptNoise.obj" \
	"$(INTDIR)\CryptPrime.obj" \
	"$(INTDIR)\CryptRand.obj" \
	"$(INTDIR)\CryptRSA.obj" \
	"$(INTDIR)\CryptSH512.obj" \
	"$(INTDIR)\CryptSHA.obj" \
	"$(INTDIR)\WindowsResources.res"

"..\Program\StepMania.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
   archutils\Win32\verinc
	cl /Zl /nologo /c verstub.cpp  /Fo.\..\Release6\

	 $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

IntDir=.\..\Release6
TargetDir=\stepmania\Program
TargetName=StepMania
SOURCE="$(InputPath)"
DS_POSTBUILD_DEP=$(INTDIR)\postbld.dep

ALL : $(DS_POSTBUILD_DEP)

$(DS_POSTBUILD_DEP) : "..\Program\StepMania.exe" "..\StepMania-StackTrace.bsc"
   archutils\Win32\mapconv .\..\Release6\StepMania.map \stepmania\Program\StepMania.vdi
	echo Helper for Post-build step > "$(DS_POSTBUILD_DEP)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("StepMania.dep")
!INCLUDE "StepMania.dep"
!ELSE 
!MESSAGE Warning: cannot find "StepMania.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "StepMania - Win32 Debug" || "$(CFG)" == "StepMania - Win32 Release"
SOURCE=.\pcre\get.c

"$(INTDIR)\get.obj"	"$(INTDIR)\get.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\pcre\maketables.c

"$(INTDIR)\maketables.obj"	"$(INTDIR)\maketables.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\pcre\pcre.c

"$(INTDIR)\pcre.obj"	"$(INTDIR)\pcre.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\pcre\study.c

"$(INTDIR)\study.obj"	"$(INTDIR)\study.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\RageBitmapTexture.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX"global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\RageBitmapTexture.obj"	"$(INTDIR)\RageBitmapTexture.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\RageBitmapTexture.obj"	"$(INTDIR)\RageBitmapTexture.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\RageDisplay.cpp

"$(INTDIR)\RageDisplay.obj"	"$(INTDIR)\RageDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageDisplay_D3D.cpp

"$(INTDIR)\RageDisplay_D3D.obj"	"$(INTDIR)\RageDisplay_D3D.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageDisplay_Null.cpp

"$(INTDIR)\RageDisplay_Null.obj"	"$(INTDIR)\RageDisplay_Null.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageDisplay_OGL.cpp

"$(INTDIR)\RageDisplay_OGL.obj"	"$(INTDIR)\RageDisplay_OGL.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageDisplay_OGL_Extensions.cpp

"$(INTDIR)\RageDisplay_OGL_Extensions.obj"	"$(INTDIR)\RageDisplay_OGL_Extensions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageException.cpp

"$(INTDIR)\RageException.obj"	"$(INTDIR)\RageException.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFile.cpp

"$(INTDIR)\RageFile.obj"	"$(INTDIR)\RageFile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileBasic.cpp

"$(INTDIR)\RageFileBasic.obj"	"$(INTDIR)\RageFileBasic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriver.cpp

"$(INTDIR)\RageFileDriver.obj"	"$(INTDIR)\RageFileDriver.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverDeflate.cpp

"$(INTDIR)\RageFileDriverDeflate.obj"	"$(INTDIR)\RageFileDriverDeflate.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverDirect.cpp

"$(INTDIR)\RageFileDriverDirect.obj"	"$(INTDIR)\RageFileDriverDirect.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverDirectHelpers.cpp

"$(INTDIR)\RageFileDriverDirectHelpers.obj"	"$(INTDIR)\RageFileDriverDirectHelpers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverMemory.cpp

"$(INTDIR)\RageFileDriverMemory.obj"	"$(INTDIR)\RageFileDriverMemory.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverSlice.cpp

"$(INTDIR)\RageFileDriverSlice.obj"	"$(INTDIR)\RageFileDriverSlice.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverTimeout.cpp

"$(INTDIR)\RageFileDriverTimeout.obj"	"$(INTDIR)\RageFileDriverTimeout.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileDriverZip.cpp

"$(INTDIR)\RageFileDriverZip.obj"	"$(INTDIR)\RageFileDriverZip.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageFileManager.cpp

"$(INTDIR)\RageFileManager.obj"	"$(INTDIR)\RageFileManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageInput.cpp

"$(INTDIR)\RageInput.obj"	"$(INTDIR)\RageInput.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageInputDevice.cpp

"$(INTDIR)\RageInputDevice.obj"	"$(INTDIR)\RageInputDevice.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageLog.cpp

"$(INTDIR)\RageLog.obj"	"$(INTDIR)\RageLog.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageMath.cpp

"$(INTDIR)\RageMath.obj"	"$(INTDIR)\RageMath.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageModelGeometry.cpp

"$(INTDIR)\RageModelGeometry.obj"	"$(INTDIR)\RageModelGeometry.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSound.cpp

"$(INTDIR)\RageSound.obj"	"$(INTDIR)\RageSound.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundManager.cpp

"$(INTDIR)\RageSoundManager.obj"	"$(INTDIR)\RageSoundManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundMixBuffer.cpp

"$(INTDIR)\RageSoundMixBuffer.obj"	"$(INTDIR)\RageSoundMixBuffer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundPosMap.cpp

"$(INTDIR)\RageSoundPosMap.obj"	"$(INTDIR)\RageSoundPosMap.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Chain.cpp

"$(INTDIR)\RageSoundReader_Chain.obj"	"$(INTDIR)\RageSoundReader_Chain.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_FileReader.cpp

"$(INTDIR)\RageSoundReader_FileReader.obj"	"$(INTDIR)\RageSoundReader_FileReader.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_MP3.cpp

"$(INTDIR)\RageSoundReader_MP3.obj"	"$(INTDIR)\RageSoundReader_MP3.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Preload.cpp

"$(INTDIR)\RageSoundReader_Preload.obj"	"$(INTDIR)\RageSoundReader_Preload.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Resample.cpp

"$(INTDIR)\RageSoundReader_Resample.obj"	"$(INTDIR)\RageSoundReader_Resample.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Resample_Fast.cpp

"$(INTDIR)\RageSoundReader_Resample_Fast.obj"	"$(INTDIR)\RageSoundReader_Resample_Fast.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Resample_Good.cpp

"$(INTDIR)\RageSoundReader_Resample_Good.obj"	"$(INTDIR)\RageSoundReader_Resample_Good.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_Vorbisfile.cpp

"$(INTDIR)\RageSoundReader_Vorbisfile.obj"	"$(INTDIR)\RageSoundReader_Vorbisfile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundReader_WAV.cpp

"$(INTDIR)\RageSoundReader_WAV.obj"	"$(INTDIR)\RageSoundReader_WAV.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundResampler.cpp

"$(INTDIR)\RageSoundResampler.obj"	"$(INTDIR)\RageSoundResampler.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSoundUtil.cpp

"$(INTDIR)\RageSoundUtil.obj"	"$(INTDIR)\RageSoundUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface.cpp

"$(INTDIR)\RageSurface.obj"	"$(INTDIR)\RageSurface.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load.cpp

"$(INTDIR)\RageSurface_Load.obj"	"$(INTDIR)\RageSurface_Load.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load_BMP.cpp

"$(INTDIR)\RageSurface_Load_BMP.obj"	"$(INTDIR)\RageSurface_Load_BMP.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load_GIF.cpp

"$(INTDIR)\RageSurface_Load_GIF.obj"	"$(INTDIR)\RageSurface_Load_GIF.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load_JPEG.cpp

"$(INTDIR)\RageSurface_Load_JPEG.obj"	"$(INTDIR)\RageSurface_Load_JPEG.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load_PNG.cpp

"$(INTDIR)\RageSurface_Load_PNG.obj"	"$(INTDIR)\RageSurface_Load_PNG.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Load_XPM.cpp

"$(INTDIR)\RageSurface_Load_XPM.obj"	"$(INTDIR)\RageSurface_Load_XPM.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Save_BMP.cpp

"$(INTDIR)\RageSurface_Save_BMP.obj"	"$(INTDIR)\RageSurface_Save_BMP.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurface_Save_JPEG.cpp

"$(INTDIR)\RageSurface_Save_JPEG.obj"	"$(INTDIR)\RageSurface_Save_JPEG.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurfaceUtils.cpp

"$(INTDIR)\RageSurfaceUtils.obj"	"$(INTDIR)\RageSurfaceUtils.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurfaceUtils_Dither.cpp

"$(INTDIR)\RageSurfaceUtils_Dither.obj"	"$(INTDIR)\RageSurfaceUtils_Dither.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurfaceUtils_Palettize.cpp

"$(INTDIR)\RageSurfaceUtils_Palettize.obj"	"$(INTDIR)\RageSurfaceUtils_Palettize.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageSurfaceUtils_Zoom.cpp

"$(INTDIR)\RageSurfaceUtils_Zoom.obj"	"$(INTDIR)\RageSurfaceUtils_Zoom.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageTexture.cpp

"$(INTDIR)\RageTexture.obj"	"$(INTDIR)\RageTexture.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageTextureID.cpp

"$(INTDIR)\RageTextureID.obj"	"$(INTDIR)\RageTextureID.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageTextureManager.cpp

"$(INTDIR)\RageTextureManager.obj"	"$(INTDIR)\RageTextureManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageThreads.cpp

"$(INTDIR)\RageThreads.obj"	"$(INTDIR)\RageThreads.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageTimer.cpp

"$(INTDIR)\RageTimer.obj"	"$(INTDIR)\RageTimer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageUtil.cpp

"$(INTDIR)\RageUtil.obj"	"$(INTDIR)\RageUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageUtil_BackgroundLoader.cpp

"$(INTDIR)\RageUtil_BackgroundLoader.obj"	"$(INTDIR)\RageUtil_BackgroundLoader.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageUtil_CharConversions.cpp

"$(INTDIR)\RageUtil_CharConversions.obj"	"$(INTDIR)\RageUtil_CharConversions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageUtil_FileDB.cpp

"$(INTDIR)\RageUtil_FileDB.obj"	"$(INTDIR)\RageUtil_FileDB.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RageUtil_WorkerThread.cpp

"$(INTDIR)\RageUtil_WorkerThread.obj"	"$(INTDIR)\RageUtil_WorkerThread.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActorCommands.cpp

"$(INTDIR)\ActorCommands.obj"	"$(INTDIR)\ActorCommands.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Attack.cpp

"$(INTDIR)\Attack.obj"	"$(INTDIR)\Attack.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\AutoKeysounds.cpp

"$(INTDIR)\AutoKeysounds.obj"	"$(INTDIR)\AutoKeysounds.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BannerCache.cpp

"$(INTDIR)\BannerCache.obj"	"$(INTDIR)\BannerCache.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CatalogXml.cpp

"$(INTDIR)\CatalogXml.obj"	"$(INTDIR)\CatalogXml.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Character.cpp

"$(INTDIR)\Character.obj"	"$(INTDIR)\Character.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CodeDetector.cpp

"$(INTDIR)\CodeDetector.obj"	"$(INTDIR)\CodeDetector.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Command.cpp

"$(INTDIR)\Command.obj"	"$(INTDIR)\Command.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CommonMetrics.cpp

"$(INTDIR)\CommonMetrics.obj"	"$(INTDIR)\CommonMetrics.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Course.cpp

"$(INTDIR)\Course.obj"	"$(INTDIR)\Course.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CourseUtil.cpp

"$(INTDIR)\CourseUtil.obj"	"$(INTDIR)\CourseUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DateTime.cpp

"$(INTDIR)\DateTime.obj"	"$(INTDIR)\DateTime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Difficulty.cpp

"$(INTDIR)\Difficulty.obj"	"$(INTDIR)\Difficulty.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\EnumHelper.cpp

"$(INTDIR)\EnumHelper.obj"	"$(INTDIR)\EnumHelper.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Font.cpp

"$(INTDIR)\Font.obj"	"$(INTDIR)\Font.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FontCharAliases.cpp

"$(INTDIR)\FontCharAliases.obj"	"$(INTDIR)\FontCharAliases.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FontCharmaps.cpp

"$(INTDIR)\FontCharmaps.obj"	"$(INTDIR)\FontCharmaps.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Game.cpp

"$(INTDIR)\Game.obj"	"$(INTDIR)\Game.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameCommand.cpp

"$(INTDIR)\GameCommand.obj"	"$(INTDIR)\GameCommand.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameConstantsAndTypes.cpp

"$(INTDIR)\GameConstantsAndTypes.obj"	"$(INTDIR)\GameConstantsAndTypes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameInput.cpp

"$(INTDIR)\GameInput.obj"	"$(INTDIR)\GameInput.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Grade.cpp

"$(INTDIR)\Grade.obj"	"$(INTDIR)\Grade.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HighScore.cpp

"$(INTDIR)\HighScore.obj"	"$(INTDIR)\HighScore.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Inventory.cpp

"$(INTDIR)\Inventory.obj"	"$(INTDIR)\Inventory.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LuaReference.cpp

"$(INTDIR)\LuaReference.obj"	"$(INTDIR)\LuaReference.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LyricsLoader.cpp

"$(INTDIR)\LyricsLoader.obj"	"$(INTDIR)\LyricsLoader.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteData.cpp

"$(INTDIR)\NoteData.obj"	"$(INTDIR)\NoteData.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteDataUtil.cpp

"$(INTDIR)\NoteDataUtil.obj"	"$(INTDIR)\NoteDataUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteDataWithScoring.cpp

"$(INTDIR)\NoteDataWithScoring.obj"	"$(INTDIR)\NoteDataWithScoring.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteFieldPositioning.cpp

"$(INTDIR)\NoteFieldPositioning.obj"	"$(INTDIR)\NoteFieldPositioning.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesLoader.cpp

"$(INTDIR)\NotesLoader.obj"	"$(INTDIR)\NotesLoader.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesLoaderBMS.cpp

"$(INTDIR)\NotesLoaderBMS.obj"	"$(INTDIR)\NotesLoaderBMS.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesLoaderDWI.cpp

"$(INTDIR)\NotesLoaderDWI.obj"	"$(INTDIR)\NotesLoaderDWI.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesLoaderKSF.cpp

"$(INTDIR)\NotesLoaderKSF.obj"	"$(INTDIR)\NotesLoaderKSF.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesLoaderSM.cpp

"$(INTDIR)\NotesLoaderSM.obj"	"$(INTDIR)\NotesLoaderSM.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesWriterDWI.cpp

"$(INTDIR)\NotesWriterDWI.obj"	"$(INTDIR)\NotesWriterDWI.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NotesWriterSM.cpp

"$(INTDIR)\NotesWriterSM.obj"	"$(INTDIR)\NotesWriterSM.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteTypes.cpp

"$(INTDIR)\NoteTypes.obj"	"$(INTDIR)\NoteTypes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OptionRowHandler.cpp

"$(INTDIR)\OptionRowHandler.obj"	"$(INTDIR)\OptionRowHandler.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayerAI.cpp

"$(INTDIR)\PlayerAI.obj"	"$(INTDIR)\PlayerAI.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayerNumber.cpp

"$(INTDIR)\PlayerNumber.obj"	"$(INTDIR)\PlayerNumber.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayerOptions.cpp

"$(INTDIR)\PlayerOptions.obj"	"$(INTDIR)\PlayerOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayerStageStats.cpp

"$(INTDIR)\PlayerStageStats.obj"	"$(INTDIR)\PlayerStageStats.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayerState.cpp

"$(INTDIR)\PlayerState.obj"	"$(INTDIR)\PlayerState.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Preference.cpp

"$(INTDIR)\Preference.obj"	"$(INTDIR)\Preference.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Profile.cpp

"$(INTDIR)\Profile.obj"	"$(INTDIR)\Profile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RadarValues.cpp

"$(INTDIR)\RadarValues.obj"	"$(INTDIR)\RadarValues.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RandomSample.cpp

"$(INTDIR)\RandomSample.obj"	"$(INTDIR)\RandomSample.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreKeeperMAX2.cpp

"$(INTDIR)\ScoreKeeperMAX2.obj"	"$(INTDIR)\ScoreKeeperMAX2.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreKeeperRave.cpp

"$(INTDIR)\ScoreKeeperRave.obj"	"$(INTDIR)\ScoreKeeperRave.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenDimensions.cpp

"$(INTDIR)\ScreenDimensions.obj"	"$(INTDIR)\ScreenDimensions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Song.cpp

"$(INTDIR)\Song.obj"	"$(INTDIR)\Song.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SongCacheIndex.cpp

"$(INTDIR)\SongCacheIndex.obj"	"$(INTDIR)\SongCacheIndex.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SongOptions.cpp

"$(INTDIR)\SongOptions.obj"	"$(INTDIR)\SongOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SongUtil.cpp

"$(INTDIR)\SongUtil.obj"	"$(INTDIR)\SongUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StageStats.cpp

"$(INTDIR)\StageStats.obj"	"$(INTDIR)\StageStats.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Steps.cpp

"$(INTDIR)\Steps.obj"	"$(INTDIR)\Steps.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StepsUtil.cpp

"$(INTDIR)\StepsUtil.obj"	"$(INTDIR)\StepsUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Style.cpp

"$(INTDIR)\Style.obj"	"$(INTDIR)\Style.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StyleUtil.cpp

"$(INTDIR)\StyleUtil.obj"	"$(INTDIR)\StyleUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TimingData.cpp

"$(INTDIR)\TimingData.obj"	"$(INTDIR)\TimingData.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TitleSubstitution.cpp

"$(INTDIR)\TitleSubstitution.obj"	"$(INTDIR)\TitleSubstitution.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Trail.cpp

"$(INTDIR)\Trail.obj"	"$(INTDIR)\Trail.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TrailUtil.cpp

"$(INTDIR)\TrailUtil.obj"	"$(INTDIR)\TrailUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\IniFile.cpp

"$(INTDIR)\IniFile.obj"	"$(INTDIR)\IniFile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MsdFile.cpp

"$(INTDIR)\MsdFile.obj"	"$(INTDIR)\MsdFile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\XmlFile.cpp

"$(INTDIR)\XmlFile.obj"	"$(INTDIR)\XmlFile.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\arch\LoadingWindow\LoadingWindow_Win32.cpp

"$(INTDIR)\LoadingWindow_Win32.obj"	"$(INTDIR)\LoadingWindow_Win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\DSoundHelpers.cpp

"$(INTDIR)\DSoundHelpers.obj"	"$(INTDIR)\DSoundHelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\RageSoundDriver_DSound.cpp

"$(INTDIR)\RageSoundDriver_DSound.obj"	"$(INTDIR)\RageSoundDriver_DSound.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\RageSoundDriver_DSound_Software.cpp

"$(INTDIR)\RageSoundDriver_DSound_Software.obj"	"$(INTDIR)\RageSoundDriver_DSound_Software.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\RageSoundDriver_Generic_Software.cpp

"$(INTDIR)\RageSoundDriver_Generic_Software.obj"	"$(INTDIR)\RageSoundDriver_Generic_Software.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\RageSoundDriver_Null.cpp

"$(INTDIR)\RageSoundDriver_Null.obj"	"$(INTDIR)\RageSoundDriver_Null.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Sound\RageSoundDriver_WaveOut.cpp

"$(INTDIR)\RageSoundDriver_WaveOut.obj"	"$(INTDIR)\RageSoundDriver_WaveOut.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\ArchHooks\ArchHooks.cpp

"$(INTDIR)\ArchHooks.obj"	"$(INTDIR)\ArchHooks.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\ArchHooks\ArchHooks_Win32.cpp

"$(INTDIR)\ArchHooks_Win32.obj"	"$(INTDIR)\ArchHooks_Win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler.cpp

"$(INTDIR)\InputHandler.obj"	"$(INTDIR)\InputHandler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler_DirectInput.cpp

"$(INTDIR)\InputHandler_DirectInput.obj"	"$(INTDIR)\InputHandler_DirectInput.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler_DirectInputHelper.cpp

"$(INTDIR)\InputHandler_DirectInputHelper.obj"	"$(INTDIR)\InputHandler_DirectInputHelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler_MonkeyKeyboard.cpp

"$(INTDIR)\InputHandler_MonkeyKeyboard.obj"	"$(INTDIR)\InputHandler_MonkeyKeyboard.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler_Win32_Para.cpp

"$(INTDIR)\InputHandler_Win32_Para.obj"	"$(INTDIR)\InputHandler_Win32_Para.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\InputHandler\InputHandler_Win32_Pump.cpp

"$(INTDIR)\InputHandler_Win32_Pump.obj"	"$(INTDIR)\InputHandler_Win32_Pump.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MovieTexture\MovieTexture.cpp

"$(INTDIR)\MovieTexture.obj"	"$(INTDIR)\MovieTexture.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MovieTexture\MovieTexture_DShow.cpp

"$(INTDIR)\MovieTexture_DShow.obj"	"$(INTDIR)\MovieTexture_DShow.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MovieTexture\MovieTexture_DShowHelper.cpp

"$(INTDIR)\MovieTexture_DShowHelper.obj"	"$(INTDIR)\MovieTexture_DShowHelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MovieTexture\MovieTexture_FFMpeg.cpp

"$(INTDIR)\MovieTexture_FFMpeg.obj"	"$(INTDIR)\MovieTexture_FFMpeg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MovieTexture\MovieTexture_Null.cpp

"$(INTDIR)\MovieTexture_Null.obj"	"$(INTDIR)\MovieTexture_Null.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\LowLevelWindow\LowLevelWindow_Win32.cpp

"$(INTDIR)\LowLevelWindow_Win32.obj"	"$(INTDIR)\LowLevelWindow_Win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Lights\LightsDriver_SystemMessage.cpp

"$(INTDIR)\LightsDriver_SystemMessage.obj"	"$(INTDIR)\LightsDriver_SystemMessage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Lights\LightsDriver_Win32Parallel.cpp

"$(INTDIR)\LightsDriver_Win32Parallel.obj"	"$(INTDIR)\LightsDriver_Win32Parallel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MemoryCard\MemoryCardDriver.cpp

"$(INTDIR)\MemoryCardDriver.obj"	"$(INTDIR)\MemoryCardDriver.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\MemoryCard\MemoryCardDriverThreaded_Windows.cpp

"$(INTDIR)\MemoryCardDriverThreaded_Windows.obj"	"$(INTDIR)\MemoryCardDriverThreaded_Windows.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Dialog\Dialog.cpp

"$(INTDIR)\Dialog.obj"	"$(INTDIR)\Dialog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Dialog\DialogDriver_Win32.cpp

"$(INTDIR)\DialogDriver_Win32.obj"	"$(INTDIR)\DialogDriver_Win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\Threads\Threads_Win32.cpp

"$(INTDIR)\Threads_Win32.obj"	"$(INTDIR)\Threads_Win32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\arch\arch.cpp

"$(INTDIR)\arch.obj"	"$(INTDIR)\arch.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\AppInstance.cpp

"$(INTDIR)\AppInstance.obj"	"$(INTDIR)\AppInstance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\arch_setup.cpp

"$(INTDIR)\arch_setup.obj"	"$(INTDIR)\arch_setup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\Crash.cpp

"$(INTDIR)\Crash.obj"	"$(INTDIR)\Crash.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\DebugInfoHunt.cpp

"$(INTDIR)\DebugInfoHunt.obj"	"$(INTDIR)\DebugInfoHunt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\GetFileInformation.cpp

"$(INTDIR)\GetFileInformation.obj"	"$(INTDIR)\GetFileInformation.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\GotoURL.cpp

"$(INTDIR)\GotoURL.obj"	"$(INTDIR)\GotoURL.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\GraphicsWindow.cpp

"$(INTDIR)\GraphicsWindow.obj"	"$(INTDIR)\GraphicsWindow.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\RegistryAccess.cpp

"$(INTDIR)\RegistryAccess.obj"	"$(INTDIR)\RegistryAccess.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\RestartProgram.cpp

"$(INTDIR)\RestartProgram.obj"	"$(INTDIR)\RestartProgram.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\USB.cpp

"$(INTDIR)\USB.obj"	"$(INTDIR)\USB.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\VideoDriverInfo.cpp

"$(INTDIR)\VideoDriverInfo.obj"	"$(INTDIR)\VideoDriverInfo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\WindowIcon.cpp

"$(INTDIR)\WindowIcon.obj"	"$(INTDIR)\WindowIcon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\archutils\Win32\WindowsResources.rc

!IF  "$(CFG)" == "StepMania - Win32 Debug"


"$(INTDIR)\WindowsResources.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\WindowsResources.res" /i "archutils\Win32" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"


"$(INTDIR)\WindowsResources.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\WindowsResources.res" /i "archutils\Win32" /d "NDEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\global.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /Yc"global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\global.obj"	"$(INTDIR)\global.sbr"	"$(INTDIR)\StepMania.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\global.obj"	"$(INTDIR)\global.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StepMania.cpp

"$(INTDIR)\StepMania.obj"	"$(INTDIR)\StepMania.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Actor.cpp

"$(INTDIR)\Actor.obj"	"$(INTDIR)\Actor.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActorFrame.cpp

"$(INTDIR)\ActorFrame.obj"	"$(INTDIR)\ActorFrame.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActorScroller.cpp

"$(INTDIR)\ActorScroller.obj"	"$(INTDIR)\ActorScroller.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActorUtil.cpp

"$(INTDIR)\ActorUtil.obj"	"$(INTDIR)\ActorUtil.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\AutoActor.cpp

"$(INTDIR)\AutoActor.obj"	"$(INTDIR)\AutoActor.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BitmapText.cpp

"$(INTDIR)\BitmapText.obj"	"$(INTDIR)\BitmapText.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Model.cpp

"$(INTDIR)\Model.obj"	"$(INTDIR)\Model.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ModelManager.cpp

"$(INTDIR)\ModelManager.obj"	"$(INTDIR)\ModelManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ModelTypes.cpp

"$(INTDIR)\ModelTypes.obj"	"$(INTDIR)\ModelTypes.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Quad.cpp

"$(INTDIR)\Quad.obj"	"$(INTDIR)\Quad.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RollingNumbers.cpp

"$(INTDIR)\RollingNumbers.obj"	"$(INTDIR)\RollingNumbers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Sprite.cpp

"$(INTDIR)\Sprite.obj"	"$(INTDIR)\Sprite.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Banner.cpp

"$(INTDIR)\Banner.obj"	"$(INTDIR)\Banner.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BGAnimation.cpp

"$(INTDIR)\BGAnimation.obj"	"$(INTDIR)\BGAnimation.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BGAnimationLayer.cpp

"$(INTDIR)\BGAnimationLayer.obj"	"$(INTDIR)\BGAnimationLayer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DifficultyIcon.cpp

"$(INTDIR)\DifficultyIcon.obj"	"$(INTDIR)\DifficultyIcon.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FadingBanner.cpp

"$(INTDIR)\FadingBanner.obj"	"$(INTDIR)\FadingBanner.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MeterDisplay.cpp

"$(INTDIR)\MeterDisplay.obj"	"$(INTDIR)\MeterDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StreamDisplay.cpp

"$(INTDIR)\StreamDisplay.obj"	"$(INTDIR)\StreamDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Transition.cpp

"$(INTDIR)\Transition.obj"	"$(INTDIR)\Transition.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActiveAttackList.cpp

"$(INTDIR)\ActiveAttackList.obj"	"$(INTDIR)\ActiveAttackList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ArrowEffects.cpp

"$(INTDIR)\ArrowEffects.obj"	"$(INTDIR)\ArrowEffects.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\AttackDisplay.cpp

"$(INTDIR)\AttackDisplay.obj"	"$(INTDIR)\AttackDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Background.cpp

"$(INTDIR)\Background.obj"	"$(INTDIR)\Background.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BeginnerHelper.cpp

"$(INTDIR)\BeginnerHelper.obj"	"$(INTDIR)\BeginnerHelper.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CharacterHead.cpp

"$(INTDIR)\CharacterHead.obj"	"$(INTDIR)\CharacterHead.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CombinedLifeMeterTug.cpp

"$(INTDIR)\CombinedLifeMeterTug.obj"	"$(INTDIR)\CombinedLifeMeterTug.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Combo.cpp

"$(INTDIR)\Combo.obj"	"$(INTDIR)\Combo.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DancingCharacters.cpp

"$(INTDIR)\DancingCharacters.obj"	"$(INTDIR)\DancingCharacters.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Foreground.cpp

"$(INTDIR)\Foreground.obj"	"$(INTDIR)\Foreground.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GhostArrow.cpp

"$(INTDIR)\GhostArrow.obj"	"$(INTDIR)\GhostArrow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GhostArrowRow.cpp

"$(INTDIR)\GhostArrowRow.obj"	"$(INTDIR)\GhostArrowRow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HoldGhostArrow.cpp

"$(INTDIR)\HoldGhostArrow.obj"	"$(INTDIR)\HoldGhostArrow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HoldJudgment.cpp

"$(INTDIR)\HoldJudgment.obj"	"$(INTDIR)\HoldJudgment.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Judgment.cpp

"$(INTDIR)\Judgment.obj"	"$(INTDIR)\Judgment.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LifeMeterBar.cpp

"$(INTDIR)\LifeMeterBar.obj"	"$(INTDIR)\LifeMeterBar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LifeMeterBattery.cpp

"$(INTDIR)\LifeMeterBattery.obj"	"$(INTDIR)\LifeMeterBattery.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LifeMeterTime.cpp

"$(INTDIR)\LifeMeterTime.obj"	"$(INTDIR)\LifeMeterTime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LyricDisplay.cpp

"$(INTDIR)\LyricDisplay.obj"	"$(INTDIR)\LyricDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteDisplay.cpp

"$(INTDIR)\NoteDisplay.obj"	"$(INTDIR)\NoteDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteField.cpp

"$(INTDIR)\NoteField.obj"	"$(INTDIR)\NoteField.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PercentageDisplay.cpp

"$(INTDIR)\PercentageDisplay.obj"	"$(INTDIR)\PercentageDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Player.cpp

"$(INTDIR)\Player.obj"	"$(INTDIR)\Player.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ReceptorArrow.cpp

"$(INTDIR)\ReceptorArrow.obj"	"$(INTDIR)\ReceptorArrow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ReceptorArrowRow.cpp

"$(INTDIR)\ReceptorArrowRow.obj"	"$(INTDIR)\ReceptorArrowRow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplay.cpp

"$(INTDIR)\ScoreDisplay.obj"	"$(INTDIR)\ScoreDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayAliveTime.cpp

"$(INTDIR)\ScoreDisplayAliveTime.obj"	"$(INTDIR)\ScoreDisplayAliveTime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayBattle.cpp

"$(INTDIR)\ScoreDisplayBattle.obj"	"$(INTDIR)\ScoreDisplayBattle.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayCalories.cpp

"$(INTDIR)\ScoreDisplayCalories.obj"	"$(INTDIR)\ScoreDisplayCalories.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayLifeTime.cpp

"$(INTDIR)\ScoreDisplayLifeTime.obj"	"$(INTDIR)\ScoreDisplayLifeTime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayNormal.cpp

"$(INTDIR)\ScoreDisplayNormal.obj"	"$(INTDIR)\ScoreDisplayNormal.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayOni.cpp

"$(INTDIR)\ScoreDisplayOni.obj"	"$(INTDIR)\ScoreDisplayOni.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayPercentage.cpp

"$(INTDIR)\ScoreDisplayPercentage.obj"	"$(INTDIR)\ScoreDisplayPercentage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScoreDisplayRave.cpp

"$(INTDIR)\ScoreDisplayRave.obj"	"$(INTDIR)\ScoreDisplayRave.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\BPMDisplay.cpp

"$(INTDIR)\BPMDisplay.obj"	"$(INTDIR)\BPMDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ComboGraph.cpp

"$(INTDIR)\ComboGraph.obj"	"$(INTDIR)\ComboGraph.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CourseContentsList.cpp

"$(INTDIR)\CourseContentsList.obj"	"$(INTDIR)\CourseContentsList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CourseEntryDisplay.cpp

"$(INTDIR)\CourseEntryDisplay.obj"	"$(INTDIR)\CourseEntryDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DifficultyDisplay.cpp

"$(INTDIR)\DifficultyDisplay.obj"	"$(INTDIR)\DifficultyDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DifficultyList.cpp

"$(INTDIR)\DifficultyList.obj"	"$(INTDIR)\DifficultyList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DifficultyMeter.cpp

"$(INTDIR)\DifficultyMeter.obj"	"$(INTDIR)\DifficultyMeter.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DifficultyRating.cpp

"$(INTDIR)\DifficultyRating.obj"	"$(INTDIR)\DifficultyRating.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\DualScrollBar.cpp

"$(INTDIR)\DualScrollBar.obj"	"$(INTDIR)\DualScrollBar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\EditCoursesMenu.cpp

"$(INTDIR)\EditCoursesMenu.obj"	"$(INTDIR)\EditCoursesMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\EditCoursesSongMenu.cpp

"$(INTDIR)\EditCoursesSongMenu.obj"	"$(INTDIR)\EditCoursesSongMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\EditMenu.cpp

"$(INTDIR)\EditMenu.obj"	"$(INTDIR)\EditMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GradeDisplay.cpp

"$(INTDIR)\GradeDisplay.obj"	"$(INTDIR)\GradeDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GraphDisplay.cpp

"$(INTDIR)\GraphDisplay.obj"	"$(INTDIR)\GraphDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GrooveGraph.cpp

"$(INTDIR)\GrooveGraph.obj"	"$(INTDIR)\GrooveGraph.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GrooveRadar.cpp

"$(INTDIR)\GrooveRadar.obj"	"$(INTDIR)\GrooveRadar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GroupList.cpp

"$(INTDIR)\GroupList.obj"	"$(INTDIR)\GroupList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\HelpDisplay.cpp

"$(INTDIR)\HelpDisplay.obj"	"$(INTDIR)\HelpDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MemoryCardDisplay.cpp

"$(INTDIR)\MemoryCardDisplay.obj"	"$(INTDIR)\MemoryCardDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MenuTimer.cpp

"$(INTDIR)\MenuTimer.obj"	"$(INTDIR)\MenuTimer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ModeSwitcher.cpp

"$(INTDIR)\ModeSwitcher.obj"	"$(INTDIR)\ModeSwitcher.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MusicBannerWheel.cpp

"$(INTDIR)\MusicBannerWheel.obj"	"$(INTDIR)\MusicBannerWheel.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MusicList.cpp

"$(INTDIR)\MusicList.obj"	"$(INTDIR)\MusicList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MusicSortDisplay.cpp

"$(INTDIR)\MusicSortDisplay.obj"	"$(INTDIR)\MusicSortDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MusicWheel.cpp

"$(INTDIR)\MusicWheel.obj"	"$(INTDIR)\MusicWheel.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MusicWheelItem.cpp

"$(INTDIR)\MusicWheelItem.obj"	"$(INTDIR)\MusicWheelItem.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OptionIcon.cpp

"$(INTDIR)\OptionIcon.obj"	"$(INTDIR)\OptionIcon.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OptionIconRow.cpp

"$(INTDIR)\OptionIconRow.obj"	"$(INTDIR)\OptionIconRow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OptionRow.cpp

"$(INTDIR)\OptionRow.obj"	"$(INTDIR)\OptionRow.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\OptionsCursor.cpp

"$(INTDIR)\OptionsCursor.obj"	"$(INTDIR)\OptionsCursor.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PaneDisplay.cpp

"$(INTDIR)\PaneDisplay.obj"	"$(INTDIR)\PaneDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\RoomWheel.cpp

"$(INTDIR)\RoomWheel.obj"	"$(INTDIR)\RoomWheel.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScrollBar.cpp

"$(INTDIR)\ScrollBar.obj"	"$(INTDIR)\ScrollBar.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScrollingList.cpp

"$(INTDIR)\ScrollingList.obj"	"$(INTDIR)\ScrollingList.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SnapDisplay.cpp

"$(INTDIR)\SnapDisplay.obj"	"$(INTDIR)\SnapDisplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\TextBanner.cpp

"$(INTDIR)\TextBanner.obj"	"$(INTDIR)\TextBanner.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WheelBase.cpp

"$(INTDIR)\WheelBase.obj"	"$(INTDIR)\WheelBase.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WheelItemBase.cpp

"$(INTDIR)\WheelItemBase.obj"	"$(INTDIR)\WheelItemBase.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\WheelNotifyIcon.cpp

"$(INTDIR)\WheelNotifyIcon.obj"	"$(INTDIR)\WheelNotifyIcon.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Screen.cpp

"$(INTDIR)\Screen.obj"	"$(INTDIR)\Screen.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenAttract.cpp

"$(INTDIR)\ScreenAttract.obj"	"$(INTDIR)\ScreenAttract.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenBookkeeping.cpp

"$(INTDIR)\ScreenBookkeeping.obj"	"$(INTDIR)\ScreenBookkeeping.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenBranch.cpp

"$(INTDIR)\ScreenBranch.obj"	"$(INTDIR)\ScreenBranch.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenCenterImage.cpp

"$(INTDIR)\ScreenCenterImage.obj"	"$(INTDIR)\ScreenCenterImage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenCredits.cpp

"$(INTDIR)\ScreenCredits.obj"	"$(INTDIR)\ScreenCredits.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenDebugOverlay.cpp

"$(INTDIR)\ScreenDebugOverlay.obj"	"$(INTDIR)\ScreenDebugOverlay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenDemonstration.cpp

"$(INTDIR)\ScreenDemonstration.obj"	"$(INTDIR)\ScreenDemonstration.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEdit.cpp

"$(INTDIR)\ScreenEdit.obj"	"$(INTDIR)\ScreenEdit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEditCoursesMenu.cpp

"$(INTDIR)\ScreenEditCoursesMenu.obj"	"$(INTDIR)\ScreenEditCoursesMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEditMenu.cpp

!IF  "$(CFG)" == "StepMania - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX"global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ScreenEditMenu.obj"	"$(INTDIR)\ScreenEditMenu.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "StepMania - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /Ob2 /I "." /I "vorbis" /I "libjpeg" /I "lua-5.0\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\StepMania.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ScreenEditMenu.obj"	"$(INTDIR)\ScreenEditMenu.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ScreenEnding.cpp

"$(INTDIR)\ScreenEnding.obj"	"$(INTDIR)\ScreenEnding.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEndlessBreak.cpp

"$(INTDIR)\ScreenEndlessBreak.obj"	"$(INTDIR)\ScreenEndlessBreak.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEvaluation.cpp

"$(INTDIR)\ScreenEvaluation.obj"	"$(INTDIR)\ScreenEvaluation.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenExit.cpp

"$(INTDIR)\ScreenExit.obj"	"$(INTDIR)\ScreenExit.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEz2SelectMusic.cpp

"$(INTDIR)\ScreenEz2SelectMusic.obj"	"$(INTDIR)\ScreenEz2SelectMusic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenEz2SelectPlayer.cpp

"$(INTDIR)\ScreenEz2SelectPlayer.obj"	"$(INTDIR)\ScreenEz2SelectPlayer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenGameplay.cpp

"$(INTDIR)\ScreenGameplay.obj"	"$(INTDIR)\ScreenGameplay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenGameplayMultiplayer.cpp

"$(INTDIR)\ScreenGameplayMultiplayer.obj"	"$(INTDIR)\ScreenGameplayMultiplayer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenHowToPlay.cpp

"$(INTDIR)\ScreenHowToPlay.obj"	"$(INTDIR)\ScreenHowToPlay.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenInstructions.cpp

"$(INTDIR)\ScreenInstructions.obj"	"$(INTDIR)\ScreenInstructions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenJukebox.cpp

"$(INTDIR)\ScreenJukebox.obj"	"$(INTDIR)\ScreenJukebox.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenLogo.cpp

"$(INTDIR)\ScreenLogo.obj"	"$(INTDIR)\ScreenLogo.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenMapControllers.cpp

"$(INTDIR)\ScreenMapControllers.obj"	"$(INTDIR)\ScreenMapControllers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenMessage.cpp

"$(INTDIR)\ScreenMessage.obj"	"$(INTDIR)\ScreenMessage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenMiniMenu.cpp

"$(INTDIR)\ScreenMiniMenu.obj"	"$(INTDIR)\ScreenMiniMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenMusicScroll.cpp

"$(INTDIR)\ScreenMusicScroll.obj"	"$(INTDIR)\ScreenMusicScroll.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNameEntry.cpp

"$(INTDIR)\ScreenNameEntry.obj"	"$(INTDIR)\ScreenNameEntry.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNameEntryTraditional.cpp

"$(INTDIR)\ScreenNameEntryTraditional.obj"	"$(INTDIR)\ScreenNameEntryTraditional.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNetEvaluation.cpp

"$(INTDIR)\ScreenNetEvaluation.obj"	"$(INTDIR)\ScreenNetEvaluation.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNetRoom.cpp

"$(INTDIR)\ScreenNetRoom.obj"	"$(INTDIR)\ScreenNetRoom.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNetSelectBase.cpp

"$(INTDIR)\ScreenNetSelectBase.obj"	"$(INTDIR)\ScreenNetSelectBase.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNetSelectMusic.cpp

"$(INTDIR)\ScreenNetSelectMusic.obj"	"$(INTDIR)\ScreenNetSelectMusic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenNetworkOptions.cpp

"$(INTDIR)\ScreenNetworkOptions.obj"	"$(INTDIR)\ScreenNetworkOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenOptions.cpp

"$(INTDIR)\ScreenOptions.obj"	"$(INTDIR)\ScreenOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenOptionsMaster.cpp

"$(INTDIR)\ScreenOptionsMaster.obj"	"$(INTDIR)\ScreenOptionsMaster.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenOptionsMasterPrefs.cpp

"$(INTDIR)\ScreenOptionsMasterPrefs.obj"	"$(INTDIR)\ScreenOptionsMasterPrefs.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenPackages.cpp

"$(INTDIR)\ScreenPackages.obj"	"$(INTDIR)\ScreenPackages.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenPlayerOptions.cpp

"$(INTDIR)\ScreenPlayerOptions.obj"	"$(INTDIR)\ScreenPlayerOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenProfileOptions.cpp

"$(INTDIR)\ScreenProfileOptions.obj"	"$(INTDIR)\ScreenProfileOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenPrompt.cpp

"$(INTDIR)\ScreenPrompt.obj"	"$(INTDIR)\ScreenPrompt.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenRanking.cpp

"$(INTDIR)\ScreenRanking.obj"	"$(INTDIR)\ScreenRanking.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenReloadSongs.cpp

"$(INTDIR)\ScreenReloadSongs.obj"	"$(INTDIR)\ScreenReloadSongs.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSandbox.cpp

"$(INTDIR)\ScreenSandbox.obj"	"$(INTDIR)\ScreenSandbox.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelect.cpp

"$(INTDIR)\ScreenSelect.obj"	"$(INTDIR)\ScreenSelect.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectCharacter.cpp

"$(INTDIR)\ScreenSelectCharacter.obj"	"$(INTDIR)\ScreenSelectCharacter.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectDifficulty.cpp

"$(INTDIR)\ScreenSelectDifficulty.obj"	"$(INTDIR)\ScreenSelectDifficulty.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectGroup.cpp

"$(INTDIR)\ScreenSelectGroup.obj"	"$(INTDIR)\ScreenSelectGroup.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectMaster.cpp

"$(INTDIR)\ScreenSelectMaster.obj"	"$(INTDIR)\ScreenSelectMaster.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectMode.cpp

"$(INTDIR)\ScreenSelectMode.obj"	"$(INTDIR)\ScreenSelectMode.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectMusic.cpp

"$(INTDIR)\ScreenSelectMusic.obj"	"$(INTDIR)\ScreenSelectMusic.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSelectStyle.cpp

"$(INTDIR)\ScreenSelectStyle.obj"	"$(INTDIR)\ScreenSelectStyle.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSetTime.cpp

"$(INTDIR)\ScreenSetTime.obj"	"$(INTDIR)\ScreenSetTime.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSMOnlineLogin.cpp

"$(INTDIR)\ScreenSMOnlineLogin.obj"	"$(INTDIR)\ScreenSMOnlineLogin.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSongOptions.cpp

"$(INTDIR)\ScreenSongOptions.obj"	"$(INTDIR)\ScreenSongOptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSplash.cpp

"$(INTDIR)\ScreenSplash.obj"	"$(INTDIR)\ScreenSplash.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenStage.cpp

"$(INTDIR)\ScreenStage.obj"	"$(INTDIR)\ScreenStage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenStyleSplash.cpp

"$(INTDIR)\ScreenStyleSplash.obj"	"$(INTDIR)\ScreenStyleSplash.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenSystemLayer.cpp

"$(INTDIR)\ScreenSystemLayer.obj"	"$(INTDIR)\ScreenSystemLayer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTest.cpp

"$(INTDIR)\ScreenTest.obj"	"$(INTDIR)\ScreenTest.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTestFonts.cpp

"$(INTDIR)\ScreenTestFonts.obj"	"$(INTDIR)\ScreenTestFonts.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTestInput.cpp

"$(INTDIR)\ScreenTestInput.obj"	"$(INTDIR)\ScreenTestInput.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTestLights.cpp

"$(INTDIR)\ScreenTestLights.obj"	"$(INTDIR)\ScreenTestLights.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTestSound.cpp

"$(INTDIR)\ScreenTestSound.obj"	"$(INTDIR)\ScreenTestSound.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTextEntry.cpp

"$(INTDIR)\ScreenTextEntry.obj"	"$(INTDIR)\ScreenTextEntry.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenTitleMenu.cpp

"$(INTDIR)\ScreenTitleMenu.obj"	"$(INTDIR)\ScreenTitleMenu.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenUnlock.cpp

"$(INTDIR)\ScreenUnlock.obj"	"$(INTDIR)\ScreenUnlock.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenWithMenuElements.cpp

"$(INTDIR)\ScreenWithMenuElements.obj"	"$(INTDIR)\ScreenWithMenuElements.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\AnnouncerManager.cpp

"$(INTDIR)\AnnouncerManager.obj"	"$(INTDIR)\AnnouncerManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Bookkeeper.cpp

"$(INTDIR)\Bookkeeper.obj"	"$(INTDIR)\Bookkeeper.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\CryptManager.cpp

"$(INTDIR)\CryptManager.obj"	"$(INTDIR)\CryptManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ezsockets.cpp

"$(INTDIR)\ezsockets.obj"	"$(INTDIR)\ezsockets.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FontManager.cpp

"$(INTDIR)\FontManager.obj"	"$(INTDIR)\FontManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameManager.cpp

"$(INTDIR)\GameManager.obj"	"$(INTDIR)\GameManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameSoundManager.cpp

"$(INTDIR)\GameSoundManager.obj"	"$(INTDIR)\GameSoundManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\GameState.cpp

"$(INTDIR)\GameState.obj"	"$(INTDIR)\GameState.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\InputFilter.cpp

"$(INTDIR)\InputFilter.obj"	"$(INTDIR)\InputFilter.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\InputMapper.cpp

"$(INTDIR)\InputMapper.obj"	"$(INTDIR)\InputMapper.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\InputQueue.cpp

"$(INTDIR)\InputQueue.obj"	"$(INTDIR)\InputQueue.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LightsManager.cpp

"$(INTDIR)\LightsManager.obj"	"$(INTDIR)\LightsManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\LuaManager.cpp

"$(INTDIR)\LuaManager.obj"	"$(INTDIR)\LuaManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MemoryCardManager.cpp

"$(INTDIR)\MemoryCardManager.obj"	"$(INTDIR)\MemoryCardManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\MessageManager.cpp

"$(INTDIR)\MessageManager.obj"	"$(INTDIR)\MessageManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NetworkSyncManager.cpp

"$(INTDIR)\NetworkSyncManager.obj"	"$(INTDIR)\NetworkSyncManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NetworkSyncServer.cpp

"$(INTDIR)\NetworkSyncServer.obj"	"$(INTDIR)\NetworkSyncServer.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\NoteSkinManager.cpp

"$(INTDIR)\NoteSkinManager.obj"	"$(INTDIR)\NoteSkinManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PrefsManager.cpp

"$(INTDIR)\PrefsManager.obj"	"$(INTDIR)\PrefsManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ProfileManager.cpp

"$(INTDIR)\ProfileManager.obj"	"$(INTDIR)\ProfileManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ScreenManager.cpp

"$(INTDIR)\ScreenManager.obj"	"$(INTDIR)\ScreenManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\SongManager.cpp

"$(INTDIR)\SongManager.obj"	"$(INTDIR)\SongManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StatsManager.cpp

"$(INTDIR)\StatsManager.obj"	"$(INTDIR)\StatsManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ThemeManager.cpp

"$(INTDIR)\ThemeManager.obj"	"$(INTDIR)\ThemeManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\UnlockManager.cpp

"$(INTDIR)\UnlockManager.obj"	"$(INTDIR)\UnlockManager.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\crypto51\algebra.cpp

"$(INTDIR)\algebra.obj"	"$(INTDIR)\algebra.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\algparam.cpp

"$(INTDIR)\algparam.obj"	"$(INTDIR)\algparam.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\asn.cpp

"$(INTDIR)\asn.obj"	"$(INTDIR)\asn.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\cryptlib.cpp

"$(INTDIR)\cryptlib.obj"	"$(INTDIR)\cryptlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\files.cpp

"$(INTDIR)\files.obj"	"$(INTDIR)\files.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\filters.cpp

"$(INTDIR)\filters.obj"	"$(INTDIR)\filters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\integer.cpp

"$(INTDIR)\integer.obj"	"$(INTDIR)\integer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\iterhash.cpp

"$(INTDIR)\iterhash.obj"	"$(INTDIR)\iterhash.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\misc.cpp

"$(INTDIR)\misc.obj"	"$(INTDIR)\misc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\mqueue.cpp

"$(INTDIR)\mqueue.obj"	"$(INTDIR)\mqueue.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\nbtheory.cpp

"$(INTDIR)\nbtheory.obj"	"$(INTDIR)\nbtheory.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\osrng.cpp

"$(INTDIR)\osrng.obj"	"$(INTDIR)\osrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\pkcspad.cpp

"$(INTDIR)\pkcspad.obj"	"$(INTDIR)\pkcspad.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\pubkey.cpp

"$(INTDIR)\pubkey.obj"	"$(INTDIR)\pubkey.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\queue.cpp

"$(INTDIR)\queue.obj"	"$(INTDIR)\queue.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\rsa.cpp

"$(INTDIR)\rsa.obj"	"$(INTDIR)\rsa.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto51\sha.cpp

"$(INTDIR)\sha.obj"	"$(INTDIR)\sha.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptBn.cpp

"$(INTDIR)\CryptBn.obj"	"$(INTDIR)\CryptBn.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\CryptHelpers.cpp

"$(INTDIR)\CryptHelpers.obj"	"$(INTDIR)\CryptHelpers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\crypto\CryptMD5.cpp

"$(INTDIR)\CryptMD5.obj"	"$(INTDIR)\CryptMD5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptNoise.cpp

"$(INTDIR)\CryptNoise.obj"	"$(INTDIR)\CryptNoise.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptPrime.cpp

"$(INTDIR)\CryptPrime.obj"	"$(INTDIR)\CryptPrime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptRand.cpp

"$(INTDIR)\CryptRand.obj"	"$(INTDIR)\CryptRand.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptRSA.cpp

"$(INTDIR)\CryptRSA.obj"	"$(INTDIR)\CryptRSA.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptSH512.cpp

"$(INTDIR)\CryptSH512.obj"	"$(INTDIR)\CryptSH512.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\crypto\CryptSHA.cpp

"$(INTDIR)\CryptSHA.obj"	"$(INTDIR)\CryptSHA.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

