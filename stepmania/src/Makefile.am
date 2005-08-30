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

AM_CXXFLAGS += $(XCFLAGS)

LIBS += $(LUA_LIBS)
LIBS += -lpthread

$(srcdir)/libresample/libresample.a:
	cd $(srcdir)/libresample && autoconf && sh ./configure && make

Screens = \
Screen.cpp Screen.h ScreenAttract.cpp ScreenAttract.h \
ScreenBookkeeping.cpp ScreenBookkeeping.h \
ScreenCenterImage.cpp ScreenCenterImage.h ScreenCredits.cpp ScreenCredits.h \
ScreenDebugOverlay.cpp ScreenDebugOverlay.h \
ScreenDemonstration.cpp ScreenDemonstration.h ScreenDimensions.h \
ScreenEdit.cpp ScreenEdit.h \
ScreenEditCourseMods.cpp ScreenEditCourseMods.h \
ScreenEditMenu.cpp ScreenEditMenu.h ScreenEnding.cpp ScreenEnding.h \
ScreenEvaluation.cpp ScreenEvaluation.h \
ScreenEvaluationMultiplayer.cpp ScreenEvaluationMultiplayer.h \
ScreenExit.cpp ScreenExit.h ScreenNetEvaluation.cpp ScreenNetEvaluation.h \
ScreenNetSelectMusic.cpp ScreenNetSelectMusic.h ScreenNetSelectBase.cpp ScreenNetSelectBase.h ScreenNetRoom.cpp ScreenNetRoom.h \
ScreenEz2SelectMusic.cpp ScreenEz2SelectMusic.h ScreenEz2SelectPlayer.cpp ScreenEz2SelectPlayer.h \
ScreenGameplay.cpp ScreenGameplay.h \
ScreenGameplayMultiplayer.cpp ScreenGameplayMultiplayer.h \
ScreenGameplayNormal.cpp ScreenGameplayNormal.h \
ScreenHowToPlay.cpp ScreenHowToPlay.h \
ScreenInstructions.cpp ScreenInstructions.h \
ScreenJoinMultiplayer.cpp ScreenJoinMultiplayer.h \
ScreenJukebox.cpp ScreenJukebox.h \
ScreenMapControllers.cpp ScreenMapControllers.h \
ScreenMessage.cpp ScreenMessage.h ScreenMiniMenu.cpp ScreenMiniMenu.h ScreenMusicScroll.cpp ScreenMusicScroll.h \
ScreenNameEntry.cpp ScreenNameEntry.h ScreenNameEntryTraditional.cpp ScreenNameEntryTraditional.h \
ScreenOptions.cpp ScreenOptions.h \
ScreenOptionsEditCourse.cpp ScreenOptionsEditCourse.h \
ScreenOptionsEditCourseEntry.cpp ScreenOptionsEditCourseEntry.h \
ScreenOptionsEditProfile.cpp ScreenOptionsEditProfile.h \
ScreenOptionsManageCourses.cpp ScreenOptionsManageCourses.h \
ScreenOptionsManageProfiles.cpp ScreenOptionsManageProfiles.h \
ScreenOptionsMaster.cpp ScreenOptionsMaster.h \
ScreenOptionsMasterPrefs.cpp ScreenOptionsMasterPrefs.h ScreenPackages.cpp ScreenPackages.h \
ScreenPlayerOptions.cpp ScreenPlayerOptions.h \
ScreenNetworkOptions.h ScreenNetworkOptions.cpp \
ScreenPrompt.cpp ScreenPrompt.h ScreenRanking.cpp ScreenRanking.h \
ScreenReloadSongs.cpp ScreenReloadSongs.h \
ScreenSandbox.cpp ScreenSandbox.h \
ScreenSaveSync.cpp ScreenSaveSync.h \
ScreenSelect.cpp ScreenSelect.h ScreenSelectCharacter.cpp ScreenSelectCharacter.h \
ScreenSelectDifficulty.cpp ScreenSelectDifficulty.h ScreenSelectGroup.cpp ScreenSelectGroup.h \
ScreenSelectMaster.cpp ScreenSelectMaster.h ScreenSelectMode.cpp ScreenSelectMode.h \
ScreenSelectMusic.cpp ScreenSelectMusic.h \
ScreenSelectStyle.cpp ScreenSelectStyle.h \
ScreenSyncOverlay.cpp ScreenSyncOverlay.h \
ScreenSystemLayer.cpp ScreenSystemLayer.h ScreenSetTime.cpp ScreenSetTime.h \
ScreenSongOptions.cpp ScreenSongOptions.h \
ScreenSplash.cpp ScreenSplash.h \
ScreenStage.cpp ScreenStage.h \
ScreenTest.cpp ScreenTest.h ScreenTestFonts.cpp ScreenTestFonts.h ScreenTestInput.cpp ScreenTestInput.h \
ScreenTestLights.cpp ScreenTestLights.h ScreenTestSound.cpp ScreenTestSound.h ScreenTextEntry.cpp ScreenTextEntry.h \
ScreenTitleMenu.cpp ScreenTitleMenu.h ScreenUnlock.cpp ScreenUnlock.h ScreenWithMenuElements.cpp ScreenWithMenuElements.h

