/*
-----------------------------------------------------------------------------
 File: MenuElements.h

 Desc: Base class for menu Screens.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/
#ifndef _MenuElements_H_
#define _MenuElements_H_



#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "RandomSample.h"
#include "TransitionFade.h"


const float MENU_ELEMENTS_TWEEN_TIME	=	0.30f;


class MenuElements : public ActorFrame
{
public:
	MenuElements();

	virtual void DrawPrimitives();

	void Load( CString sBackgroundPath, CString sTopEdgePath, CString sHelpText );

	void DrawTopLayer();
	void DrawBottomLayer();

	void TweenTopEdgeOnScreen();
	void TweenTopEdgeOffScreen();

	void TweenAllOnScreen();	// tween top edge + background
	void TweenAllOffScreen();

protected:
	void TweenBackgroundOnScreen();
	void TweenBackgroundOffScreen();

	void SetBackgroundOnScreen();
	void SetBackgroundOffScreen();

	void SetTopEdgeOnScreen();
	void SetTopEdgeOffScreen();


	Sprite	m_sprBG;
	Sprite	m_sprTopEdge;
	Sprite	m_sprBottomEdge;
	BitmapText	m_textHelp;

	RageSoundSample m_soundSwoosh;
	RageSoundSample m_soundBack;
};




#endif