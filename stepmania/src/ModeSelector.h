#ifndef ModeSelector_H
#define ModeSelector_H
/*
-----------------------------------------------------------------------------
 Class: ModeSelector

 Desc: Abstract class for a widget that selects a ModeChoice

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "Screen.h"
#include "PlayerNumber.h"
struct ModeChoice;

// send SM_BeginFadingOut when all players have confirmed choice


class ModeSelector : public Screen	// instead of ActorFrame so we can use ScreenMessages
{
public:
	ModeSelector() {};
	virtual ~ModeSelector() {};

	virtual void Init( const vector<ModeChoice>& choices, CString sClassName, CString sThemeElementPrefix ) = 0;

	virtual void MenuLeft( PlayerNumber pn ) = 0;
	virtual void MenuRight( PlayerNumber pn ) = 0;
	virtual void MenuUp( PlayerNumber pn ) = 0;
	virtual void MenuDown( PlayerNumber pn ) = 0;
	virtual void MenuStart( PlayerNumber pn ) = 0;
	virtual void MenuBack( PlayerNumber pn ) = 0;
	virtual void TweenOffScreen() = 0;
	virtual void TweenOnScreen() = 0;

	virtual void GetSelectedModeChoice( PlayerNumber pn, ModeChoice* pModeChoiceOut ) = 0;
	bool IsSelectable( const ModeChoice& choice );
	virtual void UpdateSelectableChoices() = 0;

	static ModeSelector* Create( CString sClassName );
};

#endif