if !WITHOUT_NETWORKING
Screens += ScreenGameplayMultiplayer.cpp ScreenGameplayMultiplayer.h ScreenSMOnlineLogin.cpp ScreenSMOnlineLogin.h
endif

DataStructures = \
ActorCommands.cpp ActorCommands.h Attack.cpp Attack.h AutoKeysounds.cpp AutoKeysounds.h \
BackgroundUtil.cpp BackgroundUtil.h \
BannerCache.cpp BannerCache.h CatalogXml.cpp CatalogXml.h \
Character.cpp Character.h \
CodeDetector.cpp CodeDetector.h \
Command.cpp Command.h \
CommonMetrics.cpp CommonMetrics.h \
Course.cpp Course.h \
CourseLoaderCRS.cpp CourseLoaderCRS.h \
CourseUtil.cpp CourseUtil.h \
CourseWriterCRS.cpp CourseWriterCRS.h \
DateTime.cpp DateTime.h \
Difficulty.cpp Difficulty.h \
EnumHelper.cpp EnumHelper.h \
Font.cpp Font.h \
FontCharAliases.cpp FontCharAliases.h \
FontCharmaps.cpp FontCharmaps.h Game.cpp Game.h GameCommand.cpp GameCommand.h \
GameConstantsAndTypes.cpp GameConstantsAndTypes.h \
GameInput.cpp GameInput.h Grade.cpp Grade.h \
HighScore.cpp HighScore.h \
InputEventPlus.cpp InputEventPlus.h \
Inventory.cpp Inventory.h LuaFunctions.h \
LuaReference.cpp LuaReference.h \
LyricsLoader.cpp LyricsLoader.h MenuInput.h \
NoteData.cpp NoteData.h NoteDataUtil.cpp NoteDataUtil.h NoteDataWithScoring.cpp NoteDataWithScoring.h \
NoteFieldPositioning.cpp NoteFieldPositioning.h NoteTypes.cpp NoteTypes.h NotesLoader.cpp NotesLoader.h \
NotesLoaderBMS.cpp NotesLoaderBMS.h NotesLoaderDWI.cpp NotesLoaderDWI.h NotesLoaderKSF.cpp NotesLoaderKSF.h \
NotesLoaderSM.cpp NotesLoaderSM.h NotesWriterDWI.cpp NotesWriterDWI.h NotesWriterSM.cpp NotesWriterSM.h \
OptionRowHandler.cpp OptionRowHandler.h \
PlayerAI.cpp PlayerAI.h PlayerNumber.cpp PlayerNumber.h PlayerOptions.cpp PlayerOptions.h \
PlayerStageStats.cpp PlayerStageStats.h \
PlayerState.cpp PlayerState.h Preference.cpp Preference.h Profile.cpp Profile.h \
RandomSample.cpp RandomSample.h RadarValues.cpp RadarValues.h \
RoomWheel.cpp RoomWheel.h ScreenDimensions.h ScreenDimensions.cpp \
ScoreKeeper.h ScoreKeeperMAX2.cpp ScoreKeeperMAX2.h \
ScoreKeeperRave.cpp ScoreKeeperRave.h Song.cpp song.h SongCacheIndex.cpp SongCacheIndex.h \
SongOptions.cpp SongOptions.h SongUtil.cpp SongUtil.h StageStats.cpp StageStats.h Steps.cpp Steps.h \
StepsUtil.cpp StepsUtil.h Style.cpp Style.h StyleUtil.cpp StyleUtil.h \
SubscriptionManager.h \
StyleInput.h TimingData.cpp TimingData.h \
ThemeMetric.h \
Trail.cpp Trail.h TrailUtil.cpp TrailUtil.h TitleSubstitution.cpp TitleSubstitution.h

