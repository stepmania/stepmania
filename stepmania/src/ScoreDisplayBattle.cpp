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
		this->AddChild( &m_sprItems[i] );
	}
}

void ScoreDisplayBattle::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	for( int i=0; i<NUM_ITEM_SLOTS; i++ )
	{
		int item = GAMESTATE->m_Inventory[m_PlayerNumber].m_iItems[i];
		if( item == ITEM_NONE )
			m_sprItems[i].SetDiffuse( RageColor(1,1,1,0) );
		else
		{
			m_sprItems[i].SetDiffuse( RageColor(1,1,1,1) );
			m_sprItems[i].SetState( item );
		}
	}	
}
