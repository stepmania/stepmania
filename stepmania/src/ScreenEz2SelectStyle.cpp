/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy
*****************************************/

/* Includes */

#include "stdafx.h"
#include "ScreenEz2SelectStyle.h"
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

/* Constants */

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);

const CString DANCE_STYLES[] = {
	"easy",
	"hard",
	"real",
	"double",
};

const float TWEEN_TIME		= 0.35f;
const D3DXCOLOR OPT_NOT_SELECTED = D3DXCOLOR(0.3f,0.3f,0.3f,1);
const D3DXCOLOR OPT_SELECTED = D3DXCOLOR(1.0f,1.0f,1.0f,1);


const float OPT_X[NUM_EZ2STYLE_GRAPHICS] = { 
	CENTER_X-350,
	CENTER_X,
	CENTER_X+350,
	CENTER_X+700,
}; // tells us the default X position
const float OPT_Y[NUM_EZ2STYLE_GRAPHICS] = {
	CENTER_Y,
	CENTER_Y,
	CENTER_Y,
	CENTER_Y,
}; // tells us the default Y position

const float CLUB_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[0]+0,
	OPT_X[0]-350,
	OPT_X[2]+0,
	OPT_X[1]+0,
};

const float EASY_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[1]+0,
	OPT_X[0]+0,
	OPT_X[0]-350,
	OPT_X[2]+0,
};

const float HARD_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[2]+0,
	OPT_X[1]+0,
	OPT_X[0]+0,
	OPT_X[0]-350,
};

const float REAL_X[NUM_EZ2STYLE_GRAPHICS] = {
	OPT_X[3]+0,
	OPT_X[2]+0,
	OPT_X[1]+0,
	OPT_X[0]+0,
};


/************************************
ScreenEz2SelectStyle (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectStyle::ScreenEz2SelectStyle()
{
	LOG->WriteLine( "ScreenEz2SelectStyle::ScreenEz2SelectStyle()" );

	m_iSelectedStyle=0;

// Load in the sprites we will be working with.
	for( int i=0; i<NUM_EZ2STYLE_GRAPHICS; i++ )
	{
		CString sPadGraphicPath;
		switch( i )
		{
		case 0:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_3);	//HACK! Would LIKE to have own filename :)
			break;
		case 1:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_0);	
			break;
		case 2:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_1);	
			break;
		case 3:
			sPadGraphicPath = THEME->GetPathTo(GRAPHIC_SELECT_STYLE_INFO_GAME_0_STYLE_2);	
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
		MUSIC->Load( THEME->GetPathTo(SOUND_MUSIC_SCROLL_MUSIC) );
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
ScreenEz2SelectStyle::~ScreenEz2SelectStyle()
{
	LOG->WriteLine( "ScreenEz2SelectStyle::~ScreenEz2SelectStyle()" );
}


/************************************
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenEz2SelectStyle::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

/************************************
Input
Desc: Handles player input.
************************************/
void ScreenEz2SelectStyle::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->WriteLine( "ScreenEz2SelectStyle::Input()" );

	if( m_Menu.IsClosing() )
		return;

	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );	// default input handler
}

/************************************
HandleScreenMessage
Desc: Handles Screen Messages and changes
	game states.
************************************/
void ScreenEz2SelectStyle::HandleScreenMessage( const ScreenMessage SM )
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
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	}
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectStyle::MenuBack( const PlayerNumber p )
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
void ScreenEz2SelectStyle::SetFadedStyles()
{
	m_sprOpt[0].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[1].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[2].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	m_sprOpt[3].SetTweenDiffuseColor( OPT_NOT_SELECTED );
	if (m_iSelectedStyle != 3)
	{
		m_sprOpt[m_iSelectedStyle + 1].SetTweenDiffuseColor( OPT_SELECTED );
	}
	else
	{
		m_sprOpt[0].SetTweenDiffuseColor( OPT_SELECTED );
	}
}

