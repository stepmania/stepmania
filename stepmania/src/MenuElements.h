#pragma once
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
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"
#include "MenuTimer.h"
#include "TransitionFadeWipe.h"
#include "TransitionKeepAlive.h"
#include "TransitionInvisible.h"


const float MENU_ELEMENTS_TWEEN_TIME	=	0.30f;


class MenuElements : public ActorFrame
{
public:
	MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText );

	void DrawTopLayer();
	void DrawBottomLayer();

	void TweenOnScreenFromMenu( ScreenMessage smSendWhenDone );
	void TweenOffScreenToMenu( ScreenMessage smSendWhenDone );

	void TweenOnScreenFromBlack( ScreenMessage smSendWhenDone );
	void TweenOffScreenToBlack( ScreenMessage smSendWhenDone, bool bPlayBackSound );

	bool IsClosing() { return m_Wipe.IsClosing() || m_KeepAlive.IsClosing(); };

protected:
	void TweenTopLayerOnScreen();
	void TweenTopLayerOffScreen();

	void TweenBottomLayerOnScreen();
	void TweenBottomLayerOffScreen();


	// stuff in the top bar
	ActorFrame	m_frameTopBar;
	Sprite		m_sprTopEdge;
	Sprite		m_sprStyleIcon;
	MenuTimer	m_MenuTimer;
	
	// stuff in the bottom bar
	ActorFrame	m_frameBottomBar;
	Sprite		m_sprBottomEdge;

	// stuff in the main frame
	Sprite		m_sprBG;
	BitmapText	m_textHelp;
	BitmapText	m_textCreditInfo[NUM_PLAYERS];

	TransitionFadeWipe	m_Wipe;
	TransitionKeepAlive	m_KeepAlive;
	TransitionInvisible	m_Invisible;

	RageSoundSample m_soundSwoosh;
	RageSoundSample m_soundBack;
};
