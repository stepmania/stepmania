/********************************
ScreenEz2SelectPlayer.h
Desc: The "Player Select Screen" for Ez2dancer
Copyright (c):
Frieza
*********************************/

/* Includes */

#include "ScreenWithMenuElements.h"
#include "Sprite.h"
#include "RageSound.h"

class ScreenEz2SelectPlayer : public ScreenWithMenuElements
{
public:
	ScreenEz2SelectPlayer( CString sName );
	virtual ~ScreenEz2SelectPlayer(); // Destructor
	
	/* Public Function Prototypes */
	virtual void Update( float fDeltaTime );
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	virtual void MenuDown( PlayerNumber pn );
	virtual void MenuStart( PlayerNumber pn );
	virtual void MenuBack( PlayerNumber pn );
private:
	void TweenOffScreen();
	void TweenOnScreen();
	
	Sprite	m_sprJoinMessage[NUM_PLAYERS];
	Sprite	m_sprJoinFrame[NUM_PLAYERS];

	RageSound	m_soundSelect;	
	BGAnimation	m_Background;	
};
