#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: LifeMeterBar

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "LifeMeterBar.h"
#include "PrefsManager.h"

const int NUM_LIFE_STREAM_SECTIONS = 3;
const float LIFE_STREAM_SECTION_WIDTH = 1.0f/NUM_LIFE_STREAM_SECTIONS;

const float METER_WIDTH = 258;
const float METER_HEIGHT = 20;

const float DANGER_THRESHOLD = 0.4f;
const float FAIL_THRESHOLD = 0;




LifeMeterBar::LifeMeterBar()
{
	m_fLifePercentage = 0.5f;
	m_fTrailingLifePercentage = 0;
	m_fLifeVelocity = 0;
	m_fHotAlpha = 0;
	m_bFailedEarlier = false;

	m_frame;

	m_quadBlackBackground.SetDiffuseColor( D3DXCOLOR(0,0,0,1) );
	m_quadBlackBackground.SetZoomX( METER_WIDTH );
	m_quadBlackBackground.SetZoomY( METER_HEIGHT );
	m_frame.AddSubActor( &m_quadBlackBackground );

	m_sprStreamNormal.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_LIFEMETER_STREAM_NORMAL) );
	m_frame.AddSubActor( &m_sprStreamNormal );

	m_sprStreamHot.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_LIFEMETER_STREAM_HOT) );
	m_frame.AddSubActor( &m_sprStreamHot );

	m_sprFrame.Load( THEME->GetPathTo(GRAPHIC_GAMEPLAY_LIFEMETER_BAR) );
	m_frame.AddSubActor( &m_sprFrame );

	this->AddSubActor( &m_frame );

	ResetBarVelocity();
}

void LifeMeterBar::Load( PlayerNumber p, const PlayerOptions &po )
{
	LifeMeter::Load( p, po );

	if( p == PLAYER_2 )
		m_frame.SetZoomX( -1 );
}

void LifeMeterBar::ChangeLife( TapNoteScore score )
{
	bool bWasHot = IsHot();

	float fDeltaLife;
	switch( m_PlayerOptions.m_DrainType )
	{
	case PlayerOptions::DRAIN_NORMAL:
		switch( score )
		{
		case TNS_PERFECT:	fDeltaLife = +0.008f;	break;
		case TNS_GREAT:		fDeltaLife = +0.004f;	break;
		case TNS_GOOD:		fDeltaLife = +0.000f;	break;
		case TNS_BOO:		fDeltaLife = -0.040f;	break;
		case TNS_MISS:		fDeltaLife = -0.080f;	break;
		}
		if( IsHot()  &&  score < TNS_GOOD )
			fDeltaLife = -0.10f;		// make it take a while to get back to "doing great"
		break;
	case PlayerOptions::DRAIN_NO_RECOVER:
		switch( score )
		{
		case TNS_PERFECT:	fDeltaLife = +0.000f;	break;
		case TNS_GREAT:		fDeltaLife = +0.000f;	break;
		case TNS_GOOD:		fDeltaLife = +0.000f;	break;
		case TNS_BOO:		fDeltaLife = -0.040f;	break;
		case TNS_MISS:		fDeltaLife = -0.080f;	break;
		}
		break;
	case PlayerOptions::DRAIN_SUDDEN_DEATH:
		fDeltaLife = -1;
		break;
	}


	m_fLifePercentage += fDeltaLife;
	CLAMP( m_fLifePercentage, 0, 1 );

	if( m_fLifePercentage < FAIL_THRESHOLD )
		m_bFailedEarlier = true;

	ResetBarVelocity();
}

void LifeMeterBar::ResetBarVelocity()
{
	// update bar animation
	const float fDelta = m_fLifePercentage - m_fTrailingLifePercentage;

	m_fLifeVelocity = fDelta * 5;	// change in life percent per second
}

bool LifeMeterBar::IsHot() 
{ 
	return m_fLifePercentage >= 1; 
}

bool LifeMeterBar::IsInDanger() 
{ 
	return m_fLifePercentage < DANGER_THRESHOLD; 
}

bool LifeMeterBar::IsFailing() 
{ 
	return m_fLifePercentage <= 0; 
}

