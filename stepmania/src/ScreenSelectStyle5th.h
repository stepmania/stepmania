/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle5th

 Desc: DDR 5th Mix-style screen selection.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"
#include "Quad.h"
#include "TransitionKeepAlive.h"
#include "MenuElements.h"

// 
// TODO: Remove dance hard coded portions
//

const int NUM_STYLE_DANCERS		=	7;	// single, versus dancers, double, couple dancers, solo
const int NUM_STYLE_PADS		=	5;


class ScreenSelectStyle5th : public Screen
{
public:
	ScreenSelectStyle5th();
	virtual ~ScreenSelectStyle5th();

	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );
	void TweenOffScreen();
	void TweenOnScreen();

private:
	void BeforeChange();
	void AfterChange();

	MenuElements m_Menu;
	
	Sprite	m_sprDancer[NUM_STYLE_DANCERS];
	Sprite	m_sprPad[NUM_STYLE_PADS];

	Sprite	m_sprStyleIcon;

	BitmapText	m_textExplanation1;
	BitmapText	m_textExplanation2;
	Quad	m_rectCursor;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;

	int m_iSelection;

};


