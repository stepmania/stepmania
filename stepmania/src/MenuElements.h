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
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"
#include "MenuTimer.h"
#include "TransitionFadeWipe.h"
#include "TransitionKeepAlive.h"
#include "TransitionInvisible.h"
#include "TipDisplay.h"
#include "BGAnimation.h"


const float MENU_ELEMENTS_TWEEN_TIME	=	0.5f;


class MenuElements : public ActorFrame
{
public:
	MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText, bool bShowStyleIcon, bool bTimerEnabled, int iTimerSeconds );
	void SetTimer( int iTimerSeconds );
	void StartTimer();
	void StallTimer();
	void StopTimer();

	void StealthTimer( int iActive );
	void DrawTopLayer();
	void DrawBottomLayer();

	void TweenOnScreenFromMenu( ScreenMessage smSendWhenDone, bool bLeaveKeepAliveOn = false );
	void TweenOffScreenToMenu( ScreenMessage smSendWhenDone );
	void ImmedOnScreenFromMenu( bool bLeaveKeepAliveOn = false );
	void ImmedOffScreenToMenu();

	void TweenOnScreenFromBlack( ScreenMessage smSendWhenDone );
	void TweenOffScreenToBlack( ScreenMessage smSendWhenDone, bool bPlayBackSound );

	bool IsClosing() { return m_Wipe.IsClosing() || m_KeepAlive.IsClosing() || m_Invisible.IsClosing(); };

protected:
	void TweenTopLayerOnScreen(float tm=-1);
	void TweenTopLayerOffScreen(float tm=-1);

	void TweenBottomLayerOnScreen();
	void TweenBottomLayerOffScreen();

public:
	Sprite				m_sprTopEdge;
	Sprite				m_sprStyleIcon;
	MenuTimer			m_MenuTimer;
	Sprite				m_sprBottomEdge;
	BGAnimation	m_Background;
	Quad				m_quadBrightness;	// for darkening the background
	TipDisplay			m_textHelp;

	TransitionFadeWipe	m_Wipe;			// for going back
	TransitionKeepAlive	m_KeepAlive;	// going back and forward
	TransitionInvisible	m_Invisible;	// for going forward to Menu

	RageSound m_soundSwoosh;
	RageSound m_soundBack;
};

#endif
