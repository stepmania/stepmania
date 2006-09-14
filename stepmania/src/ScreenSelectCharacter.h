/* ScreenSelectCharacter - Deprecated. Replaced by ScreenSelectMaster. */

#ifndef SCREEN_SELECT_CHARACTER_H
#define SCREEN_SELECT_CHARACTER_H

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "RageSound.h"
#include "GameConstantsAndTypes.h"
#include "OptionIcon.h"
#include "Banner.h"


#define MAX_CHAR_ICONS_TO_SHOW 11

class ScreenSelectCharacter : public ScreenWithMenuElements
{
public:
	virtual void Init();
	virtual ~ScreenSelectCharacter();

	virtual void Input( const InputEventPlus &input );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( const InputEventPlus &input );
	void MenuRight( const InputEventPlus &input );
	void MenuUp( const InputEventPlus &input );
	void MenuDown( const InputEventPlus &input );
	void MenuStart( const InputEventPlus &input );
	void MenuBack( const InputEventPlus &input );

	void TweenOffScreen();

private:
	// These functions take the PlayerNumber of the player making the selections,
	// which is not necessarily the same as the PlayerNumber that the options are being
	// chosen for.  If only one player is joined, they will pick their choices, then
	// the CPU's choices.
	void BeforeRowChange( PlayerNumber pn );
	void AfterRowChange( PlayerNumber pn );
	void AfterValueChange( PlayerNumber pn );
	void Move( PlayerNumber pn, int deltaValue );
	bool AllAreFinishedChoosing() const;
	void MakeSelection( PlayerNumber pn );

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

	RageSound		m_soundChange;
};

#endif

/*
 * (c) 2003-2004 Chris Danford
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
