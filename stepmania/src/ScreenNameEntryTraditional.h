/* ScreenNameEntryTraditional - Enter a name for a new high score. */

#ifndef SCREEN_NAME_ENTRY_TRADITIONAL_H
#define SCREEN_NAME_ENTRY_TRADITIONAL_H

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "GradeDisplay.h"
#include "Banner.h"
#include "HighScore.h"
#include "DifficultyIcon.h"
#include "PercentageDisplay.h"
#include "ActorScroller.h"
#include "ThemeMetric.h"
#include "DifficultyMeter.h"
#include "RageSound.h"


class HighScoreWheelItem : public ActorFrame
{
public:
	void Load( int iRankIndex, const HighScore& hs );
	void LoadBlank( int iRankIndex );
	void ShowFocus();

	BitmapText m_textRank;
	BitmapText m_textName;
	BitmapText m_textScore;
	BitmapText m_textDate;
};

class HighScoreWheel : public ActorScroller
{
public:
	void Load( const HighScoreList& hsl, int iIndexToFocus );
	float Scroll();	// return seconds until done scrolling

	vector<HighScoreWheelItem>	m_Items;
	int m_iIndexToFocus;
};

class ScreenNameEntryTraditional : public ScreenWithMenuElements
{
public:
	ScreenNameEntryTraditional( CString sName );
	virtual void Init();
	virtual ~ScreenNameEntryTraditional();

	void Update( float fDeltaTime );
	void HandleScreenMessage( const ScreenMessage SM );
	void Input( const InputEventPlus &input );

	void MenuStart( const InputEventPlus &input );
	void MenuSelect( const InputEventPlus &input );
	void MenuLeft( const InputEventPlus &input );
	void MenuRight( const InputEventPlus &input );

private:
	bool AnyStillEntering() const;
	void AllFinished();
	void PositionCharsAndCursor( int pn );
	void Finish( PlayerNumber pn );
	void UpdateSelectionText( int pn );
	void ChangeDisplayedFeat();
	void SelectChar( PlayerNumber pn, int c );
	void Backspace( PlayerNumber pn );
	void HandleStart( PlayerNumber pn );

	ThemeMetric<float> ALPHABET_GAP_X;
	ThemeMetric<int> NUM_ALPHABET_DISPLAYED;
	ThemeMetric<int> MAX_RANKING_NAME_LENGTH;
	ThemeMetric<float> FEAT_INTERVAL;
	ThemeMetric<CString> KEYBOARD_LETTERS;

	ActorFrame		m_Keyboard[NUM_PLAYERS];
	Sprite			m_sprCursor[NUM_PLAYERS];
	vector<BitmapText*>	m_textAlphabet[NUM_PLAYERS];
	vector<int>		m_AlphabetLetter[NUM_PLAYERS];
	int				m_SelectedChar[NUM_PLAYERS];
	AutoActor		m_sprOutOfRanking[NUM_PLAYERS];	// shown if didn't make any high scores
	Sprite			m_sprNameFrame[NUM_PLAYERS];
	BitmapText		m_textSelection[NUM_PLAYERS];

	/* Feat display: */
	struct FeatDisplay
	{
		HighScoreWheel		m_Wheel;
		GradeDisplay		m_Grade;
		DifficultyIcon		m_DifficultyIcon;
		DifficultyMeter		m_DifficultyMeter;
		BitmapText			m_textCategory;
		PercentageDisplay	m_textScore;
		Banner				m_sprBanner;
		Sprite				m_sprBannerFrame;
	};

	vector<FeatDisplay>		m_FeatDisplay[NUM_PLAYERS];
	int				m_CurFeat[NUM_PLAYERS];
	
	RageSound		m_soundChange;
	RageSound		m_soundKey;
	RageSound		m_soundInvalid;

	wstring			m_sSelection[NUM_PLAYERS];
	bool			m_bStillEnteringName[NUM_PLAYERS];
	bool			m_bGoToNextScreenWhenCardsRemoved;

	ActorCommands	CHANGE_COMMAND;
};

#endif

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
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
