/********************************
ScreenEz2SelectStyle.h
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

const int NUM_EZ2STYLE_GRAPHICS = 4;

class ScreenEz2SelectStyle : public Screen
{
public:
	ScreenEz2SelectStyle(); // Constructor
	virtual ~ScreenEz2SelectStyle(); // Destructor
	
	/* Public Function Prototypes */
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( const PlayerNumber p );
	void MenuRight( const PlayerNumber p );
	void MenuStart( const PlayerNumber p );
	void MenuBack( const PlayerNumber p );
	void TweenOffScreen();
//	void TweenOnScreen();

private:
	/* Private Function Prototypes */

	//	void BeforeChange();
	void AfterChange();
	void SetFadedStyles();
//	void AnimateGraphics();
	/* Variable Declarations */

	MenuElements m_Menu;
	Sprite	m_sprOpt[NUM_EZ2STYLE_GRAPHICS];
	int m_iSelectedStyle;

	RandomSample m_soundChange;
	RandomSample m_soundSelect;	
};
