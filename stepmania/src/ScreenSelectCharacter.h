/*
-----------------------------------------------------------------------------
 File: ScreenSelectCharacter.h

 Desc: Set the current song group by selecting from a list.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "RandomSample.h"
#include "GameConstantsAndTypes.h"
#include "OptionIcon.h"
#include "Banner.h"


#define MAX_CHAR_ICONS_TO_SHOW 11

class ScreenSelectCharacter : public ScreenWithMenuElements
{
public:
	ScreenSelectCharacter( CString sName );
	virtual ~ScreenSelectCharacter();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuUp( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );

	void TweenOffScreen();
	void TweenOnScreen();

private:
	// These functions take the PlayerNumber of the player making the selections,
	// which is not necessarily the same as the PlayerNumber that the options are being
	// chosen for.  If only one player is joined, they will pick their choices, then
	// the CPU's choices.
	void BeforeRowChange( PlayerNumber pn );
	void AfterRowChange( PlayerNumber pn );
	void AfterValueChange( PlayerNumber pn );
	void Move( PlayerNumber pn, int deltaValue );

	int	m_iSelectedCharacter[NUM_PLAYERS];
	enum { 
		CHOOSING_HUMAN_CHARACTER, 
		CHOOSING_CPU_CHARACTER, 
		FINISHED_CHOOSING
	} m_SelectionRow[NUM_PLAYERS];
	PlayerNumber GetAffectedPlayerNumber( PlayerNumber pn );	// returns the number of the player that pn is selecting for

	Sprite			m_sprTitle[NUM_PLAYERS];

	Banner			m_sprIcons[NUM_PLAYERS][MAX_CHAR_ICONS_TO_SHOW];

	Sprite			m_sprCard[NUM_PLAYERS];
	Sprite			m_sprCardArrows[NUM_PLAYERS];
	
	Sprite			m_sprAttackFrame[NUM_PLAYERS];
	OptionIcon		m_AttackIcons[NUM_PLAYERS][NUM_ATTACK_LEVELS][NUM_ATTACKS_PER_LEVEL];

	Sprite			m_sprExplanation;

	RandomSample	m_soundChange;
};