bool LifeMeterBar::FailedEarlier() 
{ 
	return m_bFailedEarlier; 
}

void LifeMeterBar::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	float fDelta = m_fLifePercentage - m_fTrailingLifePercentage;
	m_fLifeVelocity += fDelta * fDeltaTime;		// accelerate
	m_fLifeVelocity *= 1-fDeltaTime;		// dampen
	m_fTrailingLifePercentage += m_fLifeVelocity * fDeltaTime;

	float fNewDelta = m_fLifePercentage - m_fTrailingLifePercentage;

	if( fDelta * fNewDelta < 0 )	// the deltas have different signs
		m_fLifeVelocity /= 4;		// make some drag
	CLAMP( m_fTrailingLifePercentage, 0, 1 );


	// set custom texture coords
	CRect rectSize( 
		int( -METER_WIDTH/2 ), 
		int( -METER_HEIGHT/2 ), 
		int( -METER_WIDTH/2  + METER_WIDTH  * m_fTrailingLifePercentage ), 
		int( -METER_HEIGHT/2 + METER_HEIGHT) );

	float fPrecentOffset = TIMER->GetTimeSinceStart();
	fPrecentOffset -= (int)fPrecentOffset;

	FRECT frectCustomTexCoords(
		0 - fPrecentOffset,
		0,
		m_fTrailingLifePercentage - fPrecentOffset,
		1 );

	m_sprStreamNormal.StretchTo( rectSize );
	m_sprStreamNormal.SetCustomTextureRect( frectCustomTexCoords );
	m_sprStreamHot.StretchTo( rectSize );
	m_sprStreamHot.SetCustomTextureRect( frectCustomTexCoords );

	m_fHotAlpha  += IsHot() ? +fDeltaTime*2 : -fDeltaTime*2;
	CLAMP( m_fHotAlpha, 0, 1 );
	m_sprStreamHot.SetDiffuseColor(    D3DXCOLOR(1,1,1,m_fHotAlpha) );
}

void LifeMeterBar::DrawPrimitives()
{
	float fPercentRed = IsInDanger() ? sinf( TIMER->GetTimeSinceStart()*D3DX_PI*4 )/2+0.5f : 0;
	m_quadBlackBackground.SetDiffuseColor( D3DXCOLOR(fPercentRed*0.8f,0,0,1) );

	ActorFrame::DrawPrimitives();
}

