/********************************
ScreenEz2SelectStyle.h
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
#include "MenuBGAnims.h"

/* Class Definition */


class ScreenEz2SelectStyle : public Screen
{
public:
	ScreenEz2SelectStyle(); // Constructor
	virtual ~ScreenEz2SelectStyle(); // Destructor
	
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
	void TweenOffScreen();
	void TweenOnScreen();
	
	Sprite	m_sprCursors[NUM_PLAYERS];
	Sprite	m_sprControllers[NUM_PLAYERS];

	CArray<Style,Style> m_aPossibleStyles;

	MenuBGAnims m_BGAnim[10];

	ScrollingList m_ScrollingList;
	void RefreshStylesAndList();
	void RefreshStylesAndListFromMetrics();


	Sprite	m_sprBackgrounds[NUM_STYLES];

	MenuElements m_Menu;

	RageSoundSample m_soundSelect;	
	RageSoundSample m_soundChange;	
};
