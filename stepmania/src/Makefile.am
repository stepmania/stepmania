bin_PROGRAMS = stepmania test_audio_readers

AM_LDFLAGS =
AM_CXXFLAGS = 
AM_CFLAGS = 

if NHAVE_SDL
AM_CXXFLAGS += -ISDL-1.2.6/include
endif

if NHAVE_SDL
# This is a little hacky; we statically link SDL if it isn't present on the system already, and I don't
# want to have to integrate the entire build system.

# XXX Both of these should really be linked dynamically.
# XXX honor vpath
SDL-1.2.6/src/.libs/libSDL.a:
	cd SDL-1.2.6 && ./configure \
		--disable-nasm --disable-arts --disable-esd --enable-static --disable-shared \
		--disable-debug --disable-alsa --disable-oss --disable-video-directfb
	make -C SDL-1.2.6
endif

$(srcdir)/libresample/libresample.a:
	cd $(srcdir)/libresample && autoconf && ./configure && make

Screens = \
Screen.cpp Screen.h ScreenAttract.cpp ScreenAttract.h ScreenBookkeeping.h ScreenBookkeeping.cpp \
ScreenCaution.cpp ScreenCaution.h ScreencenterImage.h ScreenCenterImage.cpp ScreenCredits.cpp ScreenCredits.h ScreenDemonstration.cpp ScreenDemonstration.h \
ScreenDimensions.h ScreenEdit.cpp ScreenEdit.h ScreenEditCoursesMenu.cpp ScreenEditCoursesMenu.h \
ScreenEditMenu.cpp ScreenEditMenu.h ScreenEndlessBreak.cpp ScreenEndlessBreak.h ScreenEvaluation.cpp ScreenEvaluation.h \
ScreenExit.cpp ScreenExit.h ScreenEz2SelectMusic.cpp ScreenEz2SelectMusic.h ScreenEz2SelectPlayer.cpp ScreenEz2SelectPlayer.h \
ScreenGameOver.cpp ScreenGameOver.h ScreenGameplay.cpp ScreenGameplay.h \
ScreenHowToPlay.cpp ScreenHowToPlay.h \
ScreenInstructions.cpp ScreenInstructions.h ScreenJukebox.cpp ScreenJukebox.h \
ScreenJukeboxMenu.cpp ScreenJukeboxMenu.h ScreenLogo.cpp ScreenLogo.h \
ScreenMapControllers.cpp ScreenMapControllers.h ScreenMemoryCard.h ScreenMessage.h ScreenMiniMenu.cpp ScreenMiniMenu.h \
ScreenMusicScroll.cpp ScreenMusicScroll.h ScreenNameEntry.cpp ScreenNameEntry.h ScreenNameEntryTraditional.h ScreenNameEntryTraditional.cpp ScreenOptions.cpp ScreenOptions.h \
ScreenOptionsMaster.cpp ScreenOptionsMaster.h ScreenOptionsMasterPrefs.cpp ScreenOptionsMasterPrefs.h \
ScreenPlayerOptions.cpp ScreenPlayerOptions.h ScreenPrompt.cpp ScreenPrompt.h \
ScreenProfileOptions.cpp ScreenProfileOptions.h ScreenRanking.cpp ScreenRanking.h ScreenRaveOptions.cpp ScreenRaveOptions.h \
ScreenReloadSongs.cpp ScreenReloadSongs.h ScreenSandbox.cpp ScreenSandbox.h ScreenSelect.cpp ScreenSelect.h \
ScreenSelectCharacter.cpp ScreenSelectCharacter.h ScreenSelectCourse.cpp ScreenSelectCourse.h \
ScreenSelectDifficulty.cpp ScreenSelectDifficulty.h ScreenSelectDifficultyEX.cpp ScreenSelectDifficultyEX.h \
ScreenSelectGroup.cpp ScreenSelectGroup.h ScreenSelectMaster.cpp ScreenSelectMaster.h \
ScreenSelectMode.cpp ScreenSelectMode.h ScreenSelectMusic.cpp ScreenSelectMusic.h ScreenSelectStyle.cpp ScreenSelectStyle.h \
ScreenSongOptions.cpp ScreenSongOptions.h ScreenStage.cpp ScreenStage.h \
ScreenStyleSplash.cpp ScreenStyleSplash.h ScreenTest.cpp ScreenTest.h ScreenTestFonts.cpp ScreenTestFonts.h \
ScreenTestInput.h ScreenTestInput.cpp ScreenTestSound.cpp ScreenTestSound.h ScreenTextEntry.cpp ScreenTextEntry.h ScreenTitleMenu.cpp ScreenTitleMenu.h \
ScreenUnlock.cpp ScreenUnlock.h