FileTypes = IniFile.cpp IniFile.h \
		MsdFile.cpp MsdFile.h \
		XmlFile.cpp XmlFile.h

StepMania = StdString.h \
		StepMania.cpp StepMania.h \
		GameLoop.cpp GameLoop.h \
		global.cpp global.h

LoadingWindow = arch/LoadingWindow/LoadingWindow.h \
		arch/LoadingWindow/LoadingWindow_Null.h

if HAVE_GTK
LoadingWindow += arch/LoadingWindow/LoadingWindow_Gtk.cpp arch/LoadingWindow/LoadingWindow_Gtk.h
endif

Sound = arch/Sound/RageSoundDriver.h \
	arch/Sound/RageSoundDriver_Null.cpp arch/Sound/RageSoundDriver_Null.h \
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

ArchHooks = arch/ArchHooks/ArchHooks.cpp arch/ArchHooks/ArchHooks.h

InputHandler = arch/InputHandler/InputHandler.cpp arch/InputHandler/InputHandler.h \
		arch/InputHandler/InputHandler_MonkeyKeyboard.cpp arch/InputHandler/InputHandler_MonkeyKeyboard.h

MovieTexture = arch/MovieTexture/MovieTexture.cpp arch/MovieTexture/MovieTexture.h \
		arch/MovieTexture/MovieTexture_Null.cpp arch/MovieTexture/MovieTexture_Null.h

if HAVE_FFMPEG
MovieTexture += arch/MovieTexture/MovieTexture_FFMpeg.cpp arch/MovieTexture/MovieTexture_FFMpeg.h
endif

Lights = arch/Lights/LightsDriver.h \
		arch/Lights/LightsDriver_SystemMessage.cpp arch/Lights/LightsDriver_SystemMessage.h

MemoryCard = arch/MemoryCard/MemoryCardDriver.cpp arch/MemoryCard/MemoryCardDriver.h
LowLevelWindow = arch/LowLevelWindow/LowLevelWindow.h

ArchUtils =

Dialog = arch/Dialog/Dialog.cpp arch/Dialog/Dialog.h arch/Dialog/DialogDriver.h

Threads = arch/Threads/Threads.h

if HAVE_SDL
LoadingWindow += arch/LoadingWindow/LoadingWindow_SDL.cpp arch/LoadingWindow/LoadingWindow_SDL.h
StepMania += SDL_utils.cpp SDL_utils.h
if !HAVE_X11
#InputHandler_SDL depends on LowLevelWindow_SDL
InputHandler += arch/InputHandler/InputHandler_SDL.cpp arch/InputHandler/InputHandler_SDL.h
LowLevelWindow += arch/LowLevelWindow/LowLevelWindow_SDL.cpp arch/LowLevelWindow/LowLevelWindow_SDL.h
endif
endif

if HAVE_X11
# InputHandler_X11 depends on LowLevelWindow_X11
InputHandler += arch/InputHandler/InputHandler_X11.cpp arch/InputHandler/InputHandler_X11.h
LowLevelWindow += arch/LowLevelWindow/LowLevelWindow_X11.cpp arch/LowLevelWindow/LowLevelWindow_X11.h
LowLevelWindow += archutils/Unix/X11Helper.cpp archutils/Unix/X11Helper.h
endif

# Platform-specific drivers:

if UNIX
ArchHooks += arch/ArchHooks/ArchHooks_Unix.cpp arch/ArchHooks/ArchHooks_Unix.h
Threads += arch/Threads/Threads_Pthreads.cpp arch/Threads/Threads_Pthreads.h
Lights += arch/Lights/LightsDriver_LinuxWeedTech.cpp arch/Lights/LightsDriver_LinuxWeedTech.h
Lights += arch/Lights/LightsDriver_LinuxParallel.cpp arch/Lights/LightsDriver_LinuxParallel.h

ArchUtils +=    archutils/Unix/AssertionHandler.cpp \
		archutils/Unix/GetSysInfo.cpp archutils/Unix/GetSysInfo.h \
		archutils/Unix/SignalHandler.cpp archutils/Unix/SignalHandler.h \
		archutils/Unix/LinuxThreadHelpers.cpp archutils/Unix/LinuxThreadHelpers.h \
		archutils/Unix/RunningUnderValgrind.cpp archutils/Unix/RunningUnderValgrind.h \
		archutils/Unix/EmergencyShutdown.cpp archutils/Unix/EmergencyShutdown.h

