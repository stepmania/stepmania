#ifndef MENUELEMENTS_H
#define MENUELEMENTS_H
/*
-----------------------------------------------------------------------------
 File: MenuElements.h

 Desc: Displays common components of menu screens:
	Background, Top Bar, Bottom Bar, help message, credits or PlayerOptions, style icon,
	Menu Timer

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "Transition.h"
#include "BGAnimation.h"
#include "ActorUtil.h"
#include "RageSound.h"

class MenuTimer;
class HelpDisplay;
class MenuElements : public ActorFrame
{
public:
	MenuElements();
	virtual ~MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sClassName );

	void DrawTopLayer();
	void DrawBottomLayer();

	void StartTransitioning( ScreenMessage smSendWhenDone );
	void Back( ScreenMessage smSendWhenDone );
	void Update( float fDeltaTime );
	bool IsTransitioning();
	bool m_bTimerEnabled;

	void StopTimer();

public:	// let owner tinker with these objects
	BGAnimation			m_Background;

	AutoActor			m_autoHeader;
	Sprite				m_sprStyleIcon;
	MenuTimer			*m_MenuTimer;
	AutoActor			m_autoFooter;
	HelpDisplay			*m_textHelp;

	Transition	m_In;
	Transition	m_Out;
	Transition	m_Back;

	RageSound m_soundBack;
};

#endif
