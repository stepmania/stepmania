bin_PROGRAMS = stepmania

# This is a little hacky; we use our own version of SDL and SDL_sound, and I don't
# want to have to integrate their entire build systems.
SDL-1.2.5/src/.libs/libSDL.a:
	cd SDL-1.2.5 && ./configure \
		--disable-nasm --enable-static --disable-shared --enable-debug
	make -C SDL-1.2.5

SDL_sound-1.0.0/.libs/libSDL_sound.a:
	cd SDL_sound-1.0.0 && ./configure \
		--enable-wav \
		--enable-madlib \
		--enable-ogg \
		--disable-aiff \
		--disable-au \
		--disable-voc \
		--disable-raw \
		--disable-shn \
		--disable-midi \
		--disable-flac \
		--disable-smpeg \
		--disable-mpglib \
		--disable-mikmod \
		--disable-modplug \
		--enable-static --disable-shared
	make -C SDL_sound-1.0.0
	
#--enable-debug

RageHelpers = SDL_dither.cpp SDL_dither.h SDL_rotozoom.cpp SDL_rotozoom.h SDL_utils.cpp SDL_utils.h

Rage = $(RageHelpers) \
RageBitmapTexture.cpp RageBitmapTexture.h RageDisplay.cpp RageDisplay.h RageDisplayInternal.h \
RageException.cpp RageException.h RageInput.cpp RageInputDevice.cpp RageInputDevice.h \
RageLog.cpp RageLog.h RageMath.cpp RageMath.h RageSound.cpp RageSound.h \
RageSoundManager.cpp RageSoundManager.h RageSoundReader.h RageSoundReader_SDL_Sound.cpp RageSoundReader_SDL_Sound.h \
RageTexture.cpp RageTexture.h RageTextureManager.cpp RageTextureManager.h RageThreads.cpp RageThreads.h  \
RageTimer.cpp RageTimer.h RageTypes.h RageUtil.cpp RageUtil.h RageSoundReader_Preload.cpp RageSoundReader_Preload.h

DataStructures = \
CodeDetector.cpp CodeDetector.h Course.cpp Course.h Font.cpp Font.h FontCharAliases.cpp FontCharAliases.h \
FontCharmaps.cpp FontCharmaps.h GameConstantsAndTypes.cpp GameConstantsAndTypes.h GameDef.cpp GameDef.h \
GameInput.cpp GameInput.h GameTypes.h Grade.cpp Grade.h NoteData.cpp NoteData.h NoteDataWithScoring.cpp \
NoteDataWithScoring.h Notes.cpp Notes.h NotesLoader.cpp NotesLoader.h NotesLoaderBMS.cpp NotesLoaderBMS.h \
NotesLoaderDWI.cpp NotesLoaderDWI.h NotesLoaderKSF.cpp NotesLoaderKSF.h NotesLoaderSM.cpp NotesLoaderSM.h \
NotesWriterDWI.cpp NotesWriterDWI.h NotesWriterSM.cpp NotesWriterSM.h NoteTypes.cpp NoteTypes.h Player.cpp Player.h \
PlayerOptions.cpp PlayerOptions.h RandomSample.cpp RandomSample.h Song.cpp song.h SongCacheIndex.cpp SongCacheIndex.h \
SongOptions.cpp SongOptions.h StageStats.cpp StageStats.h StyleDef.cpp StyleDef.h StyleInput.h \
TitleSubstitution.cpp TitleSubstitution.h Inventory.cpp Inventory.h PlayerNumber.cpp PlayerNumber.h \
LyricsLoader.cpp LyricsLoader.h

FileTypes = IniFile.cpp IniFile.h MsdFile.cpp MsdFile.h


StepMania = global.h ScreenDimensions.h StdString.h StepMania.cpp StepMania.h

Transitions = \
Transition.cpp Transition.h TransitionFade.cpp TransitionFade.h \
TransitionOniFade.cpp TransitionOniFade.h \
TransitionBGAnimation.cpp TransitionBGAnimation.h

Actors = \
Actor.cpp Actor.h ActorFrame.cpp ActorFrame.h BitmapText.cpp BitmapText.h Quad.h Sample3dObject.cpp Sample3dObject.h \
Sprite.cpp Sprite.h 