if USE_CRASH_HANDLER
ArchUtils +=    archutils/Unix/Backtrace.cpp archutils/Unix/Backtrace.h \
		archutils/Unix/BacktraceNames.cpp archutils/Unix/BacktraceNames.h \
		archutils/Unix/CrashHandler.cpp archutils/Unix/CrashHandler.h \
		archutils/Unix/CrashHandlerChild.cpp archutils/Unix/CrashHandlerInternal.h
endif

endif

if LINUX
MemoryCard += arch/MemoryCard/MemoryCardDriverThreaded_Linux.cpp arch/MemoryCard/MemoryCardDriverThreaded_Linux.h
InputHandler +=	arch/InputHandler/InputHandler_Linux_Joystick.h arch/InputHandler/InputHandler_Linux_Joystick.cpp
endif

Arch = $(LoadingWindow) $(Sound) $(ArchHooks) $(InputHandler) $(MovieTexture) \
	   $(Lights) $(MemoryCard) $(LowLevelWindow) $(ArchUtils) $(Dialog) $(Threads) \
	   arch/arch.h arch/arch.cpp arch/arch_default.h arch/arch_platform.h

RageSoundFileReaders = RageSoundReader_WAV.cpp RageSoundReader_WAV.h 

if HAVE_VORBIS
RageSoundFileReaders += RageSoundReader_Vorbisfile.cpp RageSoundReader_Vorbisfile.h
endif

if HAVE_MP3
RageSoundFileReaders += RageSoundReader_MP3.cpp RageSoundReader_MP3.h
endif

ActorsInGameplayAndMenus = \
BGAnimation.cpp BGAnimation.h BGAnimationLayer.cpp BGAnimationLayer.h Banner.cpp Banner.h \
DifficultyIcon.cpp DifficultyIcon.h \
MeterDisplay.cpp MeterDisplay.h StreamDisplay.cpp StreamDisplay.h \
Transition.cpp Transition.h

ActorsInMenus = \
ActiveAttackList.cpp ActiveAttackList.h BPMDisplay.cpp BPMDisplay.h ComboGraph.cpp ComboGraph.h \
CourseContentsList.cpp CourseContentsList.h CourseEntryDisplay.cpp CourseEntryDisplay.h \
DifficultyDisplay.cpp DifficultyDisplay.h DifficultyList.cpp DifficultyList.h \
DifficultyMeter.cpp DifficultyMeter.h DifficultyRating.cpp DifficultyRating.h \
DualScrollBar.cpp DualScrollBar.h \
EditMenu.cpp EditMenu.h FadingBanner.cpp FadingBanner.h \
GradeDisplay.cpp GradeDisplay.h GraphDisplay.cpp GraphDisplay.h \
GrooveRadar.cpp GrooveRadar.h GroupList.cpp GroupList.h HelpDisplay.cpp HelpDisplay.h \
MemoryCardDisplay.cpp MemoryCardDisplay.h \
MenuTimer.cpp MenuTimer.h ModeSwitcher.cpp ModeSwitcher.h MusicBannerWheel.cpp MusicBannerWheel.h \
MusicList.cpp MusicList.h MusicSortDisplay.cpp MusicSortDisplay.h MusicWheel.cpp MusicWheel.h \
MusicWheelItem.cpp MusicWheelItem.h OptionIcon.cpp OptionIcon.h OptionIconRow.cpp OptionIconRow.h \
OptionRow.cpp OptionRow.h OptionsCursor.cpp OptionsCursor.h \
PaneDisplay.cpp PaneDisplay.h ScrollBar.cpp ScrollBar.h \
ScrollingList.cpp ScrollingList.h SnapDisplay.cpp SnapDisplay.h \
TextBanner.cpp TextBanner.h WheelBase.cpp WheelBase.h WheelItemBase.cpp WheelItemBase.h \
WheelNotifyIcon.cpp WheelNotifyIcon.h

