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

private:
	bool AnyStillEntering() const;
	void PositionCharsAndCursor( int pn );
	void Finish( PlayerNumber pn );
	void UpdateSelectionText( int pn );

	BGAnimation		m_Background;
	MenuElements	m_Menu;
// PercentDisplay m_Percent;
	GradeDisplay	m_Grade[NUM_PLAYERS];

	ActorFrame		m_Keyboard[NUM_PLAYERS];
	Sprite			m_sprCursor[NUM_PLAYERS];
	vector<BitmapText*>	m_textAlphabet[NUM_PLAYERS];
	vector<int>		m_AlphabetLetter[NUM_PLAYERS];
	int				m_SelectedChar[NUM_PLAYERS];

	Sprite			m_sprNameFrame[NUM_PLAYERS];

	BitmapText		m_textCategory[NUM_PLAYERS];

	RageSound		m_soundKey;

	BitmapText		m_textSelection[NUM_PLAYERS];
	wstring			m_sSelection[NUM_PLAYERS];
	bool			m_bStillEnteringName[NUM_PLAYERS];
};



