/********************************
ScreenEz2SelectPlayer.h
Desc: The "Player Select Screen" for Ez2dancer
Copyright (c):
Andrew Livy
*********************************/

/* Includes */

#include "Screen.h"
#include "Sprite.h"
#include "BitmapText.h"
#include "TransitionFade.h"
#include "Quad.h"
#include "RandomSample.h"
#include "Quad.h"
#include "MenuElements.h"

/* Class Definition */

const int NUM_EZ2_GRAPHICS = 4;

class ScreenEz2SelectPlayer : public Screen
{
public:
	ScreenEz2SelectPlayer(); // Constructor
	virtual ~ScreenEz2SelectPlayer(); // Destructor
	
	/* Public Function Prototypes */
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuDown( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );
	void TweenOffScreen();

private:
	/* Private Function Prototypes */
	void Update( float fDeltaTime );
	
	/* Variable Declarations */

	MenuElements m_Menu;
	Sprite	m_sprOpt[NUM_EZ2_GRAPHICS];
	/* Bitflags for m_iSelectedStyle. */
	enum { EZ2_PLAYER_1 = 0x01, 
		   EZ2_PLAYER_2 = 0x02 };
	int m_iSelectedStyle;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;	
	RandomSample m_soundInvalid;

	// used for the bouncing of the '1p' and '2p' images
	float ez2_bounce; 
};
