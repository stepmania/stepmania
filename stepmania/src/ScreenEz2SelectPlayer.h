/********************************
ScreenEz2SelectPlayer.h
Desc: The "Player Select Screen" for Ez2dancer
Copyright (c):
Andrew Livy
*********************************/

/* Includes */

#include "Screen.h"
#include "Sprite.h"
#include "RageSoundSample.h"
#include "MenuElements.h"


class ScreenEz2SelectPlayer : public Screen
{
public:
	ScreenEz2SelectPlayer(); // Constructor
	virtual ~ScreenEz2SelectPlayer(); // Destructor
	
	/* Public Function Prototypes */
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuDown( PlayerNumber p );
	virtual void MenuStart( PlayerNumber p );
	virtual void MenuBack( PlayerNumber p );
private:
	void TweenOffScreen();
	void TweenOnScreen();

	
	Sprite	m_sprCursors[NUM_PLAYERS];
	Sprite	m_sprControllers[NUM_PLAYERS];

	MenuElements m_Menu;

	RageSoundSample m_soundSelect;	
};
