#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: CombinedLifeMeterEnemy

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "CombinedLifeMeterEnemy.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "MusicWheel.h"
#include "CombinedLifeMeterEnemy.h"
#include <math.h>
#include "ThemeManager.h"
#include "GameState.h"
#include "CombinedLifeMeterEnemy.h"
#include "ThemeManager.h"
#include "GameState.h"
#include "ScreenManager.h"
#include "ScreenGameplay.h"


const float SECONDS_TO_SHOW_FACE = 1.5f;


#define FACE_X	THEME->GetMetricF("CombinedLifeMeterEnemy","FaceX")
#define FACE_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","FaceY")
#define HEALTH_X	THEME->GetMetricF("CombinedLifeMeterEnemy","HealthX")
#define HEALTH_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","HealthY")
#define FRAME_X	THEME->GetMetricF("CombinedLifeMeterEnemy","FrameX")
#define FRAME_Y	THEME->GetMetricF("CombinedLifeMeterEnemy","FrameY")


CombinedLifeMeterEnemy::CombinedLifeMeterEnemy()
{
	m_sprFace.Load( THEME->GetPathToG("CombinedLifeMeterEnemy face 2x3") );
	ASSERT( m_sprFace.GetNumStates() >= NUM_FACES );
	m_sprFace.StopAnimating();
	m_sprFace.SetXY( FACE_X, FACE_Y );
	this->AddChild( &m_sprFace );

	m_sprHealthBackground.Load( THEME->GetPathToG("CombinedLifeMeterEnemy health background") );
	m_sprHealthBackground.SetXY( HEALTH_X, HEALTH_Y );
	this->AddChild( &m_sprHealthBackground );

	m_sprHealthStream.Load( THEME->GetPathToG("CombinedLifeMeterEnemy health stream") );
	m_sprHealthStream.SetXY( HEALTH_X, HEALTH_Y );
	m_sprHealthStream.SetTexCoordVelocity(-0.2f,0.5f);
	this->AddChild( &m_sprHealthStream );

	m_fLastSeenHealthPercent = -1;

	m_sprFrame.Load( THEME->GetPathToG("CombinedLifeMeterEnemy frame") );
	m_sprFrame.SetName( "Frame" );
	m_sprFrame.SetXY( FRAME_X, FRAME_Y );
	this->AddChild( &m_sprFrame );
}

void CombinedLifeMeterEnemy::Update( float fDelta )
{
	CombinedLifeMeter::Update( fDelta );

	if( m_fSecondsUntilReturnToNormalFace > 0 )
	{
		m_fSecondsUntilReturnToNormalFace -= fDelta;

		if( m_fSecondsUntilReturnToNormalFace < 0 )
		{
			m_fSecondsUntilReturnToNormalFace = 0;
			m_sprFace.SetState( normal );
		}
	}

	if( GAMESTATE->m_fOpponentHealthPercent == 0 )
	{
		m_sprFace.SetState( defeated );
	}

	if( m_fLastSeenHealthPercent != GAMESTATE->m_fOpponentHealthPercent )
	{
		m_sprHealthStream.SetGlow( RageColor(1,1,1,1) );
		m_sprHealthStream.BeginTweening( 0.5f, TWEEN_DECELERATE );
		m_sprHealthStream.SetCropRight( 1-GAMESTATE->m_fOpponentHealthPercent );
		m_sprHealthStream.SetGlow( RageColor(1,1,1,0) );
		m_fLastSeenHealthPercent = GAMESTATE->m_fOpponentHealthPercent;

		SetFace( damage );

		if( GAMESTATE->m_fOpponentHealthPercent == 0 )
		{
			m_sprFrame.BeginTweening( 0.5f );
			m_sprFrame.SetDiffuse( RageColor(0.5f,0.5f,0.5f,1) );
		}
	}



	//
	// launch enemy attacks to human players
	//

	// Don't apply any attacks if the enemy is already defeated
	if( GAMESTATE->m_fOpponentHealthPercent>0 )
	{

		static const CString sPossibleModifiers[NUM_ATTACK_LEVELS][3] = 
		{
			{
				"1.5x",
				"dizzy",
				"drunk"
			},
			{
				"sudden",
				"hidden",
				"wave",
			},
			{
				"expand",
				"tornado",
				"flip"
			}
		};

	#define CROSSED_SONG_SECONDS( s ) ((GAMESTATE->m_fMusicSeconds-fDelta) < s  &&  (GAMESTATE->m_fMusicSeconds) >= s )

		if( CROSSED_SONG_SECONDS(10) || 
			CROSSED_SONG_SECONDS(30) ||
			CROSSED_SONG_SECONDS(50) ||
			CROSSED_SONG_SECONDS(70) ||
			CROSSED_SONG_SECONDS(90) ||
			CROSSED_SONG_SECONDS(110) )
		{
			SetFace( attack );
		}

		if( CROSSED_SONG_SECONDS(20) || CROSSED_SONG_SECONDS(40) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_1;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			SCREENMAN->SendMessageToTopScreen( SM_BattleTrickLevel1 );
			SetFace( attack );
		}
		if( CROSSED_SONG_SECONDS(60) || CROSSED_SONG_SECONDS(80) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_2;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			SCREENMAN->SendMessageToTopScreen( SM_BattleTrickLevel2 );
			SetFace( attack );
		}
		if( CROSSED_SONG_SECONDS(100) )
		{
			GameState::Attack a;
			a.fSecsRemaining = 10;
			a.level = ATTACK_LEVEL_3;
			a.sModifier = sPossibleModifiers[a.level][rand()%3];
			for( int p=0; p<NUM_PLAYERS; p++ )
				if( GAMESTATE->IsHumanPlayer(p) )
					GAMESTATE->LaunchAttack( (PlayerNumber)p, a );
			SCREENMAN->SendMessageToTopScreen( SM_BattleTrickLevel3 );
			SetFace( attack );
		}
	}
}

void CombinedLifeMeterEnemy::SetFace( Face face )
{
	m_sprFace.SetState( face );
	m_fSecondsUntilReturnToNormalFace = SECONDS_TO_SHOW_FACE;
}


void CombinedLifeMeterEnemy::OnTaunt()
{
	SetFace( taunt );
}