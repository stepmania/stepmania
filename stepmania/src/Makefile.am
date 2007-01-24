bin_PROGRAMS = stepmania
TESTS = 
tests: $(TESTS)
if BUILD_TESTS
noinst_PROGRAMS = $(TESTS)
endif

AM_LDFLAGS =
AM_CXXFLAGS = 
AM_CFLAGS = 
noinst_LIBRARIES =

# Relax inlining; the default of 600 takes way too long to compile and
# generates enormous output.
AM_CXXFLAGS += -finline-limit=300

AM_CXXFLAGS += $(XCFLAGS)

LIBS += 
LIBS += -lpthread -lrt

Screens = \
Screen.cpp Screen.h ScreenAttract.cpp ScreenAttract.h \
ScreenBookkeeping.cpp ScreenBookkeeping.h \
ScreenCenterImage.cpp ScreenCenterImage.h ScreenCredits.cpp ScreenCredits.h \
ScreenDebugOverlay.cpp ScreenDebugOverlay.h \
ScreenDemonstration.cpp ScreenDemonstration.h ScreenDimensions.h \
ScreenEdit.cpp ScreenEdit.h \
ScreenEditMenu.cpp ScreenEditMenu.h ScreenEnding.cpp ScreenEnding.h \
ScreenEvaluation.cpp ScreenEvaluation.h \
ScreenExit.cpp ScreenExit.h ScreenNetEvaluation.cpp ScreenNetEvaluation.h \
ScreenNetSelectMusic.cpp ScreenNetSelectMusic.h ScreenNetSelectBase.cpp ScreenNetSelectBase.h ScreenNetRoom.cpp ScreenNetRoom.h \
ScreenEz2SelectMusic.cpp ScreenEz2SelectMusic.h ScreenEz2SelectPlayer.cpp ScreenEz2SelectPlayer.h \
ScreenGameplay.cpp ScreenGameplay.h \
ScreenGameplayLesson.cpp ScreenGameplayLesson.h \
ScreenGameplayNormal.cpp ScreenGameplayNormal.h \
ScreenGameplaySyncMachine.cpp ScreenGameplaySyncMachine.h \
ScreenHowToPlay.cpp ScreenHowToPlay.h \
ScreenInstructions.cpp ScreenInstructions.h \
ScreenJukebox.cpp ScreenJukebox.h \
ScreenMapControllers.cpp ScreenMapControllers.h \
ScreenMessage.cpp ScreenMessage.h ScreenMiniMenu.cpp ScreenMiniMenu.h ScreenMusicScroll.cpp ScreenMusicScroll.h \
ScreenNameEntry.cpp ScreenNameEntry.h ScreenNameEntryTraditional.cpp ScreenNameEntryTraditional.h \
ScreenOptions.cpp ScreenOptions.h \
ScreenOptionsEditCourse.cpp ScreenOptionsEditCourse.h \
ScreenOptionsEditCourseEntry.cpp ScreenOptionsEditCourseEntry.h \
ScreenOptionsEditProfile.cpp ScreenOptionsEditProfile.h \
ScreenOptionsManageCourses.cpp ScreenOptionsManageCourses.h \
ScreenOptionsManageEditSteps.cpp ScreenOptionsManageEditSteps.h \
ScreenOptionsManageProfiles.cpp ScreenOptionsManageProfiles.h \
ScreenOptionsMaster.cpp ScreenOptionsMaster.h \
ScreenOptionsMasterPrefs.cpp ScreenOptionsMasterPrefs.h ScreenPackages.cpp ScreenPackages.h \
ScreenPlayerOptions.cpp ScreenPlayerOptions.h \
ScreenNetworkOptions.h ScreenNetworkOptions.cpp \
ScreenPrompt.cpp ScreenPrompt.h ScreenRanking.cpp ScreenRanking.h \
ScreenReloadSongs.cpp ScreenReloadSongs.h \
ScreenSandbox.cpp ScreenSandbox.h \
ScreenSaveSync.cpp ScreenSaveSync.h \
ScreenServiceAction.cpp ScreenServiceAction.h \
ScreenStatsOverlay.cpp ScreenStatsOverlay.h \
ScreenSelect.cpp ScreenSelect.h ScreenSelectCharacter.cpp ScreenSelectCharacter.h \
ScreenSelectDifficulty.cpp ScreenSelectDifficulty.h ScreenSelectGroup.cpp ScreenSelectGroup.h \
ScreenSelectLanguage.cpp  ScreenSelectLanguage.h \
ScreenSelectMaster.cpp ScreenSelectMaster.h ScreenSelectMode.cpp ScreenSelectMode.h \
ScreenSelectMusic.cpp ScreenSelectMusic.h \
ScreenSelectStyle.cpp ScreenSelectStyle.h \
ScreenSyncOverlay.cpp ScreenSyncOverlay.h \
ScreenSystemLayer.cpp ScreenSystemLayer.h ScreenSetTime.cpp ScreenSetTime.h \
ScreenSongOptions.cpp ScreenSongOptions.h \
ScreenSplash.cpp ScreenSplash.h \
ScreenStage.cpp ScreenStage.h \
ScreenTestFonts.cpp ScreenTestFonts.h ScreenTestInput.cpp ScreenTestInput.h \
ScreenTestLights.cpp ScreenTestLights.h ScreenTestSound.cpp ScreenTestSound.h ScreenTextEntry.cpp ScreenTextEntry.h \
ScreenTitleMenu.cpp ScreenTitleMenu.h \
ScreenUnlockBrowse.cpp ScreenUnlockBrowse.h \
ScreenUnlockCelebrate.cpp ScreenUnlockCelebrate.h \
ScreenUnlockStatus.cpp ScreenUnlockStatus.h \
ScreenWithMenuElements.cpp ScreenWithMenuElements.h

