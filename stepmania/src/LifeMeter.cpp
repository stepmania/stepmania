#include "stdafx.h"
//-----------------------------------------------------------------------------
// File: LifeMeter.cpp
//
// Desc: LifeMeter counter that displays while dancing.
//
// Copyright (c) 2001 Chris Danford.  All rights reserved.
//-----------------------------------------------------------------------------


#include "RageUtil.h"

#include "LifeMeter.h"


#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define CENTER_X		320
#define CENTER_Y		240


#define FONT		"Fonts\\Font - Arial Bold numbers 30px.font"

#define NUM_PILLS		17

//#define LIFEMETER_TWEEN_TIME	0.5f
#define LIFEMETER_FRAME_SPRITE	"Sprites\\Life Meter Frame.sprite"
#define LIFEMETER_PILLS_SPRITE	"Sprites\\Life Meter Pills.sprite"
#define LIFEMETER_Y				30
#define LIFEMETER_PILLS_Y		(LIFEMETER_Y+2)

const FLOAT PILL_OFFSET[NUM_PILLS] = {
	0.3f, 0.7f, 1.0f, 0.7f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
};


LifeMeter::LifeMeter() :
  m_fLifePercentage( 0.5f )
{
	m_sprLifeMeterFrame.LoadFromSpriteFile( LIFEMETER_FRAME_SPRITE );
	m_sprLifeMeterPills.LoadFromSpriteFile( LIFEMETER_PILLS_SPRITE );

	SetX( CENTER_X );	
}

LifeMeter::~LifeMeter()
{

}


void LifeMeter::SetX( FLOAT iNewX )
{
	m_sprLifeMeterFrame.SetXY( iNewX, LIFEMETER_Y );
	m_sprLifeMeterPills.SetXY( iNewX, LIFEMETER_PILLS_Y );
}

void LifeMeter::Update( const FLOAT &fDeltaTime )
{
	m_sprLifeMeterFrame.Update( fDeltaTime );
	m_sprLifeMeterPills.Update( fDeltaTime );
}

void LifeMeter::Draw( FLOAT fSongBeat )
{
	FLOAT fBeatPercentage = fSongBeat - (int)fSongBeat;
	int iOffsetStart = roundf( NUM_PILLS*fBeatPercentage );

	m_sprLifeMeterFrame.Draw();

	FLOAT iX = m_sprLifeMeterFrame.GetLeftEdge() + 27;
	int iNumPills = (int)(m_sprLifeMeterPills.GetNumStates() * m_fLifePercentage);
	int iPillWidth = m_sprLifeMeterPills.GetZoomedWidth();

	for( int i=0; i<iNumPills; i++ )
	{
		m_sprLifeMeterPills.SetState( i );
		m_sprLifeMeterPills.SetX( iX );

		int iOffsetNum = (iOffsetStart - i + NUM_PILLS) % NUM_PILLS;
		int iOffset = roundf( PILL_OFFSET[iOffsetNum] * m_fLifePercentage * 8.0f );
		m_sprLifeMeterPills.SetY( LIFEMETER_PILLS_Y - iOffset );

		m_sprLifeMeterPills.Draw();

		iX += iPillWidth;
	}
}

void LifeMeter::SetLife( FLOAT fNewLife )
{
	assert( fNewLife >= 0.0f  &&  fNewLife <= 1.0f );
	m_fLifePercentage = fNewLife;
	
	if( fNewLife >= 0.9f )
		m_sprLifeMeterFrame.SetEffectCamelion( 5, D3DXCOLOR(0.2f,0.2f,0.2f,1), D3DXCOLOR(1,1,1,1) );
	else if( fNewLife < 0.25f )
		m_sprLifeMeterFrame.SetEffectCamelion( 5, D3DXCOLOR(1,0.8f,0.8f,1), D3DXCOLOR(1,0.2f,0.2f,1) );
	else
		m_sprLifeMeterFrame.SetEffectNone();

}
