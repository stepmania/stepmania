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

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
	{
		float fX = (float)SCALE(i,0.f,2.f,-60.f,60.f);

		m_Inventory[i].SetX( fX );
		this->AddChild( &m_Inventory[i] );
	}
}

void ScoreDisplayBattle::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
		m_Inventory[i].Load( m_PlayerNumber, "" );
}

void ScoreDisplayBattle::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
	{
		CString sNewItem = GAMESTATE->m_sInventory[m_PlayerNumber][s];

		if( sNewItem != m_iLastSeenInventory[s] )
		{
			m_iLastSeenInventory[s] = sNewItem;

			m_Inventory[s].Load( m_PlayerNumber, sNewItem );

			if( sNewItem == "" )
				m_Inventory[s].Command( "linear,0.25;zoom,0" );
			else
			{
				m_Inventory[s].StopTweening();
				m_Inventory[s].Command( "diffuse,1,1,1,1;zoom,1;"
					"sleep,0.1;linear,0;diffusealpha,0;"
					"sleep,0.1;linear,0;diffusealpha,1;"
					"sleep,0.1;linear,0;diffusealpha,0;"
					"sleep,0.1;linear,0;diffusealpha,1;"
					"sleep,0.1;linear,0;diffusealpha,0;"
					"sleep,0.1;linear,0;diffusealpha,1;" );
			}
		}
	}	
}
