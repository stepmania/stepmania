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
#include "RageSounds.h"
#include "ScreenManager.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "PrefsManager.h"


#define HELP_TEXT		THEME->GetMetric (m_sName,"HelpText")
#define TIMER_SECONDS	THEME->GetMetricI(m_sName,"TimerSeconds")
#define NEXT_SCREEN		THEME->GetMetric (m_sName,"NextScreen")
#define PREV_SCREEN		THEME->GetMetric (m_sName,"PrevScreen")


ScreenInstructions::ScreenInstructions( CString sName ) : ScreenWithMenuElements( sName )
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
		FOREACH_HumanPlayer(p)
			easiestDifficulty = min( easiestDifficulty, GAMESTATE->m_PreferredDifficulty[p] );

		if( easiestDifficulty > DIFFICULTY_EASY )
		{
			HandleScreenMessage( SM_GoToNextScreen );
			return;
		}
	}

	CString sHowToPlayPath;
	if( GAMESTATE->m_PlayMode != PLAY_MODE_INVALID )
		sHowToPlayPath = THEME->GetPathG(m_sName,PlayModeToString(GAMESTATE->m_PlayMode)) ;
	else
		RageException::Throw( "The PlayMode has not been set.  A theme must set the PlayMode before showing ScreenInstructions." );

	m_sprHowToPlay.Load( sHowToPlayPath );
	m_sprHowToPlay.SetXY( CENTER_X, CENTER_Y );
	this->AddChild( &m_sprHowToPlay );

	m_sprHowToPlay.SetX( SCREEN_LEFT-SCREEN_WIDTH );
	m_sprHowToPlay.BeginTweening( 0.4f );		// sleep
	m_sprHowToPlay.BeginTweening( 0.6f, Actor::TWEEN_DECELERATE );
	m_sprHowToPlay.SetX( CENTER_X );

	this->SortByDrawOrder();

	SOUND->PlayMusic( THEME->GetPathS(m_sName,"music") );
}

void ScreenInstructions::Input( const DeviceInput& DeviceI, const InputEventType type, const GameInput &GameI, const MenuInput &MenuI, const StyleInput &StyleI )
{
	if( IsTransitioning() )
		return;

	// default input handler
	Screen::Input( DeviceI, type, GameI, MenuI, StyleI );
}

void ScreenInstructions::MenuBack( PlayerNumber pn )
{
	Back( SM_GoToPrevScreen );
}

void ScreenInstructions::MenuStart( PlayerNumber pn )
{
	StartTransitioning( SM_GoToNextScreen );

	m_sprHowToPlay.StopTweening();
	m_sprHowToPlay.BeginTweening( 0.3f, Actor::TWEEN_ACCELERATE );
	m_sprHowToPlay.SetX( SCREEN_RIGHT+m_sprHowToPlay.GetUnzoomedWidth()/2 );
}
