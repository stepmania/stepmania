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
#include "MenuTimer.h"
#include "TransitionBGAnimation.h"
#include "HelpDisplay.h"
#include "BGAnimation.h"


const float MENU_ELEMENTS_TWEEN_TIME	=	0.5f;


class MenuElements : public ActorFrame
{
public:
	MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sClassName, bool bEnableTimer = true, bool bLoadStyleIcon = true );

	void DrawTopLayer();
	void DrawBottomLayer();

	void StartTransitioning( ScreenMessage smSendWhenDone );
	void Back( ScreenMessage smSendWhenDone );
	void Update( float fDeltaTime );
	bool IsTransitioning();

public:	// let owner tinker with these objects

	CString				m_sClassName;

	BGAnimation			m_Background;

	Sprite				m_sprHeader;
	Sprite				m_sprStyleIcon;
	MenuTimer			m_MenuTimer;
	Sprite				m_sprFooter;
	HelpDisplay			m_textHelp;

	TransitionBGAnimation	m_In;
	TransitionBGAnimation	m_Out;
	TransitionBGAnimation	m_Back;

	RageSound m_soundBack;

	bool m_bStartedTransitionIn;
};

#endif