/************************************
MenuRight
Desc: Actions performed when a player 
presses the button bound to right
************************************/
void ScreenEz2SelectStyle::MenuRight( PlayerNumber p )
{

	if( m_iSelectedStyle == 3 )		// wrap to the first dance style
		m_iSelectedStyle = 0; // which is (club) easy (hard)
	else
		m_iSelectedStyle = m_iSelectedStyle + 1; // otherwise shuffle up a style...

	if(m_iSelectedStyle == 2) // If it's HARD and CLUB needs to appear from the other side...
	{
		m_sprOpt[0].SetXY( REAL_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
	}
	else if(m_iSelectedStyle == 3) // If it's REAL and EASY needs to appear from the other side...
	{
		m_sprOpt[1].SetXY( CLUB_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
	}
	else if(m_iSelectedStyle == 0) // If it's CLUB and HARD needs to appear from the other side...
	{
		m_sprOpt[2].SetXY( EASY_X[m_iSelectedStyle]+700, OPT_Y[0] ); // First move it over the other side off-screen...
		m_sprOpt[3].SetXY( EASY_X[m_iSelectedStyle]+1050, OPT_Y[0] ); // REAL must also move, due to the fact that it is the end item at the start, it moves differently to the rest.
	}

	m_sprOpt[0].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[0].SetTweenX( CLUB_X[m_iSelectedStyle] );
	m_sprOpt[0].SetTweenY( OPT_Y[m_iSelectedStyle] );


	m_sprOpt[1].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[1].SetTweenX( EASY_X[m_iSelectedStyle] );
	m_sprOpt[1].SetTweenY( OPT_Y[m_iSelectedStyle] );

	m_sprOpt[2].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[2].SetTweenX( HARD_X[m_iSelectedStyle] );
	m_sprOpt[2].SetTweenY( OPT_Y[m_iSelectedStyle] );

	m_sprOpt[3].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[3].SetTweenX( REAL_X[m_iSelectedStyle] );
	m_sprOpt[3].SetTweenY( OPT_Y[m_iSelectedStyle] );
	
	SetFadedStyles();

}

/************************************
MenuLeft
Desc: Actions performed when a player 
presses the button bound to left
************************************/
void ScreenEz2SelectStyle::MenuLeft( PlayerNumber p )
{

if( m_iSelectedStyle == 0 )		// wrap to the last dance style
		m_iSelectedStyle = 3;
	else
	m_iSelectedStyle = m_iSelectedStyle - 1;

	if( m_iSelectedStyle == 3 )
	{
		m_sprOpt[3].SetXY( CLUB_X[m_iSelectedStyle]-700, OPT_Y[0] ); // First move it over the other side off-screen...		
		m_sprOpt[2].SetXY( CLUB_X[m_iSelectedStyle]-1050, OPT_Y[0] );	
	}
	else if( m_iSelectedStyle == 2 )
	{
		m_sprOpt[1].SetXY( HARD_X[m_iSelectedStyle]-700, OPT_Y[0] ); // First move it over the other side off-screen...			
	}
	else if( m_iSelectedStyle == 1 )
	{
		m_sprOpt[0].SetXY( EASY_X[m_iSelectedStyle]-1050, OPT_Y[0] ); // First move it over the other side off-screen...			
	}

	m_sprOpt[0].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[0].SetTweenX( CLUB_X[m_iSelectedStyle] );
	m_sprOpt[0].SetTweenY( OPT_Y[m_iSelectedStyle] );

	m_sprOpt[1].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[1].SetTweenX( EASY_X[m_iSelectedStyle] );
	m_sprOpt[1].SetTweenY( OPT_Y[m_iSelectedStyle] );

	m_sprOpt[2].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[2].SetTweenX( HARD_X[m_iSelectedStyle] );
	m_sprOpt[2].SetTweenY( OPT_Y[m_iSelectedStyle] );

	m_sprOpt[3].BeginTweening( 0.2f, TWEEN_BIAS_BEGIN );
	m_sprOpt[3].SetTweenX( REAL_X[m_iSelectedStyle] );
	m_sprOpt[3].SetTweenY( OPT_Y[m_iSelectedStyle] );	

	SetFadedStyles();

}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectStyle::MenuStart( PlayerNumber p )
{
//	GAMEMAN->m_CurStyle = DANCE_STYLES[m_iSelectedStyle];
	
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
void ScreenEz2SelectStyle::TweenOffScreen()
{
	/*	if (m_iSelectedStyle == 0 || m_iSelectedStyle == 2)
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
		}*/
}
