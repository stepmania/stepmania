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
#include "TipDisplay.h"


const float MENU_ELEMENTS_TWEEN_TIME	=	0.5f;


class MenuElements : public ActorFrame
{
public:
	MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText, bool bTimerEnabled, int iTimerSeconds );
	void StallTimer();
	void SetTimer( int iTimerSeconds );
	void StopTimer();

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


	Sprite		m_sprTopEdge;
	Sprite		m_sprStyleIcon;
	MenuTimer	m_MenuTimer;
	Sprite		m_sprBottomEdge;
	Sprite		m_sprBG;
	TipDisplay	m_textHelp;

	TransitionFadeWipe	m_Wipe;			// for going back
	TransitionKeepAlive	m_KeepAlive;	// going back and forward
	TransitionInvisible	m_Invisible;	// for going forward to Menu

	RageSoundSample m_soundSwoosh;
	RageSoundSample m_soundBack;
};
