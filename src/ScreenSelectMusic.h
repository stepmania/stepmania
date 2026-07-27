/* ScreenSelectMusic - Choose a Song and Steps. */

#ifndef SCREEN_SELECT_MUSIC_H
#define SCREEN_SELECT_MUSIC_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "FadingBanner.h"
#include "RageUtil_BackgroundLoader.h"
#include "ThemeMetric.h"
#include "RageTexturePreloader.h"
#include "TimingData.h"
#include "GameInput.h"
#include "OptionsList.h"

#include <vector>


enum SelectionState
{
	SelectionState_SelectingSong,
	SelectionState_SelectingSteps,
	SelectionState_Finalized,
	NUM_SelectionState,
};
const RString& SelectionStateToString( SelectionState ss );

class ScreenSelectMusic : public ScreenWithMenuElements
{
public:
	virtual ~ScreenSelectMusic();
	virtual void Init();
	virtual void BeginScreen();

	virtual void Update( float fDeltaTime );
	virtual bool Input( const InputEventPlus &input );
	virtual void HandleMessage( const Message &msg );
	virtual void HandleScreenMessage( const ScreenMessage SM );
	virtual bool AllowLateJoin() const { return true; }

	virtual bool MenuStart( const InputEventPlus &input );
	virtual bool MenuBack( const InputEventPlus &input );

	// ScreenWithMenuElements override: never play music here; we do it ourself.
	virtual void StartPlayingMusic() { }

	bool GetGoToOptions() const { return m_bGoToOptions; }
	MusicWheel *GetMusicWheel() { return &m_MusicWheel; }

	void OpenOptionsList( PlayerNumber pn );
	void OnConfirmSongDeletion();

	bool can_open_options_list(PlayerNumber pn);

	// Lua
	virtual void PushSelf( lua_State *L );

protected:
	virtual bool GenericTweenOn() const { return true; }
	virtual bool GenericTweenOff() const { return true; }
	void UpdateSelectButton( PlayerNumber pn, bool bBeingPressed );

	void ChangeSteps( PlayerNumber pn, int dir );
	void AfterStepsOrTrailChange( const std::vector<PlayerNumber> &vpns );
	void SwitchToPreferredDifficulty();
	void AfterMusicChange();

	void CheckBackgroundRequests( bool bForce );
	bool DetectCodes( const InputEventPlus &input );

	std::vector<Steps*>		m_vpSteps;
	std::vector<Trail*>		m_vpTrails;
	int					m_iSelection[NUM_PLAYERS];

	RageTimer		m_timerIdleComment;
	ThemeMetric<float> IDLE_COMMENT_SECONDS;

	ThemeMetric<float>		SAMPLE_MUSIC_DELAY_INIT;
	ThemeMetric<float>		SAMPLE_MUSIC_DELAY;
	ThemeMetric<bool>		SAMPLE_MUSIC_LOOPS;
	ThemeMetric<SampleMusicPreviewMode> SAMPLE_MUSIC_PREVIEW_MODE;
	ThemeMetric<float>		SAMPLE_MUSIC_FALLBACK_FADE_IN_SECONDS;
	ThemeMetric<float>		SAMPLE_MUSIC_FADE_OUT_SECONDS;
	ThemeMetric<bool>		DO_ROULETTE_ON_MENU_TIMER;
	ThemeMetric<float>		ROULETTE_TIMER_SECONDS;
	ThemeMetric<bool>		ALIGN_MUSIC_BEATS;
	ThemeMetric<RString>	CODES;
	ThemeMetric<RString>	MUSIC_WHEEL_TYPE;
	ThemeMetric<bool>		OPTIONS_MENU_AVAILABLE;
	ThemeMetric<bool>		SELECT_MENU_AVAILABLE;
	ThemeMetric<bool>		MODE_MENU_AVAILABLE;
	ThemeMetric<bool>		USE_OPTIONS_LIST;
	ThemeMetric<float>		OPTIONS_LIST_TIMEOUT;
	ThemeMetric<bool>		USE_PLAYER_SELECT_MENU;
	ThemeMetric<RString>	SELECT_MENU_NAME;
	ThemeMetric<bool>		SELECT_MENU_CHANGES_DIFFICULTY;
	ThemeMetric<bool>		TWO_PART_SELECTION;
	ThemeMetric<bool>		TWO_PART_CONFIRMS_ONLY;
	ThemeMetric<float>		TWO_PART_TIMER_SECONDS;
	ThemeMetric<bool>		WRAP_CHANGE_STEPS;
	ThemeMetric<bool>		CHANGE_STEPS_WITH_GAME_BUTTONS;
	ThemeMetric<bool>		CHANGE_GROUPS_WITH_GAME_BUTTONS;
	ThemeMetric<RString>	NULL_SCORE_STRING;
	ThemeMetric<bool>		PLAY_SOUND_ON_ENTERING_OPTIONS_MENU;

	bool CanChangeSong() const { return m_SelectionState == SelectionState_SelectingSong; }
	bool CanChangeSteps() const { return TWO_PART_SELECTION ? m_SelectionState == SelectionState_SelectingSteps : m_SelectionState == SelectionState_SelectingSong; }
	SelectionState GetNextSelectionState() const
	{
		switch( m_SelectionState )
		{
		case SelectionState_SelectingSong:
			return TWO_PART_SELECTION ? SelectionState_SelectingSteps : SelectionState_Finalized;
		case SelectionState_SelectingSteps:
			return SelectionState_Finalized;
		DEFAULT_FAIL( m_SelectionState );
		}
	}

	GameButton m_GameButtonPreviousSong;
	GameButton m_GameButtonNextSong;
	GameButton m_GameButtonPreviousDifficulty;
	GameButton m_GameButtonNextDifficulty;
	GameButton m_GameButtonPreviousGroup;
	GameButton m_GameButtonNextGroup;
	GameButton m_GameButtonCancelTwoPart1;
	GameButton m_GameButtonCancelTwoPart2;

	RString m_sSectionMusicPath;
	RString m_sSortMusicPath;
	RString m_sRouletteMusicPath;
	RString m_sRandomMusicPath;
	RString m_sCourseMusicPath;
	RString m_sLoopMusicPath;
	RString m_sFallbackCDTitlePath;

	FadingBanner	m_Banner;
	Sprite			m_sprCDTitleFront, m_sprCDTitleBack;
	AutoActor		m_sprHighScoreFrame[NUM_PLAYERS];
	BitmapText		m_textHighScore[NUM_PLAYERS];
	MusicWheel		m_MusicWheel;
	OptionsList		m_OptionsList[NUM_PLAYERS];

	SelectionState	m_SelectionState;
	bool			m_bStepsChosen[NUM_PLAYERS];	// only used in SelectionState_SelectingSteps
	bool			m_bGoToOptions;
	RString			m_sSampleMusicToPlay;
	TimingData		*m_pSampleMusicTimingData;
	float			m_fSampleStartSeconds, m_fSampleLengthSeconds;
	bool			m_bAllowOptionsMenu, m_bAllowOptionsMenuRepeat;
	bool			m_bSelectIsDown[NUM_PLAYERS];
	bool			m_bAcceptSelectRelease[NUM_PLAYERS];

	RageSound		m_soundStart;
	RageSound		m_soundDifficultyEasier;
	RageSound		m_soundDifficultyHarder;
	RageSound		m_soundOptionsChange;
	RageSound		m_soundLocked;

	BackgroundLoader	m_BackgroundLoader;
	RageTexturePreloader	m_TexturePreload;

	Song* m_pSongAwaitingDeletionConfirmation;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