if !WITHOUT_NETWORKING
Screens += ScreenSMOnlineLogin.cpp ScreenSMOnlineLogin.h
endif

DataStructures = \
Attack.cpp Attack.h AutoKeysounds.cpp AutoKeysounds.h \
AdjustSync.cpp AdjustSync.h \
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
GamePreferences.cpp GamePreferences.h \
GameInput.cpp GameInput.h Grade.cpp Grade.h \
HighScore.cpp HighScore.h \
InputEventPlus.h \
Inventory.cpp Inventory.h \
LocalizedString.cpp LocalizedString.h \
LuaReference.cpp LuaReference.h \
LuaExpressionTransform.cpp LuaExpressionTransform.h \
LyricsLoader.cpp LyricsLoader.h \
ModsGroup.cpp ModsGroup.h \
NoteData.cpp NoteData.h NoteDataUtil.cpp NoteDataUtil.h NoteDataWithScoring.cpp NoteDataWithScoring.h \
NoteTypes.cpp NoteTypes.h NotesLoader.cpp NotesLoader.h \
NotesLoaderBMS.cpp NotesLoaderBMS.h NotesLoaderDWI.cpp NotesLoaderDWI.h NotesLoaderKSF.cpp NotesLoaderKSF.h \
NotesLoaderSM.cpp NotesLoaderSM.h NotesWriterDWI.cpp NotesWriterDWI.h NotesWriterSM.cpp NotesWriterSM.h \
OptionRowHandler.cpp OptionRowHandler.h \
PlayerAI.cpp PlayerAI.h PlayerNumber.cpp PlayerNumber.h PlayerOptions.cpp PlayerOptions.h \
PlayerStageStats.cpp PlayerStageStats.h \
PlayerState.cpp PlayerState.h Preference.cpp Preference.h Profile.cpp Profile.h \
RandomSample.cpp RandomSample.h RadarValues.cpp RadarValues.h \
SampleHistory.cpp SampleHistory.h \
ScreenDimensions.h ScreenDimensions.cpp \
ScoreKeeper.h ScoreKeeper.cpp \
ScoreKeeperGuitar.cpp ScoreKeeperGuitar.h \
ScoreKeeperNormal.cpp ScoreKeeperNormal.h \
ScoreKeeperRave.cpp ScoreKeeperRave.h \
Song.cpp song.h SongCacheIndex.cpp SongCacheIndex.h \
SongOptions.cpp SongOptions.h SongUtil.cpp SongUtil.h StageStats.cpp StageStats.h Steps.cpp Steps.h \
SoundEffectControl.cpp SoundEffectControl.h \
StepsUtil.cpp StepsUtil.h Style.cpp Style.h StyleUtil.cpp StyleUtil.h \
SubscriptionManager.h \
TimingData.cpp TimingData.h \
ThemeMetric.h \
Trail.cpp Trail.h TrailUtil.cpp TrailUtil.h TitleSubstitution.cpp TitleSubstitution.h \
Tween.cpp Tween.h \
Workout.cpp Workout.h

if !WITHOUT_NETWORKING
DataStructures += RoomWheel.cpp RoomWheel.h
endif

FileTypes = IniFile.cpp IniFile.h \
MsdFile.cpp MsdFile.h \
XmlFile.cpp XmlFile.h \
XmlFileUtil.cpp XmlFileUtil.h

StepMania = ExportStrings.h ExportStrings.cpp \
StdString.h \
StepMania.cpp StepMania.h \
GameLoop.cpp GameLoop.h \
global.cpp global.h \
SpecialFiles.cpp SpecialFiles.h

LoadingWindow = arch/LoadingWindow/LoadingWindow.cpp \
		arch/LoadingWindow/LoadingWindow.h \
		arch/LoadingWindow/LoadingWindow_Null.h

if HAVE_GTK
LoadingWindow += arch/LoadingWindow/LoadingWindow_Gtk.cpp arch/LoadingWindow/LoadingWindow_Gtk.h
endif

