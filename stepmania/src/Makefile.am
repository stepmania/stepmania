bin_PROGRAMS = stepmania
TESTS = 
tests: $(TESTS)
if BUILD_TESTS
noinst_PROGRAMS = $(TESTS)
endif

AM_LDFLAGS =
AM_CXXFLAGS = 
AM_CFLAGS = 

# Relax inlining; the default of 600 takes way too long to compile and
# generates enormous output.
AM_CXXFLAGS += -finline-limit=300

AM_CXXFLAGS += $(X_CFLAGS)

$(srcdir)/libresample/libresample.a:
	cd $(srcdir)/libresample && autoconf && sh ./configure && make

Screens = \
Screen.cpp Screen.h ScreenAttract.cpp ScreenAttract.h ScreenAward.cpp ScreenAward.h \
ScreenBookkeeping.cpp ScreenBookkeeping.h ScreenBranch.cpp ScreenBranch.h ScreenCaution.cpp ScreenCaution.h \
ScreenCenterImage.cpp ScreenCenterImage.h ScreenCredits.cpp ScreenCredits.h \
ScreenDemonstration.cpp ScreenDemonstration.h ScreenDimensions.h \
ScreenDownloadMachineStats.cpp ScreenDownloadMachineStats.h ScreenEdit.cpp ScreenEdit.h \
ScreenEditCoursesMenu.cpp ScreenEditCoursesMenu.h ScreenEditMenu.cpp ScreenEditMenu.h ScreenEnding.cpp ScreenEnding.h \
ScreenEndlessBreak.cpp ScreenEndlessBreak.h ScreenEvaluation.cpp ScreenEvaluation.h ScreenExit.cpp ScreenExit.h \
ScreenEz2SelectMusic.cpp ScreenEz2SelectMusic.h ScreenEz2SelectPlayer.cpp ScreenEz2SelectPlayer.h \
ScreenGameOver.cpp ScreenGameOver.h ScreenGameplay.cpp ScreenGameplay.h ScreenHowToPlay.cpp ScreenHowToPlay.h \
ScreenInstructions.cpp ScreenInstructions.h ScreenJukebox.cpp ScreenJukebox.h \
ScreenJukeboxMenu.cpp ScreenJukeboxMenu.h ScreenLogo.cpp ScreenLogo.h ScreenMapControllers.cpp ScreenMapControllers.h \
ScreenMessage.h ScreenMiniMenu.cpp ScreenMiniMenu.h ScreenMusicScroll.cpp ScreenMusicScroll.h \
ScreenNameEntry.cpp ScreenNameEntry.h ScreenNameEntryTraditional.cpp ScreenNameEntryTraditional.h \
ScreenOptions.cpp ScreenOptions.h ScreenOptionsMaster.cpp ScreenOptionsMaster.h \
ScreenOptionsMasterPrefs.cpp ScreenOptionsMasterPrefs.h ScreenPlayerOptions.cpp ScreenPlayerOptions.h \
ScreenProfileOptions.cpp ScreenProfileOptions.h ScreenPrompt.cpp ScreenPrompt.h ScreenRanking.cpp ScreenRanking.h \
ScreenReloadSongs.cpp ScreenReloadSongs.h ScreenSandbox.cpp ScreenSandbox.h \
ScreenSelect.cpp ScreenSelect.h ScreenSelectCharacter.cpp ScreenSelectCharacter.h \
ScreenSelectCourse.cpp ScreenSelectCourse.h ScreenSelectDifficulty.cpp ScreenSelectDifficulty.h \
ScreenSelectDifficultyEX.cpp ScreenSelectDifficultyEX.h ScreenSelectGroup.cpp ScreenSelectGroup.h \
ScreenSelectMaster.cpp ScreenSelectMaster.h ScreenSelectMode.cpp ScreenSelectMode.h \
ScreenSelectMusic.cpp ScreenSelectMusic.h ScreenSelectStyle.cpp ScreenSelectStyle.h ScreenSetTime.cpp ScreenSetTime.h \
ScreenSongOptions.cpp ScreenSongOptions.h ScreenStage.cpp ScreenStage.h ScreenStyleSplash.cpp ScreenStyleSplash.h \
ScreenTest.cpp ScreenTest.h ScreenTestFonts.cpp ScreenTestFonts.h ScreenTestInput.cpp ScreenTestInput.h \
ScreenTestLights.cpp ScreenTestLights.h ScreenTestSound.cpp ScreenTestSound.h ScreenTextEntry.cpp ScreenTextEntry.h \
ScreenTitleMenu.cpp ScreenTitleMenu.h ScreenUnlock.cpp ScreenUnlock.h

