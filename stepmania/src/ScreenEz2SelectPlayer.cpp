#include "stdafx.h"
/****************************************
ScreenEzSelectPlayer,cpp
Desc: See Header
Copyright (C):
Andrew Livy
*****************************************/

/* Includes */

#include "ScreenEz2SelectPlayer.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "RageMusic.h"
#include "ScreenTitleMenu.h"
#include "ScreenCaution.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ScreenSelectDifficulty.h"
#include "ScreenSandbox.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "ScreenSelectStyle.h"
#include "ScreenEz2SelectStyle.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"

/* Constants */

const ScreenMessage SM_GoToPrevState		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextState		=	ScreenMessage(SM_User + 2);

#define USE_NORMAL_OR_EZ2_SELECT_STYLE		THEME->GetMetricB("General","UseNormalOrEZ2SelectStyle")

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



float ez2_lasttimercheck[2];
int ez2_bounce=0; // used for the bouncing of the '1p' and '2p' images
int ez2_direct=0; // direction of the bouncing of the '1p' and '2p' images

/************************************
ScreenEz2SelectPlayer (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()
{
	LOG->Trace( "ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()" );
	ez2_lasttimercheck[0] = TIMER->GetTimeSinceStart();
	ez2_lasttimercheck[1] = 0.0f;
	m_iSelectedStyle=3; // by bbf: frieza, was this supposed to be 3 ?
	GAMESTATE->m_CurStyle = STYLE_NONE;
//	GAMESTATE->m_MasterPlayerNumber = PLAYER_INVALID;

// Load in the sprites we will be working with.
	for( int i=0; i<NUM_EZ2_GRAPHICS; i++ )
	{
		CString sOptFileName;
		switch( i )
		{
		case 0:	sOptFileName = "select difficulty hard picture";	break;
		case 1:	sOptFileName = "select difficulty hard picture";	break;
		case 2:	sOptFileName = "select difficulty medium picture";	break;
		case 3:	sOptFileName = "select difficulty easy picture";	break;
		}
		m_sprOpt[i].Load( THEME->GetPathTo("Graphics",sOptFileName) );
		m_sprOpt[i].SetXY( OPT_X[i], OPT_Y[i] );
		this->AddSubActor( &m_sprOpt[i] );
	}


	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select style background"), 
		THEME->GetPathTo("Graphics","select style top edge"),
		ssprintf("Press %c on the pad you wish to play on", char(4) ),
		false, true, 40 
		);
	this->AddSubActor( &m_Menu );

	m_soundChange.Load( THEME->GetPathTo("Sounds","select style change") );
	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );
	m_soundInvalid.Load( THEME->GetPathTo("Sounds","menu invalid") );

	/* Chris:  If EZ2 doesn't use this sound, make a theme that overrides is with a silent sound file */
	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo(ANNOUNCER_SELECT_STYLE_INTRO) );


	if( !MUSIC->IsPlaying() )
	{
		MUSIC->Load( THEME->GetPathTo("Sounds","select player music") );
        MUSIC->Play( true );
	}

//	GAMESTATE->m_bPlayersCanJoin = true;
//	GAMESTATE->m_bIsJoined[PLAYER_1] = false;
//	GAMESTATE->m_bIsJoined[PLAYER_2] = false;

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
	LOG->Trace( "ScreenEz2SelectPlayer::~ScreenEz2SelectPlayer()" );
}


/************************************
AnimateGraphics
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectPlayer::AnimateGraphics()
{

//if (bounce < 10 && direct == 0 && wait == 2) // Bounce 1p/2p up
if (TIMER->GetTimeSinceStart() > ez2_lasttimercheck[0] + 0.01f && ez2_direct == 0)
	{
		ez2_lasttimercheck[0] = TIMER->GetTimeSinceStart();
		ez2_bounce+=1;
	
		m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - ez2_bounce);
		m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - ez2_bounce);


		if (ez2_bounce == 10)
		{
			ez2_direct = 1;
		}
	}
	else if (TIMER->GetTimeSinceStart() > ez2_lasttimercheck[0] + 0.01f && ez2_direct == 1) // bounce 1p/2p down
	{
		ez2_lasttimercheck[0] = TIMER->GetTimeSinceStart();
		ez2_bounce-=1;
	
		m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - ez2_bounce);
		m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - ez2_bounce);

		if (ez2_bounce == 0)
		{
			ez2_direct = 0;
		}
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
	
	// wait for a bit incase another player wants to join before moving on.
	if (ez2_lasttimercheck[1] != 0.0f && TIMER->GetTimeSinceStart() > ez2_lasttimercheck[1] + 1)
	{
		ez2_lasttimercheck[1] = 0.0f;
		
		if (m_iSelectedStyle == 0) // only the left pad was selected
		{
//			GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
			GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;
		}
		else if (m_iSelectedStyle == 1) // only the right pad was selected
		{
//			GAMESTATE->m_MasterPlayerNumber = PLAYER_2;
			GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;
		}
		else // they both selected
		{
//			GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
			GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;
		}

		MUSIC->Stop();
		
		this->ClearMessageQueue();

		m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

		TweenOffScreen();	
	}

}

/************************************
Input
Desc: Handles player input.
************************************/
void ScreenEz2SelectPlayer::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	LOG->Trace( "ScreenEz2SelectPlayer::Input()" );

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
		MenuStart(PLAYER_1); // Automatically Choose Player 1
		break;
	case SM_GoToPrevState:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( new ScreenTitleMenu );
		break;
	case SM_GoToNextState:
		if( USE_NORMAL_OR_EZ2_SELECT_STYLE )
			SCREENMAN->SetNewScreen( new ScreenEz2SelectStyle );
		else
			SCREENMAN->SetNewScreen( new ScreenSelectStyle );
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
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/
void ScreenEz2SelectPlayer::MenuDown( PlayerNumber p )
{
	MenuStart( p );
}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectPlayer::MenuStart( PlayerNumber p )
{
	//disallow multiple presses of the menu start.
	if ( (m_iSelectedStyle == 0 && p == PLAYER_1) || (m_iSelectedStyle == 1 && p == PLAYER_2))
	{
		return;
	}

	// figure out whether we should add a player into the fray or not
//	if(	GAMESTATE->m_MasterPlayerNumber != PLAYER_2 && GAMESTATE->m_MasterPlayerNumber != PLAYER_1 )
	if (m_iSelectedStyle == 3)
	{
//		GAMESTATE->m_MasterPlayerNumber = p;

		if (p == PLAYER_1)
		{
//			GAMESTATE->m_bIsJoined[PLAYER_1] = true;
			m_iSelectedStyle = 0;
		}
		else
		{
			m_iSelectedStyle = 1;
//			GAMESTATE->m_bIsJoined[PLAYER_2] = true;
		}
		m_soundSelect.PlayRandom();
		ez2_lasttimercheck[1] = TIMER->GetTimeSinceStart(); // start the timer for going to next state
	}
	else
	{
		m_iSelectedStyle = 2;
//		GAMESTATE->m_bIsJoined[PLAYER_1] = true;
//		GAMESTATE->m_bIsJoined[PLAYER_2] = true;
		m_soundSelect.PlayRandom();
	}

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