Sound = arch/Sound/RageSoundDriver.h arch/Sound/RageSoundDriver.cpp \
	arch/Sound/RageSoundDriver_Null.cpp arch/Sound/RageSoundDriver_Null.h \
	arch/Sound/RageSoundDriver_Generic_Software.cpp


if HAVE_OSS
Sound += arch/Sound/RageSoundDriver_OSS.cpp arch/Sound/RageSoundDriver_OSS.h
endif

if HAVE_ALSA
Sound += arch/Sound/ALSA9Dynamic.cpp arch/Sound/ALSA9Dynamic.h arch/Sound/ALSA9Functions.h \
	 arch/Sound/ALSA9Helpers.cpp arch/Sound/ALSA9Helpers.h \
	 arch/Sound/RageSoundDriver_ALSA9_Software.cpp arch/Sound/RageSoundDriver_ALSA9_Software.h

# Add this to AM_CXXFLAGS, not stepmania_CXXFLAGS: that'll cause multiple copies of
# several sources to be built. We link to this at runtime, so don't add ALSA_LDFLAGS.
AM_CXXFLAGS += $(ALSA_CFLAGS)
endif

ArchHooks = arch/ArchHooks/ArchHooks.cpp arch/ArchHooks/ArchHooks.h \
	arch/ArchHooks/ArchHooksUtil.cpp

InputHandler = arch/InputHandler/InputHandler.cpp arch/InputHandler/InputHandler.h \
		arch/InputHandler/InputHandler_MonkeyKeyboard.cpp arch/InputHandler/InputHandler_MonkeyKeyboard.h

MovieTexture = arch/MovieTexture/MovieTexture.cpp arch/MovieTexture/MovieTexture.h \
		arch/MovieTexture/MovieTexture_Generic.cpp arch/MovieTexture/MovieTexture_Generic.h \
		arch/MovieTexture/MovieTexture_Null.cpp arch/MovieTexture/MovieTexture_Null.h

if HAVE_THEORA
MovieTexture += arch/MovieTexture/MovieTexture_Theora.cpp arch/MovieTexture/MovieTexture_Theora.h
endif

if HAVE_FFMPEG
MovieTexture += arch/MovieTexture/MovieTexture_FFMpeg.cpp arch/MovieTexture/MovieTexture_FFMpeg.h
endif

Lights = arch/Lights/LightsDriver.cpp arch/Lights/LightsDriver.h \
		arch/Lights/LightsDriver_Export.cpp arch/Lights/LightsDriver_Export.h \
		arch/Lights/LightsDriver_SystemMessage.cpp arch/Lights/LightsDriver_SystemMessage.h

MemoryCard = arch/MemoryCard/MemoryCardDriver.cpp arch/MemoryCard/MemoryCardDriver.h
LowLevelWindow = arch/LowLevelWindow/LowLevelWindow.cpp arch/LowLevelWindow/LowLevelWindow.h

ArchUtils =

Dialog = arch/Dialog/Dialog.cpp arch/Dialog/Dialog.h arch/Dialog/DialogDriver.h arch/Dialog/DialogDriver.cpp

Threads = arch/Threads/Threads.h

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
if LINUX
Lights += arch/Lights/LightsDriver_LinuxWeedTech.cpp arch/Lights/LightsDriver_LinuxWeedTech.h
if HAVE_PARALLEL_PORT
Lights += arch/Lights/LightsDriver_LinuxParallel.cpp arch/Lights/LightsDriver_LinuxParallel.h
endif
endif

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
		archutils/Unix/CrashHandlerChild.cpp archutils/Unix/CrashHandlerInternal.h \
		archutils/Unix/CrashHandlerInternal.cpp
endif

endif

if LINUX
MemoryCard += arch/MemoryCard/MemoryCardDriverThreaded_Linux.cpp arch/MemoryCard/MemoryCardDriverThreaded_Linux.h
InputHandler +=	arch/InputHandler/InputHandler_Linux_Joystick.h arch/InputHandler/InputHandler_Linux_Joystick.cpp
endif

Arch = $(LoadingWindow) $(Sound) $(ArchHooks) $(InputHandler) $(MovieTexture) \
	   $(Lights) $(MemoryCard) $(LowLevelWindow) $(ArchUtils) $(Dialog) $(Threads) \
	   arch/arch.h arch/arch_default.h arch/arch_platform.h \
	   arch/RageDriver.cpp arch/RageDriver.h

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
CourseContentsList.cpp CourseContentsList.h \
CourseEntryDisplay.cpp CourseEntryDisplay.h \
DifficultyList.cpp DifficultyList.h \
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
WheelNotifyIcon.cpp WheelNotifyIcon.h \
WorkoutGraph.cpp WorkoutGraph.h 

if !WITHOUT_NETWORKING
ActorsInMenus += RoomInfoDisplay.cpp RoomInfoDisplay.h
endif

