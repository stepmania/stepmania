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
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"

/* Constants */

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


#define CURSOR_P1_X				THEME->GetMetricF("ScreenEz2SelectPlayer","CursorP1X")
#define CURSOR_P1_Y				THEME->GetMetricF("ScreenEz2SelectPlayer","CursorP1Y")
#define CURSOR_P2_X				THEME->GetMetricF("ScreenEz2SelectPlayer","CursorP2X")
#define CURSOR_P2_Y				THEME->GetMetricF("ScreenEz2SelectPlayer","CursorP2Y")
#define CONTROLLER_P1_X			THEME->GetMetricF("ScreenEz2SelectPlayer","ControllerP1X")
#define CONTROLLER_P1_Y			THEME->GetMetricF("ScreenEz2SelectPlayer","ControllerP1Y")
#define CONTROLLER_P2_X			THEME->GetMetricF("ScreenEz2SelectPlayer","ControllerP2X")
#define CONTROLLER_P2_Y			THEME->GetMetricF("ScreenEz2SelectPlayer","ControllerP2Y")
#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectPlayer","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectPlayer","TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric("ScreenEz2SelectPlayer","NextScreen")

float CURSOR_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return CURSOR_P1_X;
		case PLAYER_2:	return CURSOR_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float CURSOR_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return CURSOR_P1_Y;
		case PLAYER_2:	return CURSOR_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}
float CONTROLLER_X( int p ) {
	switch( p ) {
		case PLAYER_1:	return CONTROLLER_P1_X;
		case PLAYER_2:	return CONTROLLER_P2_X;
		default:		ASSERT(0);	return 0;
	}
}
float CONTROLLER_Y( int p ) {
	switch( p ) {
		case PLAYER_1:	return CONTROLLER_P1_Y;
		case PLAYER_2:	return CONTROLLER_P2_Y;
		default:		ASSERT(0);	return 0;
	}
}

const float TWEEN_TIME		= 0.35f;


/************************************
ScreenEz2SelectPlayer (Constructor)
Desc: Sets up the screen display
************************************/

ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()
{
	// Unjoin the players, then let them join back in on this screen
	GAMESTATE->m_bPlayersCanJoin = true;
	int p;
	for( p=0; p<NUM_PLAYERS; p++ )
		GAMESTATE->m_bSideIsJoined[p] = false;

	LOG->Trace( "ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()" );

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprControllers[p].Load( THEME->GetPathTo("Graphics","select player controller") );
		m_sprControllers[p].SetXY( CONTROLLER_X(p), CONTROLLER_Y(p) );
		this->AddSubActor( &m_sprControllers[p] );

		m_sprCursors[p].Load( THEME->GetPathTo("Graphics",ssprintf("select player cursor p%d",p+1)) );
		m_sprCursors[p].SetXY( CURSOR_X(p), CURSOR_Y(p) );
		m_sprCursors[p].SetEffectBouncing( D3DXVECTOR3(0,10,0), 0.5f );
		this->AddSubActor( &m_sprCursors[p] );
	}

	m_Menu.Load( 	
		THEME->GetPathTo("Graphics","select player background"), 
		THEME->GetPathTo("Graphics","select player top edge"),
		HELP_TEXT, true, TIMER_SECONDS
		);
	this->AddSubActor( &m_Menu );

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUND->PlayOnceStreamedFromDir( ANNOUNCER->GetPathTo("select player intro") );

	MUSIC->LoadAndPlayIfNotAlready( THEME->GetPathTo("Sounds","select player music") );

	TweenOnScreen();
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
		{
			bool bAtLeastOneJoined = false;
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->m_bSideIsJoined[p] )
					bAtLeastOneJoined = true;

			if( !bAtLeastOneJoined )
			{
				MenuStart(PLAYER_1);
				m_Menu.StopTimer();
			}
	
			TweenOffScreen();
			m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		}
		break;
	case SM_GoToPrevScreen:
		MUSIC->Stop();
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN );
		break;
	}
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectPlayer::MenuBack( PlayerNumber p )
{
	MUSIC->Stop();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
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
	if( GAMESTATE->m_bSideIsJoined[p] )	// already joined
		return;	// ignore

	GAMESTATE->m_bSideIsJoined[p] = true;
	SCREENMAN->RefreshCreditsMessages();
	m_soundSelect.PlayRandom();
	m_sprCursors[p].BeginTweening( 0.25f );
	m_sprCursors[p].SetTweenZoomY( 0 );
	m_sprControllers[p].BeginTweening( 0.25f );
	m_sprControllers[p].SetTweenZoomY( 0 );

	bool bBothSidesJoined = true;
	for( int pn=0; p<NUM_PLAYERS; pn++ )
		if( !GAMESTATE->m_bSideIsJoined[pn] )
			bBothSidesJoined = false;

	if( bBothSidesJoined )
	{
		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
	}
	else
	{
		// give the other player a little time to join
		m_Menu.SetTimer( 5 );
		m_Menu.StartTimer();
	}
}

void ScreenEz2SelectPlayer::TweenOnScreen()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH/2 : +SCREEN_WIDTH/2 );

		float fOriginalX;
		
		fOriginalX = m_sprCursors[p].GetX();
		m_sprCursors[p].SetX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprCursors[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprCursors[p].SetTweenX( fOriginalX );

		fOriginalX = m_sprControllers[p].GetX();
		m_sprControllers[p].SetX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprControllers[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprControllers[p].SetTweenX( fOriginalX );
	}
}

void ScreenEz2SelectPlayer::TweenOffScreen()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH : +SCREEN_WIDTH );

		m_sprCursors[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprCursors[p].SetTweenX( m_sprCursors[p].GetX()+fOffScreenOffset );
		m_sprControllers[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprControllers[p].SetTweenX( m_sprCursors[p].GetX()+fOffScreenOffset );
	}
}
