#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayRave.h

 Desc: A graphic displayed in the ScoreDisplayRave during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayRave.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"

ScoreDisplayRave::ScoreDisplayRave()
{
	LOG->Trace( "ScoreDisplayRave::ScoreDisplayRave()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayRave frame") );
	this->AddChild( &m_sprFrame );

	m_textCurrentAttack.LoadFromFont( THEME->GetPathToF("Common normal") );
	this->AddChild( &m_textCurrentAttack );
}

void ScoreDisplayRave::Init( PlayerNumber pn )
{
	ScoreDisplay::Init( pn );
}

void ScoreDisplayRave::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	CString sNewAttack = GAMESTATE->m_ActiveAttacks[m_PlayerNumber][0].sModifier;

	if( sNewAttack != m_sLastSeenAttack )
	{
		m_textCurrentAttack.SetText( sNewAttack );
		m_textCurrentAttack.StopTweening();
		m_textCurrentAttack.Command( "diffuse,1,1,1,1;zoom,1;"
			"sleep,0.1;linear,0;diffusealpha,0;"
			"sleep,0.1;linear,0;diffusealpha,1;"
			"sleep,0.1;linear,0;diffusealpha,0;"
			"sleep,0.1;linear,0;diffusealpha,1;"
			"sleep,0.1;linear,0;diffusealpha,0;"
			"sleep,0.1;linear,0;diffusealpha,1;" );
	}
}