ActorsInGameplay = \
ArrowEffects.cpp ArrowEffects.h AttackDisplay.cpp AttackDisplay.h \
Background.cpp Background.h BeginnerHelper.cpp BeginnerHelper.h CombinedLifeMeterTug.cpp CombinedLifeMeterTug.h \
Combo.cpp Combo.h DancingCharacters.cpp DancingCharacters.h Foreground.cpp Foreground.h \
GhostArrowRow.cpp GhostArrowRow.h HoldJudgment.cpp HoldJudgment.h \
Judgment.cpp Judgment.h \
LifeMeter.cpp LifeMeter.h \
LifeMeterBar.cpp LifeMeterBar.h \
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

Lua = lua-5.1/src/lapi.c lua-5.1/src/lauxlib.c lua-5.1/src/lbaselib.c lua-5.1/src/lcode.c lua-5.1/src/ldblib.c \
lua-5.1/src/ldebug.c lua-5.1/src/ldo.c lua-5.1/src/ldump.c lua-5.1/src/lfunc.c lua-5.1/src/lgc.c lua-5.1/src/linit.c \
lua-5.1/src/liolib.c lua-5.1/src/llex.c lua-5.1/src/lmathlib.c lua-5.1/src/lmem.c lua-5.1/src/loadlib.c \
lua-5.1/src/lobject.c lua-5.1/src/lopcodes.c lua-5.1/src/loslib.c lua-5.1/src/lparser.c lua-5.1/src/lstate.c \
lua-5.1/src/lstring.c lua-5.1/src/lstrlib.c lua-5.1/src/ltable.c lua-5.1/src/ltablib.c lua-5.1/src/ltm.c \
lua-5.1/src/lundump.c lua-5.1/src/lvm.c lua-5.1/src/lzio.c lua-5.1/src/lapi.h lua-5.1/src/lauxlib.h lua-5.1/src/lcode.h \
lua-5.1/src/ldebug.h lua-5.1/src/ldo.h lua-5.1/src/lfunc.h lua-5.1/src/lgc.h lua-5.1/src/llex.h lua-5.1/src/llimits.h \
lua-5.1/src/lmem.h lua-5.1/src/lobject.h lua-5.1/src/lopcodes.h lua-5.1/src/lparser.h lua-5.1/src/lstate.h \
lua-5.1/src/lstring.h lua-5.1/src/ltable.h lua-5.1/src/ltm.h lua-5.1/src/luaconf.h lua-5.1/src/lua.h lua-5.1/src/lualib.h \
lua-5.1/src/lundump.h lua-5.1/src/lvm.h lua-5.1/src/lzio.h

RageFile = \
RageFileBasic.cpp RageFileBasic.h \
RageFile.cpp RageFile.h RageFileDriver.cpp RageFileDriver.h RageFileManager.cpp RageFileManager.h \
RageFileDriverDirect.cpp RageFileDriverDirect.h RageFileDriverDirectHelpers.cpp RageFileDriverDirectHelpers.h \
RageFileDriverMemory.cpp RageFileDriverMemory.h RageFileDriverZip.cpp RageFileDriverZip.h \
RageFileDriverDeflate.cpp RageFileDriverDeflate.h \
RageFileDriverSlice.cpp RageFileDriverSlice.h \
RageFileDriverTimeout.cpp RageFileDriverTimeout.h

Rage = $(PCRE) $(Lua) $(RageFile) $(RageSoundFileReaders) \
RageBitmapTexture.cpp RageBitmapTexture.h RageDisplay.cpp RageDisplay.h RageDisplay_OGL.cpp RageDisplay_OGL.h \
RageDisplay_OGL_Helpers.cpp RageDisplay_OGL_Helpers.h \
RageDisplay_Null.cpp RageDisplay_Null.h RageException.cpp RageException.h RageInput.cpp RageInput.h \
RageInputDevice.cpp RageInputDevice.h RageLog.cpp RageLog.h RageMath.cpp RageMath.h \
RageModelGeometry.cpp RageModelGeometry.h RageSound.cpp RageSound.h RageSoundManager.cpp RageSoundManager.h \
RageSoundUtil.cpp RageSoundUtil.h RageSoundMixBuffer.cpp RageSoundMixBuffer.h \
RageSoundPosMap.cpp RageSoundPosMap.h RageSoundReader.cpp RageSoundReader.h RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
RageSoundReader_Filter.h \
RageSoundReader_ChannelSplit.cpp RageSoundReader_ChannelSplit.h \
RageSoundReader_Extend.cpp RageSoundReader_Extend.h \
RageSoundReader_PitchChange.cpp RageSoundReader_PitchChange.h \
RageSoundReader_PostBuffering.cpp RageSoundReader_PostBuffering.h \
RageSoundReader_Pan.cpp RageSoundReader_Pan.h \
RageSoundReader_Preload.cpp RageSoundReader_Preload.h \
RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
RageSoundReader_SpeedChange.cpp RageSoundReader_SpeedChange.h \
RageSoundReader_ThreadedBuffer.cpp RageSoundReader_ThreadedBuffer.h \
RageSoundReader_Chain.cpp RageSoundReader_Chain.h \
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
RageTextureRenderTarget.cpp RageTextureRenderTarget.h \
RageSurface_Save_BMP.cpp RageSurface_Save_BMP.h \
RageSurface_Save_PNG.cpp RageSurface_Save_PNG.h \
RageTextureID.cpp RageTextureID.h RageTextureManager.cpp RageTextureManager.h RageThreads.cpp RageThreads.h \
RageTimer.cpp RageTimer.h RageTypes.cpp RageTypes.h RageUtil.cpp RageUtil.h RageUtil_CharConversions.cpp RageUtil_CharConversions.h \
RageUtil_BackgroundLoader.cpp RageUtil_BackgroundLoader.h RageUtil_FileDB.cpp RageUtil_FileDB.h RageUtil_CircularBuffer.h \
RageUtil_AutoPtr.h \
RageUtil_WorkerThread.cpp RageUtil_WorkerThread.h

