#include "global.h"
#include "ScoreDisplayBattle.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "RageTextureManager.h"

#define ITEM_X( i )				THEME->GetMetricF("ScoreDisplayBattle",ssprintf("Item%dX",i+1))
#define ITEM_Y( i )				THEME->GetMetricF("ScoreDisplayBattle",ssprintf("Item%dY",i+1))

ScoreDisplayBattle::ScoreDisplayBattle()
{
	LOG->Trace( "ScoreDisplayBattle::ScoreDisplayBattle()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayBattle frame") );
	this->AddChild( &m_sprFrame );

	for( int i=0; i<NUM_INVENTORY_SLOTS; i++ )
	{
		m_ItemIcon[i].SetXY( ITEM_X(i), ITEM_Y(i) );
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
		Attack attack = GAMESTATE->m_Inventory[m_PlayerNumber][s];
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

/*
 * (c) 2001-2003 Chris Danford
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