DataStructures = \
Attack.cpp Attack.h BannerCache.cpp BannerCache.h Character.cpp Character.h CharacterHead.cpp CharacterHead.h \
CodeDetector.cpp CodeDetector.h EnumHelper.cpp EnumHelper.h Course.cpp Course.h Font.cpp Font.h FontCharAliases.cpp FontCharAliases.h \
FontCharmaps.cpp FontCharmaps.h Game.h GameConstantsAndTypes.cpp GameConstantsAndTypes.h GameDef.cpp GameDef.h \
GameInput.cpp GameInput.h Grade.cpp Grade.h HighScore.cpp HighScore.h Inventory.cpp Inventory.h LuaFunctions.h \
LuaHelpers.cpp LuaHelpers.h LyricsLoader.cpp LyricsLoader.h MenuInput.h ModeChoice.cpp ModeChoice.h \
NoteData.cpp NoteData.h NoteDataUtil.cpp NoteDataUtil.h NoteDataWithScoring.cpp NoteDataWithScoring.h \
NoteFieldPositioning.cpp NoteFieldPositioning.h NoteTypes.cpp NoteTypes.h NotesLoader.cpp NotesLoader.h \
NotesLoaderBMS.cpp NotesLoaderBMS.h NotesLoaderDWI.cpp NotesLoaderDWI.h NotesLoaderKSF.cpp NotesLoaderKSF.h \
NotesLoaderSM.cpp NotesLoaderSM.h NotesWriterDWI.cpp NotesWriterDWI.h NotesWriterSM.cpp NotesWriterSM.h \
PlayerAI.cpp PlayerAI.h PlayerNumber.cpp PlayerNumber.h PlayerOptions.cpp PlayerOptions.h Profile.cpp Profile.h \
ProfileHtml.cpp ProfileHtml.h RandomSample.cpp RandomSample.h ScoreKeeper.h ScoreKeeperMAX2.cpp ScoreKeeperMAX2.h \
ScoreKeeperRave.cpp ScoreKeeperRave.h Song.cpp song.h SongCacheIndex.cpp SongCacheIndex.h \
SongOptions.cpp SongOptions.h StageStats.cpp StageStats.h Steps.cpp Steps.h Style.h StyleDef.cpp StyleDef.h \
StyleInput.h TimeConstants.cpp TimeConstants.h TimingData.cpp TimingData.h TitleSubstitution.cpp TitleSubstitution.h

FileTypes = IniFile.cpp IniFile.h \
		MsdFile.cpp MsdFile.h \
		XmlFile.cpp XmlFile.h

StepMania = StdString.h \
		StepMania.cpp StepMania.h \
		global.cpp global.h

LoadingWindow = arch/LoadingWindow/LoadingWindow_Null.h \
		arch/LoadingWindow/LoadingWindow_SDL.cpp arch/LoadingWindow/LoadingWindow_SDL.h

if HAVE_GTK
LoadingWindow += arch/LoadingWindow/LoadingWindow_Gtk.cpp arch/LoadingWindow/LoadingWindow_Gtk.h
endif

Sound = arch/Sound/RageSoundDriver_Null.cpp arch/Sound/RageSoundDriver_Null.h \
		arch/Sound/RageSoundDriver_Generic_Software.cpp arch/Sound/RageSoundDriver_Generic_Software.h


