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
#include "RageSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "ThemeManager.h"

/* Constants */

const ScreenMessage SM_GoToPrevScreen		=	ScreenMessage(SM_User + 1);
const ScreenMessage SM_GoToNextScreen		=	ScreenMessage(SM_User + 2);


#define JOIN_FRAME_X( p )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinFrameP%dX",p+1))
#define JOIN_FRAME_Y( i )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinFrameP%dY",i+1))
#define JOIN_MESSAGE_X( p )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinMessageP%dX",p+1))
#define JOIN_MESSAGE_Y( i )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinMessageP%dY",i+1))
#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectPlayer","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectPlayer","TimerSeconds")
#define NEXT_SCREEN				THEME->GetMetric("ScreenEz2SelectPlayer","NextScreen")
#define SILENT_WAIT				THEME->GetMetricI("ScreenEz2SelectPlayer","SilentWait")
#define BOUNCE_JOIN_MESSAGE		THEME->GetMetricB("ScreenEz2SelectPlayer","BounceJoinMessage")
#define FOLD_ON_JOIN			THEME->GetMetricB("ScreenEz2SelectPlayer","FoldOnJoin")

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

	m_Menu.Load( 	
		THEME->GetPathTo("BGAnimations","ez2 select player"), 
		THEME->GetPathTo("Graphics","select player top edge"),
		HELP_TEXT, false, true, TIMER_SECONDS
		);
	this->AddChild( &m_Menu );

	m_Background.LoadFromAniDir( THEME->GetPathTo("BGAnimations","ez2 select player") );
	this->AddChild( &m_Background ); // animated background =)

	for( p=0; p<NUM_PLAYERS; p++ )
	{
		m_sprJoinFrame[p].Load( THEME->GetPathTo("Graphics","select player join frame 1x2") );
		m_sprJoinFrame[p].StopAnimating();
		m_sprJoinFrame[p].SetState( p );
		m_sprJoinFrame[p].SetXY( JOIN_FRAME_X(p), JOIN_FRAME_Y(p) );
		this->AddChild( &m_sprJoinFrame[p] );

		if( GAMESTATE->m_bSideIsJoined[p] )
			m_sprJoinFrame[p].SetZoomY( 0 );

		m_sprJoinMessage[p].Load( THEME->GetPathTo("Graphics","select player join message 2x2") );
		m_sprJoinMessage[p].StopAnimating();
		m_sprJoinMessage[p].SetState( p );
		m_sprJoinMessage[p].SetXY( JOIN_MESSAGE_X(p), JOIN_MESSAGE_Y(p) );
		if( BOUNCE_JOIN_MESSAGE )
			m_sprJoinMessage[p].SetEffectBouncing( RageVector3(0,10,0), 0.5f );
		this->AddChild( &m_sprJoinMessage[p] );
	
		if( GAMESTATE->m_bSideIsJoined[p] )
		{
			m_sprJoinMessage[p].SetState( p+NUM_PLAYERS );

			if( FOLD_ON_JOIN )
			{
				m_sprJoinMessage[p].SetZoomY( 0 );
				m_sprJoinFrame[p].SetZoomY( 0 );
			}
		}
	}

	m_soundSelect.Load( THEME->GetPathTo("Sounds","menu start") );

	SOUNDMAN->PlayOnceFromDir( ANNOUNCER->GetPathTo("select player intro") );

	SOUNDMAN->PlayMusic( THEME->GetPathTo("Sounds","select player music") );

	TweenOnScreen();
	m_Menu.TweenOnScreenFromBlack( SM_None );
}

/************************************
~ScreenSelectMode (Destructor)
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
		if( GAMESTATE->GetNumSidesJoined() == 0 )
		{
			MenuStart(PLAYER_1);
			m_Menu.StopTimer();
		}

		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
		break;
	case SM_GoToPrevScreen:
		SOUNDMAN->music->StopPlaying();
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

void ScreenEz2SelectPlayer::MenuBack( PlayerNumber pn )
{
	SOUNDMAN->music->StopPlaying();

	m_Menu.TweenOffScreenToBlack( SM_GoToPrevScreen, true );
}


/************************************
MenuDown
Desc: Actions performed when a player 
presses the button bound to down
************************************/
void ScreenEz2SelectPlayer::MenuDown( PlayerNumber pn )
{
	MenuStart( pn );
}

/************************************
MenuStart
Desc: Actions performed when a player 
presses the button bound to start
************************************/
void ScreenEz2SelectPlayer::MenuStart( PlayerNumber pn )
{
	if( GAMESTATE->m_bSideIsJoined[pn] )	// already joined
		return;	// ignore

	GAMESTATE->m_bSideIsJoined[pn] = true;
	SCREENMAN->RefreshCreditsMessages();
	m_soundSelect.Play();
	m_sprJoinMessage[pn].SetState( pn+NUM_PLAYERS );

	if( FOLD_ON_JOIN )
	{
		m_sprJoinMessage[pn].BeginTweening( 0.25f );
		m_sprJoinMessage[pn].SetTweenZoomY( 0 );
		m_sprJoinFrame[pn].BeginTweening( 0.25f );
		m_sprJoinFrame[pn].SetTweenZoomY( 0 );
	}

	bool bBothSidesJoined = true;
	for( int p=0; p<NUM_PLAYERS; p++ )
		if( !GAMESTATE->m_bSideIsJoined[p] )
			bBothSidesJoined = false;

	if( bBothSidesJoined )
	{
		TweenOffScreen();
		m_Menu.TweenOffScreenToMenu( SM_GoToNextScreen );
	}
	else
	{
		// give the other player a little time to join
		m_Menu.SetTimer( 1 );
		m_Menu.StartTimer();
		m_Menu.StealthTimer( SILENT_WAIT ); // do we wanna make the timer 'quiet' ?
	}
}

void ScreenEz2SelectPlayer::TweenOnScreen()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH/2 : +SCREEN_WIDTH/2 );

		float fOriginalX;
		
		fOriginalX = m_sprJoinMessage[p].GetX();
		m_sprJoinMessage[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinMessage[p].SetTweenX( fOriginalX );

		fOriginalX = m_sprJoinFrame[p].GetX();
		m_sprJoinFrame[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinFrame[p].SetTweenX( fOriginalX );
	}
}

void ScreenEz2SelectPlayer::TweenOffScreen()
{
	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH : +SCREEN_WIDTH );

		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprJoinMessage[p].SetTweenX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_BIAS_END );
		m_sprJoinFrame[p].SetTweenX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
	}
}
