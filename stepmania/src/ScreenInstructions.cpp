#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenInstructions

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenInstructions.h"
#include "RageUtil.h"
#include "RageSoundManager.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"


#define HELP_TEXT			THEME->GetMetric("ScreenInstructions","HelpText")
#define TIMER_SECONDS		THEME->GetMetricI("ScreenInstructions","TimerSeconds")
#define NEXT_SCREEN( pm )	THEME->GetMetric("ScreenInstructions","NextScreen"+Capitalize(PlayModeToString(pm)) )


ScreenInstructions::ScreenInstructions() : Screen("ScreenInstructions")
{
	LOG->Trace( "ScreenInstructions::ScreenInstructions()" );

	//
	// Skip this screen unless someone chose easy or beginner
	//
	if( !PREFSMAN->m_bInstructions )
	{
		HandleScreenMessage( SM_GoToNextScreen );
		return;
	}
	if( GAMESTATE->m_PlayMode == PLAY_MODE_ARCADE )
	{
		Difficulty easiestDifficulty = (Difficulty)(NUM_DIFFICULTIES-1);
		for( int p=0; p<NUM_PLAYERS; p++ )
		{
			if( !GAMESTATE->IsHumanPlayer(p) )
				continue;
			easiestDifficulty = min( easiestDifficulty, GAMESTATE->m_PreferredDifficulty[p] );
		}
		if( easiestDifficulty > DIFFICULTY_EASY )
		{
			HandleScreenMessage( SM_GoToNextScreen );
			return;
		}
	}

	m_Menu.Load("ScreenInstructions");
	this->AddChild( &m_Menu );

	CString sHowToPlayPath;
	if( GAMESTATE->m_PlayMode != PLAY_MODE_INVALID )
		sHowToPlayPath = THEME->GetPathToG("ScreenInstructions "+PlayModeToString(GAMESTATE->m_PlayMode)) ;
	else
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before showing ScreenInstructions." );

	m_sprHowToPlay.Load( sHowToPlayPath );
	m_sprHowToPlay.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprHowToPlay );

	m_sprHowToPlay.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_sprHowToPlay.BeginTweening( 0.4f );		// sleep
	m_sprHowToPlay.BeginTweening( 0.6f, Actor::TWEEN_DECELERATE );
	m_sprHowToPlay.SetX( CENTER_X );

	SOUNDMAN->PlayMusic( THEME->GetPathToS("ScreenInstructions music") );
}

ScreenInstructions::~ScreenInstructions()
{
	LOG->Trace( "ScreenInstructions::~ScreenInstructions()" );
}

void ScreenInstructions::Update( float fDeltaTime )
{
	//LOG->Trace( "ScreenInstructions::Update(%f)", fDeltaTime );

	Screen::Update( fDeltaTime );
}

void ScreenInstructions::DrawPrimitives()
{
	m_Menu.DrawBottomLayer();
	Screen::DrawPrimitives();
	m_Menu.DrawTopLayer();
}

void ScreenInstructions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( m_Menu.IsTransitioning() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenInstructions::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_MenuTimer:
		this->MenuStart(PLAYER_1);
		break;
	case SM_GoToPrevScreen:
		SCREENMAN->SetNewScreen( "ScreenTitleMenu" );		
		break;
	case SM_GoToNextScreen:
		SCREENMAN->SetNewScreen( NEXT_SCREEN(GAMESTATE->m_PlayMode) );
		break;
	}
}

void ScreenInstructions::MenuBack( PlayerNumber pn )
{
	m_Menu.Back( SM_GoToPrevScreen );
}

void ScreenInstructions::MenuStart( PlayerNumber pn )
{
	m_Menu.StartTransitioning( SM_GoToNextScreen );

	m_sprHowToPlay.StopTweening();
	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_ACCELERATE );
	m_sprHowToPlay.SetX( SCREEN_RIGHT+m_sprHowToPlay.GetUnzoomedWidth()/2 );
}