Actors = \
Actor.cpp Actor.h ActorFrame.cpp ActorFrame.h \
ActorProxy.cpp ActorProxy.h \
ActorScroller.cpp ActorScroller.h ActorUtil.cpp ActorUtil.h \
ActorSound.cpp ActorSound.h \
AutoActor.cpp AutoActor.h \
BitmapText.cpp BitmapText.h Model.cpp Model.h \
DynamicActorScroller.cpp DynamicActorScroller.h \
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
NoteSkinManager.cpp NoteSkinManager.h \
PrefsManager.cpp PrefsManager.h ProfileManager.cpp ProfileManager.h ScreenManager.cpp \
ScreenManager.h SongManager.cpp SongManager.h \
StatsManager.cpp StatsManager.h \
ThemeManager.cpp ThemeManager.h \
UnlockManager.cpp UnlockManager.h \
WorkoutManager.cpp WorkoutManager.h

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

libtommath_a_SOURCES = \
libtommath/bncore.c libtommath/bn_mp_init.c libtommath/bn_mp_clear.c libtommath/bn_mp_exch.c libtommath/bn_mp_grow.c libtommath/bn_mp_shrink.c \
libtommath/bn_mp_clamp.c libtommath/bn_mp_zero.c  libtommath/bn_mp_set.c libtommath/bn_mp_set_int.c libtommath/bn_mp_init_size.c libtommath/bn_mp_copy.c \
libtommath/bn_mp_init_copy.c libtommath/bn_mp_abs.c libtommath/bn_mp_neg.c libtommath/bn_mp_cmp_mag.c libtommath/bn_mp_cmp.c libtommath/bn_mp_cmp_d.c \
libtommath/bn_mp_rshd.c libtommath/bn_mp_lshd.c libtommath/bn_mp_mod_2d.c libtommath/bn_mp_div_2d.c libtommath/bn_mp_mul_2d.c libtommath/bn_mp_div_2.c \
libtommath/bn_mp_mul_2.c libtommath/bn_s_mp_add.c libtommath/bn_s_mp_sub.c libtommath/bn_fast_s_mp_mul_digs.c libtommath/bn_s_mp_mul_digs.c \
libtommath/bn_fast_s_mp_mul_high_digs.c libtommath/bn_s_mp_mul_high_digs.c libtommath/bn_fast_s_mp_sqr.c libtommath/bn_s_mp_sqr.c \
libtommath/bn_mp_add.c libtommath/bn_mp_sub.c libtommath/bn_mp_karatsuba_mul.c libtommath/bn_mp_mul.c libtommath/bn_mp_karatsuba_sqr.c \
libtommath/bn_mp_sqr.c libtommath/bn_mp_div.c libtommath/bn_mp_mod.c libtommath/bn_mp_add_d.c libtommath/bn_mp_sub_d.c libtommath/bn_mp_mul_d.c \
libtommath/bn_mp_div_d.c libtommath/bn_mp_mod_d.c libtommath/bn_mp_expt_d.c libtommath/bn_mp_addmod.c libtommath/bn_mp_submod.c \
libtommath/bn_mp_mulmod.c libtommath/bn_mp_sqrmod.c libtommath/bn_mp_gcd.c libtommath/bn_mp_lcm.c libtommath/bn_fast_mp_invmod.c libtommath/bn_mp_invmod.c \
libtommath/bn_mp_reduce.c libtommath/bn_mp_montgomery_setup.c libtommath/bn_fast_mp_montgomery_reduce.c libtommath/bn_mp_montgomery_reduce.c \
libtommath/bn_mp_exptmod_fast.c libtommath/bn_mp_exptmod.c libtommath/bn_mp_2expt.c libtommath/bn_mp_n_root.c libtommath/bn_mp_jacobi.c libtommath/bn_reverse.c \
libtommath/bn_mp_count_bits.c libtommath/bn_mp_read_unsigned_bin.c libtommath/bn_mp_read_signed_bin.c libtommath/bn_mp_to_unsigned_bin.c \
libtommath/bn_mp_to_signed_bin.c libtommath/bn_mp_unsigned_bin_size.c libtommath/bn_mp_signed_bin_size.c  \
libtommath/bn_mp_xor.c libtommath/bn_mp_and.c libtommath/bn_mp_or.c libtommath/bn_mp_rand.c libtommath/bn_mp_montgomery_calc_normalization.c \
libtommath/bn_mp_prime_is_divisible.c libtommath/bn_prime_tab.c libtommath/bn_mp_prime_fermat.c libtommath/bn_mp_prime_miller_rabin.c \
libtommath/bn_mp_prime_is_prime.c libtommath/bn_mp_prime_next_prime.c libtommath/bn_mp_dr_reduce.c \
libtommath/bn_mp_dr_is_modulus.c libtommath/bn_mp_dr_setup.c libtommath/bn_mp_reduce_setup.c \
libtommath/bn_mp_toom_mul.c libtommath/bn_mp_toom_sqr.c libtommath/bn_mp_div_3.c libtommath/bn_s_mp_exptmod.c \
libtommath/bn_mp_reduce_2k.c libtommath/bn_mp_reduce_is_2k.c libtommath/bn_mp_reduce_2k_setup.c \
libtommath/bn_mp_reduce_2k_l.c libtommath/bn_mp_reduce_is_2k_l.c libtommath/bn_mp_reduce_2k_setup_l.c \
libtommath/bn_mp_radix_smap.c libtommath/bn_mp_read_radix.c libtommath/bn_mp_toradix.c libtommath/bn_mp_radix_size.c \
libtommath/bn_mp_fread.c libtommath/bn_mp_fwrite.c libtommath/bn_mp_cnt_lsb.c libtommath/bn_error.c \
libtommath/bn_mp_init_multi.c libtommath/bn_mp_clear_multi.c libtommath/bn_mp_exteuclid.c libtommath/bn_mp_toradix_n.c \
libtommath/bn_mp_prime_random_ex.c libtommath/bn_mp_get_int.c libtommath/bn_mp_sqrt.c libtommath/bn_mp_is_square.c libtommath/bn_mp_init_set.c \
libtommath/bn_mp_init_set_int.c libtommath/bn_mp_invmod_slow.c libtommath/bn_mp_prime_rabin_miller_trials.c \
libtommath/bn_mp_to_signed_bin_n.c libtommath/bn_mp_to_unsigned_bin_n.c

