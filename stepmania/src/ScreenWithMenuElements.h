#ifndef ScreenWithMenuElements_H
#define ScreenWithMenuElements_H
/*
-----------------------------------------------------------------------------
 Class: ScreenWithMenuElements

 Desc: Load one of several screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "BGAnimation.h"
#include "ActorUtil.h"
#include "RageSound.h"
#include "MemoryCardDisplay.h"

class MenuTimer;
class HelpDisplay;

class ScreenWithMenuElements : public Screen
{
public:
	ScreenWithMenuElements( CString sName );
	virtual ~ScreenWithMenuElements();

	void StartTransitioning( ScreenMessage smSendWhenDone );
	void Back( ScreenMessage smSendWhenDone );
	bool IsTransitioning();
	bool m_bTimerEnabled;

	void StopTimer();
	void ResetTimer();

protected:
	BGAnimation			m_Background;

	AutoActor			m_autoHeader;
	Sprite				m_sprStyleIcon;
	MemoryCardDisplay	m_MemoryCardDisplay[NUM_PLAYERS];
	MenuTimer			*m_MenuTimer;
	AutoActor			m_autoFooter;
	HelpDisplay			*m_textHelp;

	Transition	m_In;
	Transition	m_Out;
	Transition	m_Back;
};

#endif