ActorsInMenus = \
Banner.cpp Banner.h BannerWithFrame.cpp BannerWithFrame.h BPMDisplay.cpp BPMDisplay.h CourseContentsList.cpp \
CourseContentsList.h CourseEntryDisplay.cpp CourseEntryDisplay.h CroppedSprite.cpp CroppedSprite.h \
DifficultyIcon.cpp DifficultyIcon.h DifficultyRating.cpp DifficultyRating.h EditMenu.cpp EditMenu.h \
FadingBanner.cpp FadingBanner.h DifficultyMeter.cpp DifficultyMeter.h GradeDisplay.cpp GradeDisplay.h GrooveRadar.cpp \
GrooveRadar.h GroupList.cpp GroupList.h JukeboxMenu.cpp JukeboxMenu.h MenuElements.cpp MenuElements.h \
MenuTimer.cpp MenuTimer.h MusicBannerWheel.cpp MusicBannerWheel.h MusicList.cpp MusicList.h MusicSortDisplay.cpp \
MusicSortDisplay.h MusicWheel.cpp MusicWheel.h MusicWheelItem.cpp MusicWheelItem.h OptionIcon.cpp OptionIcon.h \
OptionIconRow.cpp OptionIconRow.h OptionsCursor.cpp OptionsCursor.h ScrollBar.cpp ScrollBar.h ScrollingList.cpp \
ScrollingList.h SmallGradeDisplay.cpp SmallGradeDisplay.h SnapDisplay.cpp SnapDisplay.h TextBanner.cpp TextBanner.h \
TipDisplay.cpp TipDisplay.h WheelNotifyIcon.cpp WheelNotifyIcon.h

ActorsInGameplay = \
ArrowEffects.cpp ArrowEffects.h Background.cpp Background.h BGAnimation.cpp BGAnimation.h BGAnimationLayer.cpp \
BGAnimationLayer.h Combo.cpp Combo.h GhostArrow.cpp GhostArrow.h \
GhostArrowBright.cpp GhostArrowBright.h GhostArrowRow.cpp GhostArrowRow.h GrayArrow.cpp GrayArrow.h \
GrayArrowRow.cpp GrayArrowRow.h HoldGhostArrow.cpp HoldGhostArrow.h HoldJudgment.cpp HoldJudgment.h \
Judgment.cpp Judgment.h LifeMeter.cpp LifeMeter.h LifeMeterBar.cpp LifeMeterBar.h LifeMeterBattery.cpp \
LifeMeterBattery.h MotionBlurSprite.cpp MotionBlurSprite.h NoteDisplay.cpp NoteDisplay.h NoteField.cpp \
NoteField.h ScoreDisplay.cpp ScoreDisplay.h ScoreDisplayNormal.cpp ScoreDisplayNormal.h ScoreDisplayOni.cpp \
ScoreDisplayOni.h ScoreKeeper.cpp ScoreKeeper.h ScoreKeeperMAX2.cpp ScoreKeeperMAX2.h ScoreDisplayBattle.cpp \
ScoreDisplayBattle.h ActiveItemList.cpp ActiveItemList.h LyricDisplay.cpp LyricDisplay.h \
TimingAssist.cpp TimingAssist.h

