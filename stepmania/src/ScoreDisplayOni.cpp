#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScoreDisplayOni.h

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


const float SCORE_TWEEN_TIME = 0.5f;


ScoreDisplayOni::ScoreDisplayOni()
{
	LOG->Trace( "ScoreDisplayOni::ScoreDisplayOni()" );

	// init the text
	LoadFromFont( THEME->GetPathTo("Fonts","score numbers") );
	TurnShadowOff();
}


void ScoreDisplayOni::Init( PlayerNumber pn )
{
	m_PlayerNumber = pn;
}

void ScoreDisplayOni::SetScore( float fNewScore ) 
{ 
}

void ScoreDisplayOni::Update( float fDeltaTime )
{
	BitmapText::Update( fDeltaTime );
}

void ScoreDisplayOni::Draw()
{
	float fSecsIntoPlay;
	if( GAMESTATE->IsPlayerEnabled(m_PlayerNumber) )
		fSecsIntoPlay = GAMESTATE->GetPlayerSurviveTime(m_PlayerNumber);
	else
		fSecsIntoPlay = 0;

	SetText( SecondsToTime(fSecsIntoPlay) );

	BitmapText::Draw();
}