DataStructures = \
BannerCache.cpp BannerCache.h Character.cpp Character.h CharacterHead.cpp CharacterHead.h CodeDetector.cpp CodeDetector.h \
Course.cpp Course.h Font.cpp Font.h FontCharAliases.cpp FontCharAliases.h FontCharmaps.cpp FontCharmaps.h Game.h \
GameConstantsAndTypes.cpp GameConstantsAndTypes.h GameDef.cpp GameDef.h GameInput.cpp GameInput.h \
Grade.cpp Grade.h Inventory.cpp Inventory.h LyricsLoader.cpp LyricsLoader.h MenuInput.h ModeChoice.cpp ModeChoice.h \
NoteData.cpp NoteData.h NoteDataUtil.cpp NoteDataUtil.h NoteDataWithScoring.cpp NoteDataWithScoring.h \
NoteFieldPositioning.cpp NoteFieldPositioning.h NoteTypes.cpp NoteTypes.h NotesLoader.cpp NotesLoader.h \
NotesLoaderBMS.cpp NotesLoaderBMS.h NotesLoaderDWI.cpp NotesLoaderDWI.h NotesLoaderKSF.cpp NotesLoaderKSF.h \
NotesLoaderSM.cpp NotesLoaderSM.h NotesWriterDWI.cpp NotesWriterDWI.h NotesWriterSM.cpp NotesWriterSM.h \
PlayerAI.cpp PlayerAI.h PlayerNumber.cpp PlayerNumber.h PlayerOptions.cpp PlayerOptions.h RandomSample.cpp RandomSample.h \
ScoreKeeper.h ScoreKeeper5th.cpp ScoreKeeper5th.h ScoreKeeperMAX2.cpp ScoreKeeperMAX2.h ScoreKeeperRave.cpp ScoreKeeperRave.h \
Song.cpp song.h SongCacheIndex.cpp SongCacheIndex.h SongOptions.cpp SongOptions.h StageStats.cpp StageStats.h Steps.cpp Steps.h \
Style.h StyleDef.cpp StyleDef.h StyleInput.h TitleSubstitution.cpp TitleSubstitution.h

FileTypes = IniFile.cpp IniFile.h MsdFile.cpp MsdFile.h

StepMania = StdString.h StepMania.cpp StepMania.h global.cpp global.h

LoadingWindow = arch/LoadingWindow/LoadingWindow_Null.h \
				arch/LoadingWindow/LoadingWindow_SDL.cpp arch/LoadingWindow/LoadingWindow_SDL.h

if HAVE_GTK
LoadingWindow += arch/LoadingWindow/LoadingWindow_Gtk.cpp arch/LoadingWindow/LoadingWindow_Gtk.h
endif

Sound = arch/Sound/RageSoundDriver_Null.cpp arch/Sound/RageSoundDriver_Null.h


if HAVE_OSS
Sound += arch/Sound/RageSoundDriver_OSS.cpp arch/Sound/RageSoundDriver_OSS.h
endif

if HAVE_ALSA
Sound += arch/Sound/RageSoundDriver_ALSA9.cpp arch/Sound/RageSoundDriver_ALSA9.h
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

LowLevelWindow = arch/LowLevelWindow/LowLevelWindow.h \
		 arch/LowLevelWindow/LowLevelWindow_SDL.cpp arch/LowLevelWindow/LowLevelWindow_SDL.h

ArchUtils = archutils/Unix/SignalHandler.cpp archutils/Unix/SignalHandler.h

if USE_CRASH_HANDLER
ArchUtils += archutils/Unix/CrashHandler.cpp archutils/Unix/CrashHandler.h \
	     archutils/Unix/CrashHandlerChild.cpp archutils/Unix/CrashHandlerInternal.h
endif

Arch = $(LoadingWindow) $(Sound) $(ArchHooks) $(InputHandler) $(MovieTexture) $(LowLevelWindow) $(ArchUtils) \
       arch/arch.cpp arch/arch.h arch/arch_default.h arch/arch_linux.h

RageSoundFileReaders = \
RageSoundReader_WAV.cpp RageSoundReader_WAV.h 

if HAVE_VORBIS
RageSoundFileReaders += RageSoundReader_Vorbisfile.cpp RageSoundReader_Vorbisfile.h
endif

