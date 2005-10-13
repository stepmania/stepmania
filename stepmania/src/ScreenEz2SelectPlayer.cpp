#include "global.h"
#include "ScreenEz2SelectPlayer.h"
#include "ScreenManager.h"
#include "PrefsManager.h"
#include "GameSoundManager.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "GameManager.h"
#include "RageLog.h"
#include "AnnouncerManager.h"
#include "GameState.h"
#include "RageException.h"
#include "RageTimer.h"
#include "ThemeManager.h"
#include "MenuTimer.h"
#include "ScreenDimensions.h"

/* Constants */
#define JOIN_FRAME_X( p )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinFrameP%dX",p+1))
#define JOIN_FRAME_Y( i )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinFrameP%dY",i+1))
#define JOIN_MESSAGE_X( p )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinMessageP%dX",p+1))
#define JOIN_MESSAGE_Y( i )		THEME->GetMetricF("ScreenEz2SelectPlayer",ssprintf("JoinMessageP%dY",i+1))
#define HELP_TEXT				THEME->GetMetric("ScreenEz2SelectPlayer","HelpText")
#define TIMER_SECONDS			THEME->GetMetricI("ScreenEz2SelectPlayer","TimerSeconds")
#define SILENT_WAIT				THEME->GetMetricB("ScreenEz2SelectPlayer","SilentWait")
#define BOUNCE_JOIN_MESSAGE		THEME->GetMetricB("ScreenEz2SelectPlayer","BounceJoinMessage")
#define FOLD_ON_JOIN			THEME->GetMetricB("ScreenEz2SelectPlayer","FoldOnJoin")

const float TWEEN_TIME		= 0.35f;


/************************************
ScreenEz2SelectPlayer (Constructor)
Desc: Sets up the screen display
************************************/

REGISTER_SCREEN_CLASS( ScreenEz2SelectPlayer );
ScreenEz2SelectPlayer::ScreenEz2SelectPlayer( CString sName ) : ScreenWithMenuElements( sName )
{
	// Unjoin the players, then let them join back in on this screen
//	GAMESTATE->m_bPlayersCanJoin = true;
	FOREACH_PlayerNumber( p )
		GAMESTATE->m_bSideIsJoined[p] = false;

	LOG->Trace( "ScreenEz2SelectPlayer::ScreenEz2SelectPlayer()" );
}


void ScreenEz2SelectPlayer::Init()
{
	ScreenWithMenuElements::Init();

	FOREACH_PlayerNumber( p )
	{
		m_sprJoinFrame[p].Load( THEME->GetPathG("ScreenEz2SelectPlayer","join frame 1x2") );
		m_sprJoinFrame[p].StopAnimating();
		m_sprJoinFrame[p].SetState( p );
		m_sprJoinFrame[p].SetXY( JOIN_FRAME_X(p), JOIN_FRAME_Y(p) );
		this->AddChild( &m_sprJoinFrame[p] );

		if( GAMESTATE->m_bSideIsJoined[p] )
			m_sprJoinFrame[p].SetZoomY( 0 );

		m_sprJoinMessage[p].Load( THEME->GetPathG("ScreenEz2SelectPlayer","join message 2x2") );
		m_sprJoinMessage[p].StopAnimating();
		m_sprJoinMessage[p].SetState( p );
		m_sprJoinMessage[p].SetXY( JOIN_MESSAGE_X(p), JOIN_MESSAGE_Y(p) );
		if( BOUNCE_JOIN_MESSAGE )
			m_sprJoinMessage[p].SetEffectBounce( 0.5f, RageVector3(0,10,0) );
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

	SOUND->PlayOnceFromDir( ANNOUNCER->GetPathTo("select player intro") );

	SOUND->PlayMusic( THEME->GetPathS("ScreenSelectPlayer","music") );

	TweenOursOnScreen();
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
Input
Desc: Handles player input.
************************************/
void ScreenEz2SelectPlayer::Input( const InputEventPlus &input )
{
	LOG->Trace( "ScreenEz2SelectPlayer::Input()" );

	if( IsTransitioning() )
		return;

	Screen::Input( input );	// default input handler
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
		}

		StartTransitioningScreen( SM_GoToNextScreen );
		break;
	}

	ScreenWithMenuElements::HandleScreenMessage( SM );
}


/************************************
MenuBack
Desc: Actions performed when a player 
presses the button bound to back
************************************/

void ScreenEz2SelectPlayer::MenuBack( PlayerNumber pn )
{
	SOUND->StopMusic();

	Cancel( SM_GoToPrevScreen );
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
	SCREENMAN->PlayStartSound();
	m_sprJoinMessage[pn].SetState( pn+NUM_PLAYERS );

	if( FOLD_ON_JOIN )
	{
		m_sprJoinMessage[pn].BeginTweening( 0.25f );
		m_sprJoinMessage[pn].SetZoomY( 0 );
		m_sprJoinFrame[pn].BeginTweening( 0.25f );
		m_sprJoinFrame[pn].SetZoomY( 0 );
	}

	bool bBothSidesJoined = true;
	FOREACH_PlayerNumber( p )
		if( !GAMESTATE->m_bSideIsJoined[p] )
			bBothSidesJoined = false;

	if( bBothSidesJoined )
	{
		StartTransitioningScreen( SM_GoToNextScreen );
	}
	else
	{
		// give the other player a little time to join
		m_MenuTimer->SetSeconds( 1 );
		m_MenuTimer->Start();
		m_MenuTimer->EnableStealth( SILENT_WAIT ); // do we wanna make the timer 'quiet' ?
	}
}

void ScreenEz2SelectPlayer::TweenOursOnScreen()
{
	FOREACH_PlayerNumber( p )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH/2 : +SCREEN_WIDTH/2 );

		float fOriginalX;
		
		fOriginalX = m_sprJoinMessage[p].GetX();
		m_sprJoinMessage[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinMessage[p].SetX( fOriginalX );

		fOriginalX = m_sprJoinFrame[p].GetX();
		m_sprJoinFrame[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_BOUNCE_END );
		m_sprJoinFrame[p].SetX( fOriginalX );
	}
}

void ScreenEz2SelectPlayer::TweenOffScreen()
{
	ScreenWithMenuElements::TweenOffScreen();

	FOREACH_PlayerNumber( p )
	{
		float fOffScreenOffset = float( (p==PLAYER_1) ? -SCREEN_WIDTH : +SCREEN_WIDTH );

		m_sprJoinMessage[p].BeginTweening( 0.5f, Actor::TWEEN_DECELERATE );
		m_sprJoinMessage[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
		m_sprJoinFrame[p].BeginTweening( 0.5f, Actor::TWEEN_DECELERATE );
		m_sprJoinFrame[p].SetX( m_sprJoinMessage[p].GetX()+fOffScreenOffset );
	}
}

/*
 * (c) 2002-2003 "Frieza"
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