if HAVE_OSS
Sound += arch/Sound/RageSoundDriver_OSS.cpp arch/Sound/RageSoundDriver_OSS.h
endif

if HAVE_ALSA
Sound += arch/Sound/ALSA9Dynamic.cpp arch/Sound/ALSA9Dynamic.h arch/Sound/ALSA9Functions.h \
	 arch/Sound/ALSA9Helpers.cpp arch/Sound/ALSA9Helpers.h \
	 arch/Sound/RageSoundDriver_ALSA9.cpp arch/Sound/RageSoundDriver_ALSA9.h \
	 arch/Sound/RageSoundDriver_ALSA9_Software.cpp arch/Sound/RageSoundDriver_ALSA9_Software.h

# Add this to AM_CXXFLAGS, not stepmania_CXXFLAGS: that'll cause multiple copies of
# several sources to be built. We link to this at runtime, so don't add ALSA_LDFLAGS.
AM_CXXFLAGS += $(ALSA_CFLAGS)
endif

ArchHooks = arch/ArchHooks/ArchHooks.cpp arch/ArchHooks/ArchHooks.h \
		arch/ArchHooks/ArchHooks_Unix.cpp arch/ArchHooks/ArchHooks_Unix.h

InputHandler = arch/InputHandler/InputHandler.cpp arch/InputHandler/InputHandler.h \
		arch/InputHandler/InputHandler_SDL.cpp arch/InputHandler/InputHandler_SDL.h

MovieTexture = arch/MovieTexture/MovieTexture.cpp arch/MovieTexture/MovieTexture.h \
		arch/MovieTexture/MovieTexture_Null.cpp arch/MovieTexture/MovieTexture_Null.h

if HAVE_FFMPEG
MovieTexture += arch/MovieTexture/MovieTexture_FFMpeg.cpp arch/MovieTexture/MovieTexture_FFMpeg.h
endif

Lights = arch/Lights/LightsDriver.h \
		arch/Lights/LightsDriver_SystemMessage.cpp arch/Lights/LightsDriver_SystemMessage.h

MemoryCard = arch/MemoryCard/MemoryCardDriver.h \
	     arch/MemoryCard/MemoryCardDriverThreaded.cpp arch/MemoryCard/MemoryCardDriverThreaded.h \
	     arch/MemoryCard/MemoryCardDriverThreaded_Linux.cpp arch/MemoryCard/MemoryCardDriverThreaded_Linux.h 

LowLevelWindow = arch/LowLevelWindow/LowLevelWindow.h \
		arch/LowLevelWindow/LowLevelWindow_SDL.cpp arch/LowLevelWindow/LowLevelWindow_SDL.h

ArchUtils = archutils/Unix/AssertionHandler.cpp \
		archutils/Unix/GetSysInfo.cpp archutils/Unix/GetSysInfo.h \
		archutils/Unix/SignalHandler.cpp archutils/Unix/SignalHandler.h

if USE_CRASH_HANDLER
ArchUtils += archutils/Unix/Backtrace.cpp archutils/Unix/Backtrace.h \
		archutils/Unix/BacktraceNames.cpp archutils/Unix/BacktraceNames.h \
		archutils/Unix/CrashHandler.cpp archutils/Unix/CrashHandler.h \
		archutils/Unix/CrashHandlerChild.cpp archutils/Unix/CrashHandlerInternal.h \
		archutils/Unix/LinuxThreadHelpers.cpp archutils/Unix/LinuxThreadHelpers.h
endif

Arch = $(LoadingWindow) $(Sound) $(ArchHooks) $(InputHandler) $(MovieTexture) \
	   $(Lights) $(MemoryCard) $(LowLevelWindow) $(ArchUtils) \
	   arch/arch.cpp arch/arch.h arch/arch_default.h arch/arch_linux.h arch/arch_platform.h

RageSoundFileReaders = RageSoundReader_WAV.cpp RageSoundReader_WAV.h 

if HAVE_VORBIS
RageSoundFileReaders += RageSoundReader_Vorbisfile.cpp RageSoundReader_Vorbisfile.h
endif

