/*
-----------------------------------------------------------------------------
 Class: ScreenNameEntryTraditional

 Desc: Enter you name for a new high score.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "BGAnimation.h"
#include "MenuElements.h"
#include "GradeDisplay.h"
#include "Banner.h"

class ScreenNameEntryTraditional : public Screen
{
public:
	ScreenNameEntryTraditional( CString sName );
	~ScreenNameEntryTraditional();

	void Update( float fDeltaTime );
	void DrawPrimitives();
	void HandleScreenMessage( const ScreenMessage SM );

	void MenuStart( PlayerNumber pn, const InputEventType type );
	void MenuLeft( PlayerNumber pn, const InputEventType type );
	void MenuRight( PlayerNumber pn, const InputEventType type );

	enum { MAX_DISPLAYED_FEATS=16 };
private:
	bool AnyStillEntering() const;
	void PositionCharsAndCursor( int pn );
	void Finish( PlayerNumber pn );
	void UpdateSelectionText( int pn );
	void ChangeDisplayedFeat();
	void SelectChar( PlayerNumber pn, int c );

	int				m_NumFeats[NUM_PLAYERS], m_CurFeat[NUM_PLAYERS];

	BGAnimation		m_Background;
	MenuElements	m_Menu;

	ActorFrame		m_Keyboard[NUM_PLAYERS];
	Sprite			m_sprCursor[NUM_PLAYERS];
	vector<BitmapText*>	m_textAlphabet[NUM_PLAYERS];
	vector<int>		m_AlphabetLetter[NUM_PLAYERS];
	int				m_SelectedChar[NUM_PLAYERS];

	/* Feat display: */
	GradeDisplay	m_Grade[NUM_PLAYERS][MAX_DISPLAYED_FEATS];
	BitmapText		m_textCategory[NUM_PLAYERS][MAX_DISPLAYED_FEATS];
	BitmapText		m_textScore[NUM_PLAYERS][MAX_DISPLAYED_FEATS];
	Banner			m_sprBanner[NUM_PLAYERS][MAX_DISPLAYED_FEATS];
	Sprite			m_sprBannerFrame[NUM_PLAYERS];
	
	Sprite			m_sprNameFrame[NUM_PLAYERS];


	RageSound		m_soundChange;
	RageSound		m_soundKey;

	BitmapText		m_textSelection[NUM_PLAYERS];
	wstring			m_sSelection[NUM_PLAYERS];
	bool			m_bStillEnteringName[NUM_PLAYERS];
	bool			m_bGoToNextScreenWhenCardsRemoved;
};