libtommath_a_CPPFLAGS = -I$(srcdir)/libtommath $(AM_CPPFLAGS)
noinst_LIBRARIES += libtommath.a

libtomcrypt_a_SOURCES = \
libtomcrypt/src/ciphers/aes/aes.c \
libtomcrypt/src/hashes/md5.c libtomcrypt/src/hashes/sha1.c libtomcrypt/src/hashes/helper/hash_memory.c \
libtomcrypt/src/math/fp/ltc_ecc_fp_mulmod.c libtomcrypt/src/math/ltm_desc.c libtomcrypt/src/math/multi.c \
libtomcrypt/src/math/rand_prime.c libtomcrypt/src/misc/base64/base64_decode.c \
libtomcrypt/src/misc/base64/base64_encode.c libtomcrypt/src/misc/burn_stack.c libtomcrypt/src/misc/crypt/crypt.c \
libtomcrypt/src/misc/crypt/crypt_argchk.c libtomcrypt/src/misc/crypt/crypt_cipher_descriptor.c \
libtomcrypt/src/misc/crypt/crypt_cipher_is_valid.c libtomcrypt/src/misc/crypt/crypt_find_cipher.c \
libtomcrypt/src/misc/crypt/crypt_find_cipher_any.c libtomcrypt/src/misc/crypt/crypt_find_cipher_id.c \
libtomcrypt/src/misc/crypt/crypt_find_hash.c libtomcrypt/src/misc/crypt/crypt_find_hash_any.c \
libtomcrypt/src/misc/crypt/crypt_find_hash_id.c libtomcrypt/src/misc/crypt/crypt_find_hash_oid.c \
libtomcrypt/src/misc/crypt/crypt_find_prng.c libtomcrypt/src/misc/crypt/crypt_fsa.c libtomcrypt/src/misc/crypt/crypt_hash_descriptor.c \
libtomcrypt/src/misc/crypt/crypt_hash_is_valid.c libtomcrypt/src/misc/crypt/crypt_ltc_mp_descriptor.c \
libtomcrypt/src/misc/crypt/crypt_prng_descriptor.c libtomcrypt/src/misc/crypt/crypt_prng_is_valid.c \
libtomcrypt/src/misc/crypt/crypt_register_cipher.c libtomcrypt/src/misc/crypt/crypt_register_hash.c \
libtomcrypt/src/misc/crypt/crypt_register_prng.c libtomcrypt/src/misc/crypt/crypt_unregister_cipher.c \
libtomcrypt/src/misc/crypt/crypt_unregister_hash.c libtomcrypt/src/misc/crypt/crypt_unregister_prng.c \
libtomcrypt/src/misc/error_to_string.c libtomcrypt/src/misc/pkcs5/pkcs_5_1.c libtomcrypt/src/misc/pkcs5/pkcs_5_2.c libtomcrypt/src/misc/zeromem.c \
libtomcrypt/src/modes/cbc/cbc_decrypt.c libtomcrypt/src/modes/cbc/cbc_done.c libtomcrypt/src/modes/cbc/cbc_encrypt.c \
libtomcrypt/src/modes/cbc/cbc_getiv.c libtomcrypt/src/modes/cbc/cbc_setiv.c libtomcrypt/src/modes/cbc/cbc_start.c \
libtomcrypt/src/modes/cfb/cfb_decrypt.c libtomcrypt/src/modes/cfb/cfb_done.c libtomcrypt/src/modes/cfb/cfb_encrypt.c \
libtomcrypt/src/modes/cfb/cfb_getiv.c libtomcrypt/src/modes/cfb/cfb_setiv.c libtomcrypt/src/modes/cfb/cfb_start.c \
libtomcrypt/src/modes/ctr/ctr_decrypt.c libtomcrypt/src/modes/ctr/ctr_done.c libtomcrypt/src/modes/ctr/ctr_encrypt.c \
libtomcrypt/src/modes/ctr/ctr_getiv.c libtomcrypt/src/modes/ctr/ctr_setiv.c libtomcrypt/src/modes/ctr/ctr_start.c libtomcrypt/src/modes/ctr/ctr_test.c \
libtomcrypt/src/modes/ecb/ecb_decrypt.c libtomcrypt/src/modes/ecb/ecb_done.c libtomcrypt/src/modes/ecb/ecb_encrypt.c \
libtomcrypt/src/modes/ecb/ecb_start.c \
libtomcrypt/src/modes/ofb/ofb_decrypt.c libtomcrypt/src/modes/ofb/ofb_done.c \
libtomcrypt/src/pk/asn1/der/bit/der_decode_bit_string.c \
libtomcrypt/src/pk/asn1/der/bit/der_encode_bit_string.c libtomcrypt/src/pk/asn1/der/bit/der_length_bit_string.c \
libtomcrypt/src/pk/asn1/der/boolean/der_decode_boolean.c libtomcrypt/src/pk/asn1/der/boolean/der_encode_boolean.c \
libtomcrypt/src/pk/asn1/der/boolean/der_length_boolean.c libtomcrypt/src/pk/asn1/der/choice/der_decode_choice.c \
libtomcrypt/src/pk/asn1/der/ia5/der_decode_ia5_string.c libtomcrypt/src/pk/asn1/der/ia5/der_encode_ia5_string.c \
libtomcrypt/src/pk/asn1/der/ia5/der_length_ia5_string.c libtomcrypt/src/pk/asn1/der/integer/der_decode_integer.c \
libtomcrypt/src/pk/asn1/der/integer/der_encode_integer.c libtomcrypt/src/pk/asn1/der/integer/der_length_integer.c \
libtomcrypt/src/pk/asn1/der/object_identifier/der_decode_object_identifier.c \
libtomcrypt/src/pk/asn1/der/object_identifier/der_encode_object_identifier.c \
libtomcrypt/src/pk/asn1/der/object_identifier/der_length_object_identifier.c \
libtomcrypt/src/pk/asn1/der/octet/der_decode_octet_string.c libtomcrypt/src/pk/asn1/der/octet/der_encode_octet_string.c \
libtomcrypt/src/pk/asn1/der/octet/der_length_octet_string.c \
libtomcrypt/src/pk/asn1/der/printable_string/der_decode_printable_string.c \
libtomcrypt/src/pk/asn1/der/printable_string/der_encode_printable_string.c \
libtomcrypt/src/pk/asn1/der/printable_string/der_length_printable_string.c \
libtomcrypt/src/pk/asn1/der/sequence/der_decode_sequence_ex.c \
libtomcrypt/src/pk/asn1/der/sequence/der_decode_sequence_flexi.c \
libtomcrypt/src/pk/asn1/der/sequence/der_decode_sequence_multi.c \
libtomcrypt/src/pk/asn1/der/sequence/der_encode_sequence_ex.c \
libtomcrypt/src/pk/asn1/der/sequence/der_encode_sequence_multi.c libtomcrypt/src/pk/asn1/der/sequence/der_length_sequence.c \
libtomcrypt/src/pk/asn1/der/sequence/der_sequence_free.c libtomcrypt/src/pk/asn1/der/set/der_encode_set.c \
libtomcrypt/src/pk/asn1/der/set/der_encode_setof.c libtomcrypt/src/pk/asn1/der/short_integer/der_decode_short_integer.c \
libtomcrypt/src/pk/asn1/der/short_integer/der_encode_short_integer.c \
libtomcrypt/src/pk/asn1/der/short_integer/der_length_short_integer.c libtomcrypt/src/pk/asn1/der/utctime/der_decode_utctime.c \
libtomcrypt/src/pk/asn1/der/utctime/der_encode_utctime.c libtomcrypt/src/pk/asn1/der/utctime/der_length_utctime.c \
libtomcrypt/src/pk/asn1/der/utf8/der_decode_utf8_string.c libtomcrypt/src/pk/asn1/der/utf8/der_encode_utf8_string.c \
libtomcrypt/src/pk/asn1/der/utf8/der_length_utf8_string.c libtomcrypt/src/pk/dsa/dsa_decrypt_key.c \
libtomcrypt/src/pk/dsa/dsa_encrypt_key.c libtomcrypt/src/pk/dsa/dsa_export.c libtomcrypt/src/pk/dsa/dsa_free.c libtomcrypt/src/pk/dsa/dsa_import.c \
libtomcrypt/src/pk/dsa/dsa_make_key.c libtomcrypt/src/pk/dsa/dsa_shared_secret.c libtomcrypt/src/pk/dsa/dsa_sign_hash.c \
libtomcrypt/src/pk/dsa/dsa_verify_hash.c libtomcrypt/src/pk/dsa/dsa_verify_key.c \
libtomcrypt/src/pk/pkcs1/pkcs_1_i2osp.c libtomcrypt/src/pk/pkcs1/pkcs_1_mgf1.c \
libtomcrypt/src/pk/pkcs1/pkcs_1_oaep_decode.c libtomcrypt/src/pk/pkcs1/pkcs_1_oaep_encode.c libtomcrypt/src/pk/pkcs1/pkcs_1_os2ip.c \
libtomcrypt/src/pk/pkcs1/pkcs_1_pss_decode.c libtomcrypt/src/pk/pkcs1/pkcs_1_pss_encode.c libtomcrypt/src/pk/pkcs1/pkcs_1_v1_5_decode.c \
libtomcrypt/src/pk/pkcs1/pkcs_1_v1_5_encode.c libtomcrypt/src/pk/rsa/rsa_decrypt_key.c libtomcrypt/src/pk/rsa/rsa_encrypt_key.c \
libtomcrypt/src/pk/rsa/rsa_export.c libtomcrypt/src/pk/rsa/rsa_exptmod.c libtomcrypt/src/pk/rsa/rsa_free.c libtomcrypt/src/pk/rsa/rsa_import.c \
libtomcrypt/src/pk/rsa/rsa_make_key.c libtomcrypt/src/pk/rsa/rsa_sign_hash.c libtomcrypt/src/pk/rsa/rsa_verify_hash.c libtomcrypt/src/prngs/fortuna.c \
libtomcrypt/src/prngs/rng_get_bytes.c libtomcrypt/src/prngs/rng_make_prng.c \
libtomcrypt/src/prngs/sprng.c libtomcrypt/src/prngs/yarrow.c 