if HAVE_MP3
RageSoundFileReaders += RageSoundReader_MP3.cpp RageSoundReader_MP3.h
endif

ActorsInGameplayAndMenus = \
BGAnimation.cpp BGAnimation.h BGAnimationLayer.cpp BGAnimationLayer.h Banner.cpp Banner.h \
ConditionalBGA.cpp ConditionalBGA.h DifficultyIcon.cpp DifficultyIcon.h MeterDisplay.cpp MeterDisplay.h \
Transition.cpp Transition.h

ActorsInMenus = \
ActiveAttackList.cpp ActiveAttackList.h BPMDisplay.cpp BPMDisplay.h ComboGraph.cpp ComboGraph.h \
CourseContentsList.cpp CourseContentsList.h CourseEntryDisplay.cpp CourseEntryDisplay.h \
DifficultyDisplay.cpp DifficultyDisplay.h DifficultyList.cpp DifficultyList.h \
DifficultyMeter.cpp DifficultyMeter.h DifficultyRating.cpp DifficultyRating.h DualScrollBar.cpp DualScrollBar.h \
EditCoursesMenu.cpp EditCoursesMenu.h EditMenu.cpp EditMenu.h FadingBanner.cpp FadingBanner.h \
GradeDisplay.cpp GradeDisplay.h GraphDisplay.cpp GraphDisplay.h GrooveGraph.cpp GrooveGraph.h \
GrooveRadar.cpp GrooveRadar.h GroupList.cpp GroupList.h HelpDisplay.cpp HelpDisplay.h JukeboxMenu.cpp JukeboxMenu.h \
ListDisplay.cpp ListDisplay.h MemoryCardDisplay.cpp MemoryCardDisplay.h MenuElements.cpp MenuElements.h \
MenuTimer.cpp MenuTimer.h ModeSwitcher.cpp ModeSwitcher.h MusicBannerWheel.cpp MusicBannerWheel.h \
MusicList.cpp MusicList.h MusicSortDisplay.cpp MusicSortDisplay.h MusicWheel.cpp MusicWheel.h \
MusicWheelItem.cpp MusicWheelItem.h OptionIcon.cpp OptionIcon.h OptionIconRow.cpp OptionIconRow.h \
OptionsCursor.cpp OptionsCursor.h PaneDisplay.cpp PaneDisplay.h ScrollBar.cpp ScrollBar.h \
ScrollingList.cpp ScrollingList.h SongCreditDisplay.cpp SongCreditDisplay.h SnapDisplay.cpp SnapDisplay.h \
TextBanner.cpp TextBanner.h WheelNotifyIcon.cpp WheelNotifyIcon.h

ActorsInGameplay = \
ArrowBackdrop.cpp ArrowBackdrop.h ArrowEffects.cpp ArrowEffects.h AttackDisplay.cpp AttackDisplay.h \
Background.cpp Background.h BeginnerHelper.cpp BeginnerHelper.h CombinedLifeMeterTug.cpp CombinedLifeMeterTug.h \
Combo.cpp Combo.h DancingCharacters.cpp DancingCharacters.h Foreground.cpp Foreground.h GhostArrow.cpp GhostArrow.h \
GhostArrowRow.cpp GhostArrowRow.h HoldGhostArrow.cpp HoldGhostArrow.h HoldJudgment.cpp HoldJudgment.h \
Judgment.cpp Judgment.h LifeMeter.h LifeMeterBar.cpp LifeMeterBar.h LifeMeterBattery.cpp LifeMeterBattery.h \
LyricDisplay.cpp LyricDisplay.h NoteDisplay.cpp NoteDisplay.h NoteField.cpp NoteField.h \
NoteFieldPlus.cpp NoteFieldPlus.h PercentageDisplay.cpp PercentageDisplay.h Player.cpp Player.h \
ProTimingDisplay.cpp ProTimingDisplay.h ReceptorArrow.cpp ReceptorArrow.h ReceptorArrowRow.cpp ReceptorArrowRow.h \
ScoreDisplay.cpp ScoreDisplay.h ScoreDisplayBattle.cpp ScoreDisplayBattle.h \
ScoreDisplayNormal.cpp ScoreDisplayNormal.h ScoreDisplayOni.cpp ScoreDisplayOni.h \
ScoreDisplayPercentage.cpp ScoreDisplayPercentage.h ScoreDisplayRave.cpp ScoreDisplayRave.h

