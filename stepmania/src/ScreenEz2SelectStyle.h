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

/* Class Definition */

 const int NUM_EZ2STYLE_GRAPHICS = 4;
// const int NUM_EZ2P_GRAPHICS = 4;

class ScreenEz2SelectStyle : public Screen
{
public:
	ScreenEz2SelectStyle(); // Constructor
	virtual ~ScreenEz2SelectStyle(); // Destructor
	
	/* Public Function Prototypes */
	virtual void DrawPrimitives();
	virtual void Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI );
	virtual void HandleScreenMessage( const ScreenMessage SM );

	void MenuLeft( PlayerNumber p );
	void MenuRight( PlayerNumber p );
	void MenuStart( PlayerNumber p );
	void MenuBack( PlayerNumber p );
	void MenuDown( PlayerNumber p );
	void TweenOffScreen();
	void TweenPlyOffScreen();

private:
	/* Private Function Prototypes */
	void AnimateGraphics();
	void AnimateBackground();
	//	void BeforeChange();
	void SetFadedStyles();
//	void AnimateGraphics();
	/* Variable Declarations */

	MenuElements m_Menu;
//	Sprite	m_sprOpt[NUM_EZ2STYLE_GRAPHICS];
//	Sprite  m_sprPly[NUM_EZ2P_GRAPHICS];
	Sprite	m_sprBackground[NUM_EZ2STYLE_GRAPHICS];

	ScrollingList m_ScrList;


	int m_iSelectedStyle;
	int m_iSelectedPlayer;
	RandomSample m_soundChange;
	RandomSample m_soundSelect;	
	RandomSample m_soundInvalid;
protected:
};
