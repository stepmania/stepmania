#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: Judgement.h
//
// Desc: Feedback about the last step that appears in the middle of a player's stream of arrows.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------

#include "RageUtil.h"

#include "Judgement.h"


#define CENTER_X	320
#define CENTER_Y	240

#define JUDGEMENT_DISPLAY_TIME	1.0f
#define JUDGEMENT_SPRITE		"Sprites\\Judgement.sprite"
#define JUDGEMENT_Y				CENTER_Y



Judgement::Judgement() :
  m_fDisplayTimeLeft( 0.0 )
{
	m_sprJudgement.LoadFromSpriteFile( JUDGEMENT_SPRITE );

	SetX( CENTER_X );
}

Judgement::~Judgement()
{

}

void Judgement::SetX( int iNewX )
{
	m_sprJudgement.SetXY(  iNewX,  CENTER_Y );
}

void Judgement::Update( const FLOAT &fDeltaTime )
{
	if( m_fDisplayTimeLeft > 0.0 )
		m_fDisplayTimeLeft -= fDeltaTime;
	m_sprJudgement.Update( fDeltaTime );
}

void Judgement::Draw()
{
//	RageLog( "Judgement::Draw()" );

	if( m_fDisplayTimeLeft > 0.0 )
		m_sprJudgement.Draw();
}


void Judgement::Perfect()
{
	RageLog( "Judgement::Perfect()" );

	m_sprJudgement.SetState( 0 );

	TweenFromBigToSmall();
}

void Judgement::Great()
{
	m_sprJudgement.SetState( 1 );

	TweenFromBigToSmall();
}

void Judgement::Good()
{
	m_sprJudgement.SetState( 2 );

	TweenFromBigToSmall();
}

void Judgement::Boo()
{
	m_sprJudgement.SetState( 3 );

	TweenFromBigToSmall();
}

void Judgement::Miss()
{
	m_sprJudgement.SetState( 4 );

	TweenFromBigToSmall();
}


void Judgement::TweenFromBigToSmall()
{
	m_fDisplayTimeLeft = JUDGEMENT_DISPLAY_TIME;

	m_sprJudgement.SetZoom( 1.5f );
	m_sprJudgement.TweenTo( JUDGEMENT_DISPLAY_TIME/2.0,
							m_sprJudgement.GetX(), 
							m_sprJudgement.GetY()  );
}