/*

  Chris:  
  I'm making the coloring of the lifemeter a property of the theme.  
  That's where it belongs anyway...

  
const D3DXCOLOR COLOR_EZ2NORMAL_1	= D3DXCOLOR(0.7f,0.4f,0,1);
const D3DXCOLOR COLOR_EZ2NORMAL_2	= D3DXCOLOR(0.8f,0.4f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFULL_1	= D3DXCOLOR(0.7f,0.6f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFULL_2	= D3DXCOLOR(0.8f,0.7f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFAIL_1	= D3DXCOLOR(0.9f,0.0f,0,1);
const D3DXCOLOR COLOR_EZ2NEARFAIL_2	= D3DXCOLOR(0.8f,0.1f,0,1);
const D3DXCOLOR COLOR_EZ2FULL_1	= D3DXCOLOR(0.3f,0.9f,0.4f,1);
const D3DXCOLOR COLOR_EZ2FULL_2	= D3DXCOLOR(0.2f,0.7f,0.3f,1);
const D3DXCOLOR COLOR_NORMAL_1	= D3DXCOLOR(1,1,1,1);
const D3DXCOLOR COLOR_NORMAL_2	= D3DXCOLOR(0,1,0,1);
const D3DXCOLOR COLOR_FULL_1	= D3DXCOLOR(1,0,0,1);
const D3DXCOLOR COLOR_FULL_2	= D3DXCOLOR(1,1,0,1);

D3DXCOLOR LifeStream::GetColor( float fPercentIntoSection )
{
	float fPercentColor1 = fabsf( fPercentIntoSection*2 - 1 );
	fPercentColor1 *= fPercentColor1 * fPercentColor1 * fPercentColor1;	// make the color bunch around one side
	if (GAMESTATE->m_CurGame != GAME_EZ2)
	{
		if( m_fLifePercentage == 1 )
			return COLOR_FULL_1 * fPercentColor1 + COLOR_FULL_2 * (1-fPercentColor1);
		else
			return COLOR_NORMAL_1 * fPercentColor1 + COLOR_NORMAL_2 * (1-fPercentColor1);
	}
	else
	{
		if( m_fLifePercentage == 1 )
			return COLOR_EZ2FULL_1 * fPercentColor1 + COLOR_EZ2FULL_2 * (1-fPercentColor1);
		else if ( m_fLifePercentage > 0.60 )
			return COLOR_EZ2NEARFULL_1 * fPercentColor1 + COLOR_EZ2NEARFULL_2 * (1-fPercentColor1);
		else if ( m_fLifePercentage < 0.25 )
			return COLOR_EZ2NEARFAIL_1 * fPercentColor1 + COLOR_EZ2NEARFAIL_2 * (1-fPercentColor1);
		else
			return COLOR_EZ2NORMAL_1 * fPercentColor1 + COLOR_EZ2NORMAL_2 * (1-fPercentColor1);
	}
}

void LifeStream::DrawPrimitives()
{
	// make the object in logical units centered at the origin
	LPDIRECT3DVERTEXBUFFER8 pVB = DISPLAY->GetVertexBuffer();
	RAGEVERTEX* v;
	pVB->Lock( 0, 0, (BYTE**)&v, 0 );

	int iNumV = 0;

	float fPercentIntoSection = (TIMER->GetTimeSinceStart()/0.3f* (m_bIsHot ? 2 : 1) )*LIFE_STREAM_SECTION_WIDTH;
	fPercentIntoSection -= (int)fPercentIntoSection;
	fPercentIntoSection = 1-fPercentIntoSection;
	fPercentIntoSection -= (int)fPercentIntoSection;
	ASSERT( fPercentIntoSection >= 0  &&  fPercentIntoSection < 1 );
	float fX = - 0.5f; 


	while( fX < m_fTrailingLifePercentage-0.5f )
	{
		//
		// draw middle
		//
		v[iNumV++].p = D3DXVECTOR3( fX, -0.5f, 0 );
		v[iNumV++].p = D3DXVECTOR3( fX,  0.5f, 0 );
		
		iNumV -= 2;
		const D3DXCOLOR color = GetColor( fPercentIntoSection ); 
		v[iNumV++].color = color;
		v[iNumV++].color = color;


		if( fPercentIntoSection < 0.5f )
		{
			const float fPercentToAdd = 0.5f-fPercentIntoSection;
			fPercentIntoSection += fPercentToAdd;
			fX += fPercentToAdd*LIFE_STREAM_SECTION_WIDTH;
		}
		else if( fPercentIntoSection < 1.0f )
		{
			const float fPercentToAdd = 1.0f-fPercentIntoSection;
			fPercentIntoSection = 0;
			fX += fPercentToAdd*LIFE_STREAM_SECTION_WIDTH;
		}
		else
			ASSERT( false );
	}

	const float fXToSubtract = fX - (m_fTrailingLifePercentage-0.5f);
	fX -= fXToSubtract;
	fPercentIntoSection -= fXToSubtract/LIFE_STREAM_SECTION_WIDTH;
	if( fPercentIntoSection < 0 )
		fPercentIntoSection += 1;

	//
	// draw right edge
	//
	v[iNumV++].p = D3DXVECTOR3( fX, -0.5f, 0 );
	v[iNumV++].p = D3DXVECTOR3( fX,  0.5f, 0 );
	
	iNumV -= 2;
	const D3DXCOLOR color = GetColor( fPercentIntoSection ); 
	v[iNumV++].color = color;
	v[iNumV++].color = color; 



	pVB->Unlock();


	LPDIRECT3DDEVICE8 pd3dDevice = DISPLAY->GetDevice();
	pd3dDevice->SetTexture( 0, NULL );



	// set texture and alpha properties
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );

	pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );


	// finally!  Pump those triangles!	
	pd3dDevice->SetVertexShader( D3DFVF_RAGEVERTEX );
	pd3dDevice->SetStreamSource( 0, pVB, sizeof(RAGEVERTEX) );
	pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, iNumV-2 );

}
*/