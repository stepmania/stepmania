#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayOni

 Desc: A graphic displayed in the ScoreDisplayOni during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScoreDisplayOni.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "PrefsManager.h"
#include "RageLog.h"
#include "GameState.h"
#include "ThemeManager.h"
#include "StageStats.h"


ScoreDisplayOni::ScoreDisplayOni()
{
	LOG->Trace( "ScoreDisplayOni::ScoreDisplayOni()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayOni frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromNumbers( THEME->GetPathToN("ScoreDisplayOni") );
	m_text.SetShadowLength( 0 );
	this->AddChild( &m_text );
}

void ScoreDisplayOni::Init( PlayerNumber pn ) 
{
	ScoreDisplay::Init( pn );
	m_text.SetDiffuse( PlayerToColor(pn) );
}


void ScoreDisplayOni::Update( float fDelta )
{
	ScoreDisplay::Update( fDelta );

	float fSecsIntoPlay = 0;
	if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		fSecsIntoPlay = g_CurStageStats.fAliveSeconds[m_PlayerNumber];

	m_text.SetText( SecondsToMMSSMsMs(fSecsIntoPlay) );
}
