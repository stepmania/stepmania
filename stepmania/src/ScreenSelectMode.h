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
#include "GameConstantsAndTypes.h"
#include "ModeChoice.h"


/* Class Definition */

const int MAX_MODE_CHOICES = 10;


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
	
	Sprite	m_sprJoinMessage[NUM_PLAYERS];
	Sprite	m_sprJoinFrame[NUM_PLAYERS];
	Sprite  m_ChoiceListFrame;
	Sprite  m_ChoiceListHighlight;
	Sprite  m_Guide;

	Sprite m_Infotext[MAX_MODE_CHOICES];

	CArray<ModeChoice*,ModeChoice*> m_apPossibleModeChoices;

	ScrollingList m_ScrollingList;
	void RefreshModeChoices();

	BGAnimation	m_BGAnimations[MAX_MODE_CHOICES];
	MenuElements m_Menu;

	RageSound	m_soundSelect;	
	RageSound	m_soundChange;	
};
