/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy
*****************************************/

/* Includes */

#include "stdafx.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSandbox.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "ScreenEz2SelectStyle.h"

/* Constants */

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);

const CString N_PLAYERS[] = {
	"one player",
	"two players",
};

const float TWEEN_TIME		= 0.35f;
const D3DXCOLOR OPT_NOT_SELECTED = D3DXCOLOR(0.3f,0.3f,0.3f,1);
const D3DXCOLOR OPT_SELECTED = D3DXCOLOR(1.0f,1.0f,1.0f,1);

const float OPT_X[NUM_EZ2_GRAPHICS] = { 
	CENTER_X+200, // This is the pad X
	CENTER_X-200, // This is the pad X
	CENTER_X-198, // This is the 1p X
	CENTER_X+195, // This is the 2p X
}; // tells us the default X position
const float OPT_Y[NUM_EZ2_GRAPHICS] = {
	CENTER_Y+130,
	CENTER_Y+130,
	CENTER_Y+115,
	CENTER_Y+115,
}; // tells us the default Y position

/* Variable Declaration */
int bounce=0; // used for the bouncing of the '1p' and '2p' images
int direct=0; // direction of the bouncing of the '1p' and '2p' images
int wait=0; // how long we wait between updating the bounce.


/************************************
ScreenEz2SelectPlayer (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()
{
	LOG->WriteLine( "ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()" );

m_iSelectedStyle=0;

// Load in the sprites we will be working with.
	for( int i=0; i<NUM_EZ2_GRAPHICS; i++ )
	{
		CString sPadGraphicPath;
		switch( i )
		{
		case 0:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE);	
			break;
		case 1:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_HARD_PICTURE);	
			break;
		case 2:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_MEDIUM_PICTURE);	
			break;
		case 3:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_DIFFICULTY_EASY_PICTURE);	
			break;
		}
		m_sprOpt[i].Load( sPadGraphicPath );
		m_sprOpt[i].SetXY( OPT_X[i], OPT_Y[i] );
		m_sprOpt[i].SetZoom( 1 );
		this->AddActor( &m_sprOpt[i] );
	}


	m_Menu.Load( 	
		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_BACKGROUND), 
		THEME->GetPathTo(GRAPHIC_SELECT_STYLE_TOP_EDGE),
		ssprintf("Use %c %c to select, then press START", char(1), char(2) )
		);
	this->AddActor( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo(SOUND_SELECT_STYLE_CHANGE) );
	m_soundSelect.Load( THEME->GetPathTo(SOUND_MENU_START) );


//	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_STYLE_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo(SOUND_MENU_MUSIC) );
        MUSIC->Play( true );
	}

//	AfterChange();
//	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );
}

/************************************
~ScreenEz2SelectStyle (Destructor)
Desc: Writes line to log when screen
is terminated.
************************************/
ScreenEz2SelectPlayer::~ScreenEz2SelectPlayer()
{
	LOG->WriteLine( "ScreenEz2SelectPlayer::~ScreenEz2SelectPlayer()" );
}


/************************************
AnimateGraphics
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectPlayer::AnimateGraphics()
{
/********** THIS NEEDS IMPROVING WITH TIMER FUNCTIONS ***********/

if (bounce < 10 && direct == 0 && wait == 2) // Bounce 1p/2p up
	{
		bounce++;
		if (m_iSelectedStyle == 0)
		{
			m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - bounce);
		}
		else if (m_iSelectedStyle == 1)
		{
			m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - bounce);
		}
		else
		{
			m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - bounce);
			m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - bounce);
		}
		
		if (bounce == 10)
		{
			direct = 1;
		}
		wait = 0;
	}
	else if (bounce > 0 && direct == 1 && wait == 2) // bounce 1p/2p down
	{
		bounce--;
		if (m_iSelectedStyle == 0)
		{
			m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - bounce);
		}
		else if (m_iSelectedStyle == 1)
		{
			m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - bounce);
		}
		else
		{
			m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - bounce);
			m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - bounce);
		}

		if (bounce == 0)
		{
			direct = 0;
		}
		wait = 0;
	}

	if (wait < 2) // don't make the 1p/2p bounce too quick.
	{
		wait++;
	}
}

/************************************
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenEz2SelectPlayer::DrawPrimitives()
{
	AnimateGraphics();
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

/************************************
Input
Desc: Handles player input.
************************************/
void ScreenEz2SelectPlayer::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenEz2SelectPlayer::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

/************************************
HandleScreenMessage
Desc: Handles Screen Messages and changes
	game states.
************************************/
void ScreenEz2SelectPlayer::HandleScreenMessage( const ScreenMessage SM )
{
	Screen::HandleScreenMessage( SM );

	switch( SM )
	{
	case SM_MenuTimer:
		MenuStart(PLAYER_NONE);
		break;
	case SM_GoToPrevState:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		SCREENMAN->SetNewScreen( new ScreenEz2SelectStyle );
		break;
	}
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectPlayer::MenuBack( const PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevState, true );

//	m_Fade.CloseWipingLeft( SM_GoToPrevState );

//	TweenOffScreen();
}

/************************************
SetFadedStyles
Desc: Fades out non-highlighted items
depending on the users choice.
************************************/
void ScreenEz2SelectPlayer::SetFadedStyles()
{
	m_sprOpt[0].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[1].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[2].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[3].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	if (m_iSelectedStyle != 1)
	{
		m_sprOpt[2].SetTweenDiffuseColor( OPT_SELECTED );
		m_sprOpt[3].SetTweenDiffuseColor( OPT_SELECTED );
	}
	else
	{
		m_sprOpt[0].SetTweenDiffuseColor( OPT_SELECTED );
		m_sprOpt[1].SetTweenDiffuseColor( OPT_SELECTED );
	}
}

/************************************
MenuRight
Desc: Actions performed when a player 
presses the button bound to right
************************************/
void ScreenEz2SelectPlayer::MenuRight( PlayerNumber p )
{

	m_soundChange.PlayRandom();

	if (m_iSelectedStyle == 2)
	{
		m_iSelectedStyle = 0;
	}
	else
	{
		m_iSelectedStyle++;
	}

}

/************************************
MenuLeft
Desc: Actions performed when a player 
presses the button bound to left
************************************/
void ScreenEz2SelectPlayer::MenuLeft( PlayerNumber p )
{

	m_soundChange.PlayRandom();

	if (m_iSelectedStyle == 0)
	{
		m_iSelectedStyle = 2;
	}
	else
	{
		m_iSelectedStyle--;
	}

}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectPlayer::MenuStart( PlayerNumber p )
{
//	GAME->m_sCurrentStyle = DANCE_STYLES[m_iSelectedStyle];

	MUSIC->Stop();
	
	m_soundSelect.PlayRandom();
	
	this->ClearMessageQueue();

	m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

	TweenOffScreen();

}

/************************************
TweenOffScreen
Desc: Squashes graphics before the screen
changes state.
************************************/
void ScreenEz2SelectPlayer::TweenOffScreen()
{
		if (m_iSelectedStyle == 0 || m_iSelectedStyle == 2)
		{	
			m_sprOpt[1].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprOpt[1].SetTweenZoomY( 0 );
			m_sprOpt[2].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprOpt[2].SetTweenZoomY( 0 );
		}
		if (m_iSelectedStyle == 1 || m_iSelectedStyle == 2)
		{	
			m_sprOpt[0].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprOpt[0].SetTweenZoomY( 0 );
			m_sprOpt[3].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
			m_sprOpt[3].SetTweenZoomY( 0 );
		}
}
