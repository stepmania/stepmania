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
const ScreenMessage SM_PlayersChosen		=	ScreenMessage(SM_User + 3);

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



/************************************
ScreenEz2SelectPlayer (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()
{
	LOG->Trace( "ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()" );
	m_iSelectedStyle=0;
	GAMESTATE->m_CurStyle = STYLE_NONE;
	ez2_bounce=0.f;
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
Update
Desc: Animates the 1p/2p selection
************************************/
void ScreenEz2SelectPlayer::Update( float fDeltaTime )
{
	Screen::Update( fDeltaTime );

	fDeltaTime /= .01f;

	ez2_bounce = fmod((ez2_bounce+fDeltaTime), 20);

	/* 0..10..19 -> 10..0..9 */
	int offset = abs(10-ez2_bounce);
	m_sprOpt[2].SetXY( OPT_X[2], OPT_Y[2] - offset);
	m_sprOpt[3].SetXY( OPT_X[3], OPT_Y[3] - offset);
}

/************************************
DrawPrimitives
Desc: Draws the screen =P
************************************/

void ScreenEz2SelectPlayer::DrawPrimitives()
{
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
		/* If a player has already chosen, then he chose within the last second
		 * of the menu timer; just stop and let it come through as if the menu
		 * timer didn't expire. */
		if(!m_iSelectedStyle)
			MenuStart(PLAYER_1);
		break;
	case SM_PlayersChosen:
		if (m_iSelectedStyle & EZ2_PLAYER_1) 
		{
			if (m_iSelectedStyle & EZ2_PLAYER_2) 
			{
				// they both selected
//				GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
//				GAMESTATE->m_bIsJoined[PLAYER_1] = true;
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE_VERSUS;
			} else {
				// only the left pad was selected
//				GAMESTATE->m_MasterPlayerNumber = PLAYER_1;
				GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;
//				GAMESTATE->m_bIsJoined[PLAYER_2] = true;
			}
		}
		else if (m_iSelectedStyle & EZ2_PLAYER_2)  {
			// only the right pad was selected
//			GAMESTATE->m_MasterPlayerNumber = PLAYER_2;
			GAMESTATE->m_CurStyle = STYLE_EZ2_SINGLE;
//			GAMESTATE->m_bIsJoined[PLAYER_2] = true;
		} else ASSERT(0);

		MUSIC->Stop();
		
		m_Menu.TweenOffScreenToMenu( SM_GoToNextState );

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
	int this_player_bit = (p == PLAYER_1)? EZ2_PLAYER_1:EZ2_PLAYER_2;

	// disallow multiple presses of the menu start.
	if ( m_iSelectedStyle & this_player_bit )
		return;
	
	// figure out whether we should add a player into the fray or not
//	if(	GAMESTATE->m_MasterPlayerNumber != PLAYER_2 && GAMESTATE->m_MasterPlayerNumber != PLAYER_1 )
	if (m_iSelectedStyle == 0)
	{
//		GAMESTATE->m_MasterPlayerNumber = p;

		m_iSelectedStyle |= this_player_bit;

		// wait for a bit in case another player wants to join before moving on.
		SCREENMAN->SendMessageToTopScreen( SM_PlayersChosen, 1.f );
	}
	else
	{
		m_iSelectedStyle |= this_player_bit;
	}

	m_soundSelect.PlayRandom();
	TweenOffScreen();
}

/************************************
TweenOffScreen
Desc: Squashes graphics before the screen
changes state.
************************************/
void ScreenEz2SelectPlayer::TweenOffScreen()
{
	if (m_iSelectedStyle & EZ2_PLAYER_1)
	{	
		m_sprOpt[1].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprOpt[1].SetTweenZoomY( 0 );
		m_sprOpt[2].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprOpt[2].SetTweenZoomY( 0 );
	}
	if (m_iSelectedStyle & EZ2_PLAYER_2)
	{	
		m_sprOpt[0].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprOpt[0].SetTweenZoomY( 0 );
		m_sprOpt[3].BeginTweening( MENU_ELEMENTS_TWEEN_TIME );
		m_sprOpt[3].SetTweenZoomY( 0 );
	}
}
