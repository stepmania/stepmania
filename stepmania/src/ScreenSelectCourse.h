/*
-----------------------------------------------------------------------------
 Class: ScreenSelectCourse

 Desc: The screen in PLAY_MODE_ONI where you choose a Course.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RageSound.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CourseContentsList.h"
#include "MenuElements.h"
#include "FadingBanner.h"


class ScreenSelectCourse : public Screen
{
public:
	ScreenSelectCourse();
	virtual ~ScreenSelectCourse();

	virtual void DrawPrimitives();

	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void TweenOnScreen();
	void TweenOffScreen();

	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

protected:
	void AdjustOptions();
	void AfterCourseChange();
	void UpdateOptionsDisplays();

	MenuElements		m_Menu;

	Sprite				m_sprExplanation;
	Sprite				m_sprBannerFrame;
	FadingBanner		m_Banner;
	BitmapText			m_textNumSongs;
	BitmapText			m_textTime;
	CourseContentsList	m_CourseContentsFrame;
	Sprite				m_sprHighScoreFrame[NUM_PLAYERS];
	ScoreDisplayNormal	m_HighScore[NUM_PLAYERS];
	BitmapText			m_textPlayerOptions[NUM_PLAYERS];
	BitmapText			m_textSongOptions;
	MusicWheel			m_MusicWheel;

	bool				m_bMadeChoice;
	bool				m_bGoToOptions, m_bAllowOptionsMenuRepeat;
	Sprite				m_sprOptionsMessage;

	RageSound			m_soundSelect;
	RageSound			m_soundOptionsChange;
};


