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
	LOG->WriteLine( "ScoreDisplayOni::ScoreDisplayOni()" );

	// init the text
	Load( THEME->GetPathTo(FONT_SCORE_NUMBERS) );
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
	float fSecsIntoPlay = GAMESTATE->GetPlayerSurviveTime(m_PlayerNumber);

	int iMinsDisplay = (int)fSecsIntoPlay/60;
	int iSecsDisplay = (int)fSecsIntoPlay - iMinsDisplay*60;
	int iLeftoverDisplay = int( (fSecsIntoPlay - iMinsDisplay*60 - iSecsDisplay) * 100 );
	SetText( ssprintf( "%02d:%02d:%02d", iMinsDisplay, iSecsDisplay, iLeftoverDisplay ) );

	BitmapText::Draw();
}
