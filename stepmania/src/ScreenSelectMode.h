/********************************
ScreenSelectMode.h
Desc: The "Style Select Screen" for Ez2dancer
Copyright (c):
Andrew Livy
*********************************/

/* Includes */

#include "Background.h"
#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"
#include "Quad.h"
#include "MenuElements.h"
#include "ScrollingList.h"


/* Class Definition */


class ScreenSelectMode : public Screen
{
public:
	ScreenSelectMode(); // Constructor
	virtual ~ScreenSelectMode(); // Destructor
	
	/* Public Function Prototypes */
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( PlayerNumber pn );
	void MenuRight( PlayerNumber pn );
	void MenuStart( PlayerNumber pn );
	void MenuBack( PlayerNumber pn );
	void MenuDown( PlayerNumber pn );

protected:
	void AfterChange();

	void TweenOffScreen();
	void TweenOnScreen();
	
	Sprite	m_sprCursors[NUM_PLAYERS];
	Sprite	m_sprControllers[NUM_PLAYERS];
	Sprite  m_StyleListFrame;
	Sprite  m_SelectedStyleFrame;

	CArray<ModeChoice,ModeChoice> m_aPossibleModeChoices;

	ScrollingList m_ScrollingList;
	void RefreshModeChoices();

	MenuElements m_Menu;

	RageSoundSample m_soundSelect;	
	RageSoundSample m_soundChange;	
};
