#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayBattle.h

 Desc: A graphic displayed in the ScoreDisplayBattle during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayBattle.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"



ScoreDisplayBattle::ScoreDisplayBattle()
{
	LOG->Trace( "ScoreDisplayBattle::ScoreDisplayBattle()" );

	for( int i=0; i<NUM_ITEM_SLOTS; i++ )
	{
		float fX = (float)SCALE(i,0.f,2.f,-60.f,60.f);

		m_sprFrames[i].Load( THEME->GetPathTo("Graphics","gameplay battle item frames 3x1") );
		m_sprFrames[i].SetX( fX );
		m_sprFrames[i].StopAnimating();
		m_sprFrames[i].SetState( i );
		this->AddChild( &m_sprFrames[i] );

		m_sprItems[i].SetX( fX );
		m_sprItems[i].Load( THEME->GetPathTo("Graphics","gameplay battle item icons") );
		m_sprItems[i].StopAnimating();
		m_sprItems[i].SetZoom( 0 );
		this->AddChild( &m_sprItems[i] );

        m_iLastSeenItems[i] = ITEM_NONE;
	}
}

void ScoreDisplayBattle::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	for( int s=0; s<NUM_ITEM_SLOTS; s++ )
	{
		int iNewItem = GAMESTATE->m_iItems[m_PlayerNumber][s];
		int iOldItem = m_iLastSeenItems[s];

		m_iLastSeenItems[s] = iNewItem;

		if( iNewItem != iOldItem )	// there was a change of items
		{
			if( iNewItem == ITEM_NONE )
			{
				m_sprItems[s].StopTweening();
				m_sprItems[s].BeginTweening( 0.25f, Actor::TWEEN_BOUNCE_BEGIN );
				m_sprItems[s].SetTweenZoom( 0 );
			}
			else
			{
				m_sprItems[s].SetDiffuse( RageColor(1,1,1,1) );
				m_sprItems[s].SetState( iNewItem );
				m_sprItems[s].SetZoom( 1 );
				
				// blink
				m_sprItems[s].StopTweening();
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,1) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,0) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,1) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,0) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,1) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,0) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,1) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,0) );
				m_sprItems[s].BeginTweening( 0.1f );	// sleep
				m_sprItems[s].BeginTweening( 0.001f );	// snap
				m_sprItems[s].SetTweenDiffuse( RageColor(1,1,1,1) );
			}
		}
	}	
}
