#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: StageBox

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "StageBox.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageLog.h"

const float NUMBER_X = 16;
const float NUMBER_Y = 10;

const float ST_X = NUMBER_X+4;
const float ST_Y = NUMBER_Y;


StageBox::StageBox()
{
	this->AddSubActor( &m_sprBox );
	
	m_textNumber.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textNumber.TurnShadowOff();
	m_textNumber.SetHorizAlign( Actor::align_right );
	m_textNumber.SetVertAlign( Actor::align_top );
	m_textNumber.SetZoom( 2.0f );
	m_textNumber.SetXY( NUMBER_X, NUMBER_Y );
	this->AddSubActor( &m_textNumber );

	m_textST.LoadFromFont( THEME->GetPathTo("Fonts","header1") );
	m_textST.TurnShadowOff();
	m_textST.SetHorizAlign( Actor::align_left );
	m_textST.SetVertAlign( Actor::align_top );
	m_textST.SetZoom( 1 );
	m_textST.SetXY( ST_X, ST_Y );
	this->AddSubActor( &m_textST );
}

void StageBox::SetStageInfo( PlayerNumber p, int iNumStagesCompleted )
{
	m_sprBox.Load( THEME->GetPathTo("Graphics",p==PLAYER_1?"evaluation stage frame p1":"evaluation stage frame p2") );
	m_sprBox.StopAnimating();

	m_textNumber.SetText( ssprintf("%2d", iNumStagesCompleted) );

	m_textST.SetText( "ST." );
}