if HAVE_MP3
RageSoundFileReaders += RageSoundReader_MP3.cpp RageSoundReader_MP3.h
endif

ActorsInGameplayAndMenus = \
BGAnimation.cpp BGAnimation.h BGAnimationLayer.cpp BGAnimationLayer.h Banner.cpp Banner.h \
DifficultyIcon.cpp DifficultyIcon.h MeterDisplay.cpp MeterDisplay.h Transition.cpp Transition.h

ActorsInMenus = \
BPMDisplay.cpp BPMDisplay.h CourseContentsList.cpp CourseContentsList.h CourseEntryDisplay.cpp CourseEntryDisplay.h \
DifficultyDisplay.cpp DifficultyDisplay.h DifficultyMeter.cpp DifficultyMeter.h DifficultyRating.cpp DifficultyRating.h \
EditCoursesMenu.cpp EditCoursesMenu.h EditMenu.cpp EditMenu.h FadingBanner.cpp FadingBanner.h GradeDisplay.cpp GradeDisplay.h \
GrooveGraph.cpp GrooveGraph.h GrooveRadar.cpp GrooveRadar.h GroupList.cpp GroupList.h HelpDisplay.cpp HelpDisplay.h \
JukeboxMenu.cpp JukeboxMenu.h MenuElements.cpp MenuElements.h MenuTimer.cpp MenuTimer.h ModeSwitcher.cpp ModeSwitcher.h \
MusicBannerWheel.cpp MusicBannerWheel.h MusicList.cpp MusicList.h MusicSortDisplay.cpp MusicSortDisplay.h \
MusicWheel.cpp MusicWheel.h MusicWheelItem.cpp MusicWheelItem.h OptionIcon.cpp OptionIcon.h \
OptionIconRow.cpp OptionIconRow.h OptionsCursor.cpp OptionsCursor.h ScrollBar.cpp ScrollBar.h \
ScrollingList.cpp ScrollingList.h SnapDisplay.cpp SnapDisplay.h TextBanner.cpp TextBanner.h \
WheelNotifyIcon.cpp WheelNotifyIcon.h

ActorsInGameplay = \
ArrowBackdrop.cpp ArrowBackdrop.h ArrowEffects.cpp ArrowEffects.h Background.cpp Background.h \
BeginnerHelper.cpp BeginnerHelper.h CombinedLifeMeterEnemy.h CombinedLifeMeterEnemy.cpp \
CombinedLifeMeterTug.h CombinedLifeMeterTug.cpp Combo.cpp Combo.h DancingCharacters.cpp DancingCharacters.h \
GhostArrow.cpp GhostArrow.h GhostArrowRow.cpp GhostArrowRow.h GrayArrow.cpp GrayArrow.h \
GrayArrowRow.cpp GrayArrowRow.h HoldGhostArrow.cpp HoldGhostArrow.h HoldJudgment.cpp HoldJudgment.h \
Judgment.cpp Judgment.h LifeMeter.h LifeMeterBar.cpp LifeMeterBar.h LifeMeterBattery.cpp LifeMeterBattery.h \
LyricDisplay.cpp LyricDisplay.h NoteDisplay.cpp NoteDisplay.h NoteField.cpp NoteField.h \
NoteFieldPlus.h NoteFieldPlus.cpp Player.cpp Player.h ProTimingDisplay.cpp ProTimingDisplay.h \
ScoreDisplay.cpp ScoreDisplay.h ScoreDisplayBattle.cpp ScoreDisplayBattle.h ScoreDisplayNormal.cpp ScoreDisplayNormal.h \
ScoreDisplayOni.cpp ScoreDisplayOni.h ScoreDisplayRave.cpp ScoreDisplayRave.h TimingAssist.cpp TimingAssist.h \
PercentageDisplay.cpp PercentageDisplay.h ScoreDisplayPercentage.cpp ScoreDisplayPercentage.h

Helpers = SDL_dither.cpp SDL_dither.h SDL_rotozoom.cpp SDL_rotozoom.h SDL_utils.cpp SDL_utils.h

PCRE = pcre/chartables.c pcre/get.c pcre/internal.h pcre/maketables.c pcre/pcre.c pcre/pcre.h pcre/study.c

