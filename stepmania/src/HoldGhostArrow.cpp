#include "stdafx.h"
//
// HoldGhostArrow.cpp: implementation of the GhostArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "HoldGhostArrow.h"


const CString HOLD_GHOST_ARROW_SPRITE = "Sprites\\Hold Ghost Arrow.sprite";
const float  HOLD_GHOST_ARROW_TWEEN_TIME = 0.5f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HoldGhostArrow::HoldGhostArrow()
{
	m_bWasSteppedOnLastFrame = false;
	m_fHeatLevel = 0;

	LoadFromSpriteFile( HOLD_GHOST_ARROW_SPRITE );
	SetDiffuseColor( D3DXCOLOR(1,1,1,1) );
	SetZoom( 1.1f );
}

void HoldGhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );

	if( m_bWasSteppedOnLastFrame )
	{
		m_fHeatLevel += fDeltaTime * 4;
		if( m_fHeatLevel >= 1 )
			m_fHeatLevel = 1;
	}
	else
	{
		m_fHeatLevel -= fDeltaTime * 4;
		if( m_fHeatLevel < 0 )
			m_fHeatLevel = 0;
	}

	int iStateNum = min( m_fHeatLevel * GetNumStates(), GetNumStates()-1 );
	SetState( iStateNum );
	
	SetDiffuseColor( D3DXCOLOR(1,1,1,m_fHeatLevel*3) );

	m_bWasSteppedOnLastFrame = false;	// reset for next frame
}

void HoldGhostArrow::SetBeat( const float fSongBeat )
{
	//SetState( fmod(fSongBeat,1)<0.25 ? 1 : 0 );
}

void HoldGhostArrow::Step()
{
	m_bWasSteppedOnLastFrame = true;
}