ActorsInGameplay = \
ArrowEffects.cpp ArrowEffects.h AttackDisplay.cpp AttackDisplay.h \
Background.cpp Background.h BeginnerHelper.cpp BeginnerHelper.h CombinedLifeMeterTug.cpp CombinedLifeMeterTug.h \
Combo.cpp Combo.h DancingCharacters.cpp DancingCharacters.h Foreground.cpp Foreground.h GhostArrow.cpp GhostArrow.h \
GhostArrowRow.cpp GhostArrowRow.h HoldGhostArrow.cpp HoldGhostArrow.h HoldJudgment.cpp HoldJudgment.h \
Judgment.cpp Judgment.h LifeMeter.h LifeMeterBar.cpp LifeMeterBar.h \
LifeMeterBattery.cpp LifeMeterBattery.h LifeMeterTime.cpp LifeMeterTime.h \
LyricDisplay.cpp LyricDisplay.h NoteDisplay.cpp NoteDisplay.h NoteField.cpp NoteField.h \
PercentageDisplay.cpp PercentageDisplay.h Player.cpp Player.h \
ReceptorArrow.cpp ReceptorArrow.h ReceptorArrowRow.cpp ReceptorArrowRow.h \
ScoreDisplay.cpp ScoreDisplay.h \
ScoreDisplayBattle.cpp ScoreDisplayBattle.h \
ScoreDisplayCalories.cpp ScoreDisplayCalories.h \
ScoreDisplayLifeTime.cpp ScoreDisplayLifeTime.h \
ScoreDisplayNormal.cpp ScoreDisplayNormal.h ScoreDisplayOni.cpp ScoreDisplayOni.h \
ScoreDisplayPercentage.cpp ScoreDisplayPercentage.h ScoreDisplayRave.cpp ScoreDisplayRave.h

PCRE = pcre/chartables.c pcre/get.c pcre/internal.h pcre/maketables.c pcre/pcre.c pcre/pcre.h pcre/study.c

RageFile = \
RageFileBasic.cpp RageFileBasic.h \
RageFile.cpp RageFile.h RageFileDriver.cpp RageFileDriver.h RageFileManager.cpp RageFileManager.h \
RageFileDriverDirect.cpp RageFileDriverDirect.h RageFileDriverDirectHelpers.cpp RageFileDriverDirectHelpers.h \
RageFileDriverMemory.cpp RageFileDriverMemory.h RageFileDriverZip.cpp RageFileDriverZip.h \
RageFileDriverDeflate.cpp RageFileDriverDeflate.h \
RageFileDriverSlice.cpp RageFileDriverSlice.h \
RageFileDriverTimeout.cpp RageFileDriverTimeout.h

Rage = $(PCRE) $(RageFile) $(RageSoundFileReaders) \
RageBitmapTexture.cpp RageBitmapTexture.h RageDisplay.cpp RageDisplay.h RageDisplay_OGL.cpp RageDisplay_OGL.h \
RageDisplay_OGL_Extensions.cpp RageDisplay_OGL_Extensions.h \
RageDisplay_Null.cpp RageDisplay_Null.h RageException.cpp RageException.h RageInput.cpp RageInput.h \
RageInputDevice.cpp RageInputDevice.h RageLog.cpp RageLog.h RageMath.cpp RageMath.h \
RageModelGeometry.cpp RageModelGeometry.h RageSound.cpp RageSound.h RageSoundManager.cpp RageSoundManager.h \
RageSoundUtil.cpp RageSoundUtil.h RageSoundMixBuffer.cpp RageSoundMixBuffer.h \
RageSoundPosMap.cpp RageSoundPosMap.h RageSoundReader.h RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
RageSoundReader_Preload.cpp RageSoundReader_Preload.h RageSoundReader_Resample.cpp RageSoundReader_Resample.h \
RageSoundReader_Resample_Fast.cpp RageSoundReader_Resample_Fast.h \
RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
RageSoundReader_Chain.cpp RageSoundReader_Chain.h \
RageSoundResampler.cpp RageSoundResampler.h \
RageSurface.cpp RageSurface.h RageSurfaceUtils.cpp RageSurfaceUtils.h \
RageSurfaceUtils_Dither.cpp RageSurfaceUtils_Dither.h \
RageSurface_Save_JPEG.cpp RageSurface_Save_JPEG.h \
RageSurfaceUtils_Palettize.cpp RageSurfaceUtils_Palettize.h \
RageSurfaceUtils_Zoom.cpp RageSurfaceUtils_Zoom.h \
RageSurface_Load.cpp RageSurface_Load.h RageSurface_Load_PNG.cpp RageSurface_Load_PNG.h \
RageSurface_Load_JPEG.cpp RageSurface_Load_JPEG.h RageSurface_Load_GIF.cpp RageSurface_Load_GIF.h \
RageSurface_Load_BMP.cpp RageSurface_Load_BMP.h \
RageSurface_Load_XPM.cpp RageSurface_Load_XPM.h RageTexture.cpp RageTexture.h \
RageTexturePreloader.cpp RageTexturePreloader.h \
RageSurface_Save_BMP.cpp RageSurface_Save_BMP.h \
RageTextureID.cpp RageTextureID.h RageTextureManager.cpp RageTextureManager.h RageThreads.cpp RageThreads.h \
RageTimer.cpp RageTimer.h RageTypes.h RageUtil.cpp RageUtil.h RageUtil_CharConversions.cpp RageUtil_CharConversions.h \
RageUtil_BackgroundLoader.cpp RageUtil_BackgroundLoader.h RageUtil_FileDB.cpp RageUtil_FileDB.h RageUtil_CircularBuffer.h \
RageUtil_AutoPtr.h \
RageUtil_WorkerThread.cpp RageUtil_WorkerThread.h

