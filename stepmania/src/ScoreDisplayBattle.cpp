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
#include "RageTextureManager.h"

ScoreDisplayBattle::ScoreDisplayBattle()
{
	LOG->Trace( "ScoreDisplayBattle::ScoreDisplayBattle()" );

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
	{
		float fX = (float)SCALE(i,0.f,2.f,-60.f,60.f);

		m_ItemFrame[i].Load( THEME->GetPathToG("ScoreDisplayBattle frames") );
		m_ItemFrame[i].SetX( fX );
		m_ItemFrame[i].StopAnimating();
		m_ItemFrame[i].SetState( i );
		this->AddChild( &m_ItemFrame[i] );

		m_ItemIcon[i].SetX( fX );
		m_ItemIcon[i].StopAnimating();
		this->AddChild( &m_ItemIcon[i] );
	}

	CStringArray asIconPaths;
	GetDirListing( THEME->GetCurThemeDir()+"Graphic/ScoreDisplayBattle icon*.*", asIconPaths );
	for( unsigned j=0; j<asIconPaths.size(); j++ )
		TEXTUREMAN->CacheTexture( asIconPaths[j] );
}

void ScoreDisplayBattle::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );
}

void ScoreDisplayBattle::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	for( int s=0; s<NUM_INVENTORY_SLOTS; s++ )
	{
		GameState::Attack attack = GAMESTATE->m_Inventory[m_PlayerNumber][s];
		CString sNewModifier = attack.sModifier;

		if( sNewModifier != m_iLastSeenInventory[s] )
		{
			m_iLastSeenInventory[s] = sNewModifier;

			if( sNewModifier == "" )
				m_ItemIcon[s].Command( "linear,0.25;zoom,0" );
			else
			{
				// TODO:  Cache all of the icon graphics so we don't load them dynamically from disk.
				m_ItemIcon[s].Load( THEME->GetPathToG("ScoreDisplayBattle icon "+sNewModifier) );
				m_ItemIcon[s].StopTweening();
				m_ItemIcon[s].Command( "diffuse,1,1,1,1;zoom,1;"
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
