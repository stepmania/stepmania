/*
-----------------------------------------------------------------------------
 Class: ScreenSelectMusic

 Desc: The screen in PLAY_MODE_ARCADE where you choose a Song and Steps.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "Banner.h"
#include "FadingBanner.h"
#include "BPMDisplay.h"
#include "GrooveRadar.h"
#include "GrooveGraph.h"
#include "DifficultyIcon.h"
#include "DifficultyMeter.h"
#include "OptionIconRow.h"
#include "DifficultyDisplay.h"
#include "DifficultyList.h"
#include "CourseContentsList.h"
#include "HelpDisplay.h"
#include "PaneDisplay.h"
#include "Character.h"
#include "BGAnimation.h"

class ScreenSelectMusic : public ScreenWithMenuElements
{
public:
	ScreenSelectMusic( CString sName );
	virtual ~ScreenSelectMusic();

	virtual void DrawPrimitives();

	virtual void Update( float fDeltaTime );
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );

protected:
	void TweenOnScreen();
	void TweenOffScreen();
	void TweenScoreOnAndOffAfterChangeSort();
	void EnterCourseDisplayMode();
	void ExitCourseDisplayMode();
	bool m_bInCourseDisplayMode;
	void TweenSongPartsOnScreen( bool Initial );
	void TweenSongPartsOffScreen( bool Final );
	void TweenCoursePartsOnScreen( bool Initial );
	void TweenCoursePartsOffScreen( bool Final );
	void SkipSongPartTweens();
	void SkipCoursePartTweens();

	void EasierDifficulty( PlayerNumber pn );
	void HarderDifficulty( PlayerNumber pn );

	void AfterNotesChange( PlayerNumber pn );
	void SwitchToPreferredDifficulty();
	void AfterMusicChange();
	void SortOrderChanged();

	void UpdateOptionsDisplays();

	vector<Steps*> m_arrayNotes;
	int					m_iSelection[NUM_PLAYERS];

	Sprite				m_sprCharacterIcon[NUM_PLAYERS];
	Sprite				m_sprBannerMask;
	FadingBanner		m_Banner;
	AutoActor			m_sprBannerFrame;
	BPMDisplay			m_BPMDisplay;
	Sprite				m_sprStage;
	Sprite				m_sprCDTitleFront, m_sprCDTitleBack;
	Sprite				m_sprDifficultyFrame[NUM_PLAYERS];
	DifficultyIcon		m_DifficultyIcon[NUM_PLAYERS];
	Sprite				m_AutoGenIcon[NUM_PLAYERS];
    GrooveRadar			m_GrooveRadar;
    GrooveGraph			m_GrooveGraph;
	BitmapText			m_textSongOptions;
	OptionIconRow		m_OptionIconRow[NUM_PLAYERS];
	Sprite				m_sprMeterFrame[NUM_PLAYERS];
	Sprite				m_sprNonPresence[NUM_PLAYERS];
	DifficultyMeter			m_DifficultyMeter[NUM_PLAYERS];
	MusicSortDisplay	m_MusicSortDisplay;
	Sprite				m_sprHighScoreFrame[NUM_PLAYERS];
	BitmapText			m_textHighScore[NUM_PLAYERS];
	MusicWheel			m_MusicWheel;
	AutoActor			m_MusicWheelUnder;
	Sprite				m_sprBalloon;
	AutoActor			m_sprCourseHasMods;
	DifficultyDisplay   m_DifficultyDisplay;
	DifficultyList		m_DifficultyList;
	CourseContentsList	m_CourseContentsFrame;
	HelpDisplay			m_Artist;
	BitmapText			m_MachineRank;
	PaneDisplay			m_PaneDisplay[NUM_PLAYERS];

	bool				m_bMadeChoice;
	bool				m_bGoToOptions;
	Sprite				m_sprOptionsMessage;
	float				m_fPlaySampleCountdown;
	CString				m_sSampleMusicToPlay, m_sSampleMusicTimingData;
	float				m_fSampleStartSeconds, m_fSampleLengthSeconds;
	bool				m_bAllowOptionsMenu, m_bAllowOptionsMenuRepeat;

	BGAnimation			m_Overlay;
	Transition			m_bgOptionsOut;
	Transition			m_bgNoOptionsOut;

	RageSound			m_soundSelect;
	RageSound			m_soundDifficultyEasier;
	RageSound			m_soundDifficultyHarder;
	RageSound			m_soundOptionsChange;
	RageSound			m_soundLocked;
};