Actors = \
Actor.cpp Actor.h ActorCollision.h ActorFrame.cpp ActorFrame.h \
ActorScroller.cpp ActorScroller.h ActorUtil.cpp ActorUtil.h \
AutoActor.cpp AutoActor.h \
BitmapText.cpp BitmapText.h Model.cpp Model.h \
ModelManager.cpp ModelManager.h ModelTypes.cpp ModelTypes.h \
Quad.cpp Quad.h \
RollingNumbers.cpp RollingNumbers.h Sprite.cpp Sprite.h

GlobalSingletons = \
AnnouncerManager.cpp AnnouncerManager.h \
Bookkeeper.cpp Bookkeeper.h \
CharacterManager.cpp CharacterManager.h \
CryptManager.cpp CryptManager.h \
FontManager.cpp FontManager.h GameSoundManager.cpp GameSoundManager.h \
GameManager.cpp GameManager.h \
GameState.cpp GameState.h \
InputFilter.cpp InputFilter.h \
InputMapper.cpp InputMapper.h InputQueue.cpp InputQueue.h \
LuaBinding.cpp LuaBinding.h \
LuaManager.cpp LuaManager.h LightsManager.cpp LightsManager.h \
MemoryCardManager.cpp MemoryCardManager.h \
MessageManager.cpp MessageManager.h NetworkSyncManager.cpp NetworkSyncManager.h \
NetworkSyncServer.cpp NetworkSyncServer.h NoteSkinManager.cpp NoteSkinManager.h \
PrefsManager.cpp PrefsManager.h ProfileManager.cpp ProfileManager.h ScreenManager.cpp \
ScreenManager.h SongManager.cpp SongManager.h \
StatsManager.cpp StatsManager.h \
ThemeManager.cpp ThemeManager.h \
UnlockManager.cpp UnlockManager.h

if !WITHOUT_NETWORKING
# Compile NetworkSyncManager even if networking is disabled; it'll stub itself.
GlobalSingletons += ezsockets.cpp ezsockets.h
endif

crypto = \
crypto/CryptBn.cpp crypto/CryptBn.h crypto/CryptMD5.cpp crypto/CryptMD5.h crypto/CryptNoise.cpp \
crypto/CryptPrime.cpp crypto/CryptPrime.h crypto/CryptRSA.cpp crypto/CryptRSA.h \
crypto/CryptRand.cpp crypto/CryptRand.h crypto/CryptSH512.cpp crypto/CryptSH512.h \
crypto/CryptSHA.cpp crypto/CryptSHA.h

