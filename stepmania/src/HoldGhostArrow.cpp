#include "stdafx.h"
//
// HoldGhostArrow.cpp: implementation of the GhostArrow class.
//
//////////////////////////////////////////////////////////////////////

#include "HoldGhostArrow.h"
#include "ThemeManager.h"


const float  HOLD_GHOST_ARROW_TWEEN_TIME = 0.5f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HoldGhostArrow::HoldGhostArrow()
{
	m_bWasSteppedOnLastFrame = false;
	m_fHeatLevel = 0;

//	LoadFromSpriteFile( THEME->GetPathTo(GRAPHIC_HOLD_GHOST_ARROW) );
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

	int iStateNum = (int)min( m_fHeatLevel * GetNumStates(), GetNumStates()-1 );
	SetState( iStateNum );

	if( m_fHeatLevel == 1 )
	{
		bool bZooomALittle = (GetTickCount() % 50) > 25;
		SetZoom( bZooomALittle ? 1.04f : 1.0f );
	}
	else
		SetZoom( 1 );
	
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
