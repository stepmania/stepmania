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


ScoreDisplayOni::ScoreDisplayOni()
{
	LOG->Trace( "ScoreDisplayOni::ScoreDisplayOni()" );

	m_sprFrame.Load( THEME->GetPathToG("ScoreDisplayOni frame") );
	this->AddChild( &m_sprFrame );

	// init the text
	m_text.LoadFromNumbers( THEME->GetPathToN("ScoreDisplayOni") );
	m_text.EnableShadow( false );
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

	float fSecsIntoPlay;
	if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		// cure: multiplied by music rate to adjust timer
		fSecsIntoPlay = GAMESTATE->m_CurStageStats.fAliveSeconds[m_PlayerNumber] * GAMESTATE->m_SongOptions.m_fMusicRate;
	else
		fSecsIntoPlay = 0;

	m_text.SetText( SecondsToTime(fSecsIntoPlay) );
}