cryptopp = \
crypto51/algebra.cpp crypto51/algebra.h crypto51/algparam.cpp crypto51/algparam.h crypto51/argnames.h \
crypto51/asn.cpp crypto51/asn.h crypto51/config.h crypto51/cryptlib.cpp crypto51/cryptlib.h crypto51/files.cpp \
crypto51/files.h crypto51/filters.cpp crypto51/filters.h crypto51/fltrimpl.h crypto51/integer.cpp crypto51/integer.h \
crypto51/iterhash.cpp crypto51/iterhash.h crypto51/misc.cpp crypto51/misc.h crypto51/modarith.h \
crypto51/mqueue.cpp crypto51/mqueue.h crypto51/nbtheory.cpp crypto51/nbtheory.h \
crypto51/oids.h crypto51/osrng.cpp crypto51/osrng.h crypto51/pch.h crypto51/pkcspad.cpp \
crypto51/pkcspad.h crypto51/pubkey.cpp crypto51/pubkey.h crypto51/queue.cpp crypto51/queue.h \
crypto51/rsa.cpp crypto51/rsa.h crypto51/secblock.h crypto51/seckey.h crypto51/sha.cpp \
crypto51/sha.h crypto51/simple.h crypto51/smartptr.h crypto51/words.h \
CryptHelpers.cpp CryptHelpers.h

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
		$(crypto) \
		$(cryptopp)

main_LDADD = \
		  $(AUDIO_LIBS) \
		  $(SDL_LIBS) \
		  $(XLIBS) \
		  $(srcdir)/libresample/libresample.a

stepmania_SOURCES = $(main_SOURCES)
stepmania_LDADD = $(main_LDADD)

if HAVE_GTK
bin_PROGRAMS += GtkModule.so
GtkModule_so_LDFLAGS = -rdynamic -shared
GtkModule_so_CPPFLAGS = $(GTK_CFLAGS) -fPIC
GtkModule_so_LDADD = $(GTK_LIBS)
GtkModule_so_SOURCES = arch/LoadingWindow/LoadingWindow_GtkModule.cpp
endif

all_test_SOURCES = \
		$(ArchUtils) \
		$(PCRE) \
		$(RageFile) \
		$(ArchHooks) \
		$(Threads) \
		$(Dialog) \
		tests/test_misc.cpp tests/test_misc.h \
		IniFile.cpp IniFile.h \
		XmlFile.cpp XmlFile.h \
		DateTime.cpp DateTime.h \
		PrefsManager.cpp PrefsManager.h \
		Preference.cpp Preference.h \
		LuaBinding.cpp LuaBinding.h \
		LuaFunctions.h LuaHelpers.cpp LuaHelpers.h \
		LuaReference.cpp LuaReference.h \
		RageException.cpp \
		RageLog.cpp \
		RageThreads.cpp \
		RageTimer.cpp RageTimer.h \
		RageUtil.cpp \
		RageUtil_FileDB.cpp RageUtil_FileDB.h \
		global.cpp global.h

TESTS += test_audio_readers
test_audio_readers_SOURCES = \
		$(RageSoundFileReaders) \
		$(all_test_SOURCES) \
		tests/test_audio_readers.cpp \
		RageSoundUtil.cpp RageSoundUtil.h RageSoundMixBuffer.cpp RageSoundMixBuffer.h \
		RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
		RageSoundReader_Preload.cpp RageSoundReader_Preload.h \
		RageSoundReader_Resample.cpp  RageSoundReader_Resample.h \
		RageSoundReader_Resample_Fast.cpp RageSoundReader_Resample_Fast.h \
		RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
		RageSoundReader_Chain.cpp RageSoundReader_Chain.h \
		RageSoundResampler.cpp RageSoundResampler.h

test_audio_readers_LDADD = \
		$(AUDIO_LIBS) \
		$(srcdir)/libresample/libresample.a

TESTS += test_file_readers
test_file_readers_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_file_readers.cpp

TESTS += test_file_errors
test_file_errors_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_file_errors.cpp \
		MsdFile.cpp MsdFile.h \
		XmlFile.cpp XmlFile.h \
		CryptManager.cpp CryptManager.h $(cryptopp) $(crypto)

TESTS += test_timing_data
test_timing_data_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_timing_data.cpp \
		TimingData.cpp TimingData.h

#TESTS += test_notedata
#test_notedata_SOURCES = \
#		$(all_test_SOURCES) \
#		tests/test_notedata.cpp \
#		NoteData.cpp NoteData.h NoteDataUtil.cpp NoteDataUtil.h \
#		NewNoteData.cpp NewNoteData.h \
#		NoteTypes.cpp NoteTypes.h \
#		TimingData.cpp TimingData.h
#NewNoteDataUtil.cpp NewNoteDataUtil.h

TESTS += test_threads
test_threads_SOURCES = \
		$(all_test_SOURCES) \
		tests/test_threads.cpp

INCLUDES = $(LUA_CFLAGS) $(SDL_CFLAGS)