Rage = $(Helpers) $(PCRE)\
RageBitmapTexture.cpp RageBitmapTexture.h RageDisplay.cpp RageDisplay.h RageDisplay_OGL.cpp RageDisplay_OGL.h \
RageException.cpp RageException.h RageFile.cpp RageFile.h RageInput.cpp RageInput.h \
RageInputDevice.cpp RageInputDevice.h RageLog.cpp RageLog.h RageMath.cpp RageMath.h RageSound.cpp RageSound.h \
RageSoundManager.cpp RageSoundManager.h RageSoundReader.h RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
RageSoundReader_Preload.cpp RageSoundReader_Preload.h RageSoundReader_Resample.cpp RageSoundReader_Resample.h \
RageSoundReader_Resample_Fast.cpp RageSoundReader_Resample_Fast.h RageSoundReader_Resample_Good.cpp RageSoundReader_Resample_Good.h \
RageSoundResampler.cpp RageSoundResampler.h RageSounds.cpp RageSounds.h RageTexture.cpp RageTexture.h \
RageTextureManager.cpp RageTextureManager.h RageThreads.cpp RageThreads.h RageTimer.cpp RageTimer.h RageTypes.h \
RageUtil.cpp RageUtil.h RageUtil_CharConversions.cpp RageUtil_CharConversions.h RageUtil_FileDB.cpp RageUtil_FileDB.h \
$(RageSoundFileReaders)

Actors = \
Actor.cpp Actor.h ActorCollision.h ActorFrame.cpp ActorFrame.h ActorScroller.cpp ActorScroller.h \
ActorUtil.cpp ActorUtil.h BitmapText.cpp BitmapText.h Milkshape.h Model.cpp Model.h ModelTypes.cpp ModelTypes.h \
Quad.h Sprite.cpp Sprite.h mathlib.c mathlib.h 

GlobalSingletons = \
AnnouncerManager.cpp AnnouncerManager.h Bookkeeper.h Bookkeeper.cpp FontManager.cpp FontManager.h GameManager.cpp GameManager.h \
GameState.cpp GameState.h InputFilter.cpp InputFilter.h InputMapper.cpp InputMapper.h InputQueue.cpp InputQueue.h \
NoteSkinManager.cpp NoteSkinManager.h PrefsManager.cpp PrefsManager.h ProfileManager.cpp ProfileManager.h \
ScreenManager.cpp ScreenManager.h SongManager.cpp SongManager.h ThemeManager.cpp ThemeManager.h \
UnlockSystem.cpp UnlockSystem.h

stepmania_SOURCES = $(Screens) $(DataStructures) $(FileTypes) $(StepMania) $(Arch) \
					$(ActorsInGameplayAndMenus) $(ActorsInMenus) $(ActorsInGameplay) \
					$(Rage) $(Actors) $(GlobalSingletons)

# Note that we still end up linking dynamically to the shared system library
# for SDL_image.  This is annoying, but it's not a major problem.  The
# "fix" would be to statically link SDL_image, but that would mean
# dealing with its build and libjpeg, png, etc--not worth it.

if NHAVE_SDL
USE_SDL_LIBS = $(srcdir)/SDL-1.2.6/src/.libs/libSDL.a
else
USE_SDL_LIBS = $(SDL_LIBS)
endif

stepmania_LDADD = \
		  $(GL_LIBS) \
		  $(USE_SDL_LIBS) -lSDL_image \
		  $(AUDIO_LIBS) \
		  $(srcdir)/libresample/libresample.a

# A hack for now:
test_audio_readers_SOURCES = tests/test_audio_readers.cpp \
		RageUtil.cpp RageLog.cpp RageThreads.cpp RageException.cpp \
		RageUtil_FileDB.cpp RageUtil_FileDB.h \
		RageTimer.cpp RageTimer.h \
		RageFile.cpp RageFile.h \
		RageSoundReader_FileReader.cpp RageSoundReader_FileReader.h \
		PrefsManager.cpp PrefsManager.h \
		arch/ArchHooks/ArchHooks.cpp arch/ArchHooks/ArchHooks.h \
		IniFile.cpp IniFile.h \
		$(RageSoundFileReaders) \
		$(PCRE)

test_audio_readers_LDADD = \
	      $(USE_SDL_LIBS) \
	      $(AUDIO_LIBS)

INCLUDES = $(SDL_CFLAGS)

if HAVE_GTK
bin_PROGRAMS += GtkModule.so
GtkModule_so_LDFLAGS = -rdynamic -shared
GtkModule_so_CPPFLAGS = $(GTK_CFLAGS)
GtkModule_so_LDADD = $(GTK_LIBS)
GtkModule_so_SOURCES = arch/LoadingWindow/LoadingWindow_GtkModule.cpp
endif
