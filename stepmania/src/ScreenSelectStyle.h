/*
-----------------------------------------------------------------------------
 Class: ScreenSelectStyle

 Desc: Select the game mode (single, versus, double).

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
#include "MenuElements.h"


class ScreenSelectStyle : public Screen
{
public:
	ScreenSelectStyle();
	virtual ~ScreenSelectStyle();

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
	
	Sprite	m_sprIcon[NUM_STYLES];
	Sprite	m_sprExplanation;
	Sprite	m_sprPreview;
	Sprite	m_sprInfo;

	// Sprites that are never drawn.  They exist to keep the style textures in memory
	Sprite	m_sprDummyPreview[NUM_STYLES];
	Sprite	m_sprDummyInfo[NUM_STYLES];

	RandomSample m_soundChange;
	RandomSample m_soundSelect;

	int					m_iSelection;
	CArray<Style,Style>	m_aPossibleStyles;

	Style	GetSelectedStyle() { return m_aPossibleStyles[m_iSelection]; };
};