Helpers = SDL_SaveJPEG.cpp SDL_SaveJPEG.h \
	  SDL_dither.cpp SDL_dither.h \
	  SDL_rotozoom.cpp SDL_rotozoom.h \
	  SDL_utils.cpp SDL_utils.h

PCRE = pcre/chartables.c pcre/get.c pcre/internal.h pcre/maketables.c pcre/pcre.c pcre/pcre.h pcre/study.c

RageFile = \
RageFile.cpp RageFile.h RageFileDriver.cpp RageFileDriver.h RageFileManager.cpp RageFileManager.h \
RageFileDriverDirect.cpp RageFileDriverDirect.h RageFileDriverDirectHelpers.cpp RageFileDriverDirectHelpers.h \
RageFileDriverZip.cpp RageFileDriverZip.h

Rage = $(Helpers) $(PCRE) $(RageFile) $(RageSoundFileReaders) \
RageBitmapTexture.cpp RageBitmapTexture.h RageDisplay.cpp RageDisplay.h RageDisplay_OGL.cpp RageDisplay_OGL.h \
RageDisplay_Null.cpp RageDisplay_Null.h RageException.cpp RageException.h RageInput.cpp RageInput.h \
RageInputDevice.cpp RageInputDevice.h RageLog.cpp RageLog.h RageMath.cpp RageMath.h \
RageModelGeometry.cpp RageModelGeometry.h RageSound.cpp RageSound.h RageSoundManager.cpp RageSoundManager.h \
RageSoundPosMap.cpp RageSoundPosMap.h RageSoundReader.h RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
RageSoundReader_Preload.cpp RageSoundReader_Preload.h RageSoundReader_Resample.cpp RageSoundReader_Resample.h \
RageSoundReader_Resample_Fast.cpp RageSoundReader_Resample_Fast.h \
RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
RageSoundResampler.cpp RageSoundResampler.h RageSounds.cpp RageSounds.h RageTexture.cpp RageTexture.h \
RageTextureID.cpp RageTextureID.h RageTextureManager.cpp RageTextureManager.h RageThreads.cpp RageThreads.h \
RageTimer.cpp RageTimer.h RageTypes.h RageUtil.cpp RageUtil.h RageUtil_CharConversions.cpp RageUtil_CharConversions.h \
RageUtil_FileDB.cpp RageUtil_FileDB.h RageUtil_CircularBuffer.h

Actors = \
Actor.cpp Actor.h ActorCollision.h ActorCommands.cpp ActorCommands.h ActorFrame.cpp ActorFrame.h \
ActorScroller.cpp ActorScroller.h ActorUtil.cpp ActorUtil.h BitmapText.cpp BitmapText.h Model.cpp Model.h \
ModelManager.cpp ModelManager.h ModelTypes.cpp ModelTypes.h Quad.h Sprite.cpp Sprite.h

GlobalSingletons = \
AnnouncerManager.cpp AnnouncerManager.h Bookkeeper.cpp Bookkeeper.h CryptManager.cpp CryptManager.h \
FontManager.cpp FontManager.h GameManager.cpp GameManager.h GameState.cpp GameState.h InputFilter.cpp InputFilter.h \
InputMapper.cpp InputMapper.h InputQueue.cpp InputQueue.h LightsManager.cpp LightsManager.h \
MemoryCardManager.cpp MemoryCardManager.h NetworkSyncManager.cpp NetworkSyncManager.h \
NoteSkinManager.cpp NoteSkinManager.h PrefsManager.cpp PrefsManager.h ProfileManager.cpp ProfileManager.h \
ScreenManager.cpp ScreenManager.h SongManager.cpp SongManager.h ThemeManager.cpp ThemeManager.h \
UnlockSystem.cpp UnlockSystem.h ezsockets.cpp ezsockets.h

