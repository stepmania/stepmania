/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntryTraditional

 Desc: Enter you name for a new high score.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenWithMenuElements.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "BGAnimation.h"
#include "GradeDisplay.h"
#include "Banner.h"
#include "HighScore.h"
#include "DifficultyIcon.h"
#include "PercentageDisplay.h"


class HighScoreWheelItem : public ActorFrame
{
public:
	void Load( int iRankIndex, const HighScore& hs );
	void LoadBlank( int iRankIndex );
	void ShowFocus();

	BitmapText m_textRank;
	BitmapText m_textName;
	BitmapText m_textScore;
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
	~ScreenNameEntryTraditional();

	void Update( float fDeltaTime );
	void DrawPrimitives();
	void HandleScreenMessage( const ScreenMessage SM );
	void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );

	void MenuStart( PlayerNumber pn, const InputEventType type );
	void MenuLeft( PlayerNumber pn, const InputEventType type );
	void MenuRight( PlayerNumber pn, const InputEventType type );

private:
	bool AnyStillEntering() const;
	void AllFinished();
	void PositionCharsAndCursor( int pn );
	void Finish( PlayerNumber pn );
	void UpdateSelectionText( int pn );
	void ChangeDisplayedFeat();
	void SelectChar( PlayerNumber pn, int c );

	int				m_NumFeats[NUM_PLAYERS];
	int				m_CurFeat[NUM_PLAYERS];

	BGAnimation		m_Background;

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
		DifficultyIcon		m_Difficulty;
		BitmapText			m_textCategory;
		PercentageDisplay	m_textScore;
		Banner				m_sprBanner;
		Sprite				m_sprBannerFrame;
	};

	vector<FeatDisplay>		m_FeatDisplay[NUM_PLAYERS];
	
	RageSound		m_soundChange;
	RageSound		m_soundKey;
	RageSound		m_soundInvalid;

	wstring			m_sSelection[NUM_PLAYERS];
	bool			m_bStillEnteringName[NUM_PLAYERS];
	bool			m_bGoToNextScreenWhenCardsRemoved;
};