Screens = \
Screen.cpp Screen.h ScreenAlbums.cpp ScreenAlbums.h ScreenAppearanceOptions.cpp ScreenAppearanceOptions.h \
ScreenAttract.cpp ScreenAttract.h ScreenCaution.cpp ScreenCaution.h ScreenCompany.h \
ScreenDemonstration.cpp ScreenDemonstration.h ScreenEdit.cpp ScreenEdit.h ScreenEditMenu.cpp \
ScreenEditMenu.h ScreenEvaluation.cpp ScreenEvaluation.h ScreenEz2SelectMusic.cpp ScreenEz2SelectMusic.h \
ScreenEz2SelectPlayer.cpp ScreenEz2SelectPlayer.h ScreenGameOver.cpp ScreenGameOver.h ScreenGameplay.cpp \
ScreenGameplay.h ScreenGameplayOptions.cpp ScreenGameplayOptions.h ScreenGraphicOptions.cpp \
ScreenGraphicOptions.h ScreenHowToPlay.h ScreenInputOptions.cpp ScreenInputOptions.h ScreenInstructions.cpp \
ScreenInstructions.h ScreenIntroMovie.cpp ScreenIntroMovie.h ScreenJukebox.cpp ScreenJukebox.h ScreenJukeboxMenu.cpp \
ScreenJukeboxMenu.h ScreenLogo.cpp ScreenLogo.h ScreenMachineOptions.cpp ScreenMachineOptions.h ScreenManager.cpp \
ScreenManager.h ScreenMapControllers.cpp ScreenMapControllers.h ScreenMemoryCard.h ScreenMessage.h \
ScreenMusicScroll.cpp ScreenMusicScroll.h ScreenNameEntry.cpp ScreenNameEntry.h ScreenOptions.cpp ScreenOptions.h \
ScreenOptionsMenu.cpp ScreenOptionsMenu.h ScreenPlayerOptions.cpp ScreenPlayerOptions.h ScreenPrompt.cpp \
ScreenPrompt.h ScreenRanking.cpp ScreenRanking.h ScreenSandbox.cpp ScreenSandbox.h ScreenSelectCourse.cpp \
ScreenSelect.cpp ScreenSelect.h ScreenSelectCourse.h ScreenSelectDifficulty.cpp \
ScreenSelectDifficulty.h ScreenSelectDifficultyEX.cpp ScreenSelectDifficultyEX.h ScreenSelectGame.cpp \
ScreenSelectGame.h ScreenSelectGroup.cpp ScreenSelectGroup.h ScreenSelectMode.cpp ScreenSelectMode.h \
ScreenSelectMusic.cpp ScreenSelectMusic.h ScreenSelectStyle.cpp ScreenSelectStyle.h ScreenSelectStyle5th.cpp \
ScreenSelectStyle5th.h ScreenSongOptions.cpp ScreenSongOptions.h ScreenStage.cpp ScreenStage.h \
ScreenStyleSplash.cpp ScreenStyleSplash.h ScreenTest.cpp ScreenTest.h ScreenTestFonts.cpp ScreenTestFonts.h \
ScreenTestSound.cpp ScreenTestSound.h ScreenTextEntry.cpp ScreenTextEntry.h ScreenTitleMenu.cpp ScreenTitleMenu.h \
ScreenUnlock.h ScreenWarning.h ScreenMiniMenu.cpp ScreenMiniMenu.h ScreenSoundOptions.cpp ScreenSoundOptions.h

GlobalSingletons = \
AnnouncerManager.cpp AnnouncerManager.h FontManager.cpp FontManager.h GameManager.cpp GameManager.h \
GameState.cpp GameState.h InputFilter.cpp InputFilter.h InputMapper.cpp InputMapper.h InputQueue.cpp InputQueue.h \
NoteSkinManager.cpp NoteSkinManager.h PrefsManager.cpp PrefsManager.h SongManager.cpp SongManager.h \
ThemeManager.cpp ThemeManager.h 

stepmania_SOURCES = $(Rage) $(DataStructures) $(FileTypes) $(StepMania) $(Transitions) \
		    $(Actors) $(ActorsInMenus) $(ActorsInGameplay) $(Screens) $(GlobalSingletons) \
		    arch/arch.cpp \
		    arch/LoadingWindow/LoadingWindow_SDL.cpp \
		    arch/LoadingWindow/LoadingWindow_SDL.h \
		    arch/MovieTexture/MovieTexture.cpp \
		    arch/Sound/RageSoundDriver_Null.cpp

# Note that we still end up linking dynamically to the shared system library
# for SDL_image.  This is annoying, but it's not a major problem.  The
# "fix" would be to statically link SDL_image, but that would mean
# dealing with its build and libjpeg, png, etc--not worth it.
stepmania_LDADD = \
		  $(GL_LIBS) \
		  SDL_sound-1.0.0/.libs/libSDL_sound.a \
		  SDL-1.2.5/src/.libs/libSDL.a \
		  $(SDL_LIBS)

INCLUDES = $(SDL_CFLAGS) -O0 -g