libtomcrypt_a_CPPFLAGS = -I$(srcdir)/libtomcrypt/src/headers $(AM_CPPFLAGS)

noinst_LIBRARIES += libtomcrypt.a

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
		  $(XLIBS)

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
		$(Lua) \
		$(ArchHooks) \
		$(Threads) \
		$(Dialog) \
		tests/test_misc.cpp tests/test_misc.h \
		IniFile.cpp IniFile.h \
		XmlFile.cpp XmlFile.h \
		Command.cpp Command.h \
		DateTime.cpp DateTime.h \
		EnumHelper.cpp EnumHelper.h \
		PrefsManager.cpp PrefsManager.h \
		Preference.cpp Preference.h \
		LocalizedString.cpp LocalizedString.h \
		LuaBinding.cpp LuaBinding.h \
		LuaFunctions.h \
		LuaManager.cpp LuaManager.h \
		LuaReference.cpp LuaReference.h \
		SpecialFiles.cpp SpecialFiles.h \
		RageException.cpp \
		RageLog.cpp \
		RageThreads.cpp \
		RageTimer.cpp RageTimer.h \
		RageUtil.cpp \
		RageUtil_FileDB.cpp RageUtil_FileDB.h \
		global.cpp global.h \
		RageUtil_WorkerThread.cpp RageUtil_WorkerThread.h \
		MessageManager.cpp MessageManager.h

TESTS += test_audio_readers
test_audio_readers_SOURCES = \
		$(RageSoundFileReaders) \
		$(all_test_SOURCES) \
		tests/test_audio_readers.cpp \
		RageSoundUtil.cpp RageSoundUtil.h RageSoundMixBuffer.cpp RageSoundMixBuffer.h \
		RageSoundReader.cpp RageSoundReader.h \
		RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
		RageSoundReader_Pan.cpp RageSoundReader_Pan.h \
		RageSoundReader_Preload.cpp RageSoundReader_Preload.h \
		RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
		RageSoundReader_Chain.cpp RageSoundReader_Chain.h

test_audio_readers_LDADD = $(AUDIO_LIBS)

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

INCLUDES = 