crypto = \
crypto/CryptBn.cpp crypto/CryptBn.h crypto/CryptMD5.cpp crypto/CryptMD5.h crypto/CryptNoise.cpp \
crypto/CryptPrime.cpp crypto/CryptPrime.h crypto/CryptRSA.cpp crypto/CryptRSA.h \
crypto/CryptRand.cpp crypto/CryptRand.h crypto/CryptSH512.cpp crypto/CryptSH512.h \
crypto/CryptSHA.cpp crypto/CryptSHA.h

main_SOURCES = $(Screens) \
		$(DataStructures) \
		$(FileTypes) \
		$(StepMania) \
		$(Arch) \
		$(ActorsInGameplayAndMenus) \
		$(ActorsInMenus) \
		$(ActorsInGameplay) \
		$(Rage) \
		$(Actors) \
		$(GlobalSingletons) \
		$(crypto)

main_LDADD = \
		  $(AUDIO_LIBS) \
		  $(LUA_LIBS) \
		  $(SDL_LIBS) \
		  $(XLIBS) \
		  -lSDL_image \
		  $(srcdir)/libresample/libresample.a

stepmania_SOURCES = $(main_SOURCES)
stepmania_LDADD = $(main_LDADD)

if HAVE_GTK
bin_PROGRAMS += GtkModule.so
GtkModule_so_LDFLAGS = -rdynamic -shared
GtkModule_so_CPPFLAGS = $(GTK_CFLAGS)
GtkModule_so_LDADD = $(GTK_LIBS)
GtkModule_so_SOURCES = arch/LoadingWindow/LoadingWindow_GtkModule.cpp
endif

# XXX: Make ArchHooks not depend on Display
# Display depends on SaveJPEG  and SDL_utils, SDL_utils depends on SDL_rotozoom.
all_test_SOURCES = \
		$(ArchUtils) \
		$(PCRE) \
		$(RageFile) \
		arch/ArchHooks/ArchHooks.cpp arch/ArchHooks/ArchHooks.h \
		tests/test_misc.cpp tests/test_misc.h \
		IniFile.cpp IniFile.h \
		PrefsManager.cpp PrefsManager.h \
		RageDisplay.cpp RageDisplay.h \
		RageException.cpp \
		RageLog.cpp \
		RageMath.cpp RageMath.h \
		RageThreads.cpp \
		RageTimer.cpp RageTimer.h \
		RageUtil.cpp \
		RageUtil_FileDB.cpp RageUtil_FileDB.h \
		SDL_SaveJPEG.cpp SDL_SaveJPEG.h \
		SDL_rotozoom.cpp SDL_rotozoom.h \
		SDL_utils.cpp SDL_utils.h \
		global.cpp global.h

TESTS += test_audio_readers
test_audio_readers_SOURCES = \
		$(RageSoundFileReaders) \
		$(all_test_SOURCES) \
		tests/test_audio_readers.cpp \
		RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
		RageSoundReader_Preload.cpp RageSoundReader_Preload.h \
		RageSoundReader_Resample.cpp  RageSoundReader_Resample.h \
		RageSoundReader_Resample_Fast.cpp RageSoundReader_Resample_Fast.h \
		RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
		RageSoundResampler.cpp RageSoundResampler.h

test_audio_readers_LDADD = \
		$(AUDIO_LIBS) \
		$(SDL_LIBS) \
		$(srcdir)/libresample/libresample.a

TESTS += test_file_readers
test_file_readers_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_file_readers.cpp

TESTS += test_timing_data
test_timing_data_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_timing_data.cpp \
		TimingData.cpp TimingData.h

TESTS += test_threads
test_threads_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_threads.cpp

INCLUDES = $(LUA_CFLAGS) $(SDL_CFLAGS)
