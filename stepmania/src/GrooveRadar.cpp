#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GrooveRadar

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "GrooveRadar.h"
#include "ThemeManager.h"
#include "RageBitmapTexture.h"
#include "GameConstants.h"


GrooveRadar::GrooveRadar()
{
	m_sprRadarBase.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_RADAR_BASE) );
	this->AddActor( &m_sprRadarBase );

	m_sprRadarWords.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_RADAR_WORDS) );
	m_sprRadarWords.SetY( -10 );
	this->AddActor( &m_sprRadarWords );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_bValuesVisible[p] = false;
		m_PercentTowardNew[p] = 0;

		for( int c=0; c<NUM_RADAR_CATEGORIES; c++ )
		{
			m_fValuesNew[p][c] = 0;
			m_fValuesOld[p][c] = 0;
		}
	}
}

void GrooveRadar::SetFromNoteMetadata( PlayerNumber p, NoteMetadata* pNoteMetadata )		// NULL means no song
{
	if( pNoteMetadata != NULL )
	{
		for( int c=0; c<NUM_RADAR_CATEGORIES; c++ )
		{
			const float fValueCurrent = m_fValuesOld[p][c] * (1-m_PercentTowardNew[p]) + m_fValuesNew[p][c] * m_PercentTowardNew[p];
			m_fValuesOld[p][c] = fValueCurrent;
			m_fValuesNew[p][c] = randomf(0.5f,1.1f);
		}

		if( m_bValuesVisible[p] == false )	// the values WERE invisible
			m_PercentTowardNew[p] = 1;
		else
			m_PercentTowardNew[p] = 0;

		m_bValuesVisible[p] = true;
	}
	else	// pNoteMetadata == NULL
	{
		m_bValuesVisible[p] = false;
	}
}

void GrooveRadar::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		m_PercentTowardNew[p] = min( m_PercentTowardNew[p]+4.0f*fDeltaTime, 1 );
	}
}

void GrooveRadar::RenderPrimitives()
{
	ActorFrame::RenderPrimitives();

	// draw radar filling
	const float fRadius = m_sprRadarBase.GetZoomedHeight()/2.0f;
	LPDIRECT3DVERTEXBUFFER8 pVB = SCREEN->GetVertexBuffer();
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();
	pd3dDevice->SetTexture( 0, NULL );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	CUSTOMVERTEX* v;

	for( int p=0; p<NUM_PLAYERS; p++ )
	{
		if( !m_bValuesVisible[p] )
			continue;

		//
		// use a fan to draw the volume
		//
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		D3DXCOLOR color = PLAYER_COLOR[p];
		color.a = 0.5f;
		v[0].p = D3DXVECTOR3( 0, 0, 0 );
		v[0].color = color;

		int i;

		for( i=0; i<NUM_RADAR_CATEGORIES+1; i++ )	// do one extra to close the fan
		{
			const int c = i%NUM_RADAR_CATEGORIES;
			const float fDistFromCenter = 
				( m_fValuesOld[p][c] * (1-m_PercentTowardNew[p]) + m_fValuesNew[p][c] * m_PercentTowardNew[p] ) * fRadius;
			const float fRotation = D3DX_PI/2 + D3DX_PI*2 / 5.0f * i;
			const float fX = cosf(fRotation) * fDistFromCenter;
			const float fY = -sinf(fRotation) * fDistFromCenter;

			v[1+i].p = D3DXVECTOR3( fX, fY,	0 );
			v[1+i].color = color;
		}

		pVB->Unlock();

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, 5 );


		//
		// use a strip to draw the thick line
		//
		pVB->Lock( 0, 0, (BYTE**)&v, 0 );

		for( i=0; i<NUM_RADAR_CATEGORIES+1; i++ )	// do one extra to close the fan
		{
			const int c = i%NUM_RADAR_CATEGORIES;
			const float fDistFromCenter = 
				( m_fValuesOld[p][c] * (1-m_PercentTowardNew[p]) + m_fValuesNew[p][c] * m_PercentTowardNew[p] ) * fRadius;
			const float fDistFromCenterInner = fDistFromCenter-2;
			const float fDistFromCenterOutter = fDistFromCenter+2;
			const float fRotation = D3DX_PI/2 + D3DX_PI*2 / 5.0f * i;
			const float fXInner = cosf(fRotation) * fDistFromCenterInner;
			const float fXOutter = cosf(fRotation) * fDistFromCenterOutter;
			const float fYInner = -sinf(fRotation) * fDistFromCenterInner;
			const float fYOutter = -sinf(fRotation) * fDistFromCenterOutter;

			v[i*2+0].p = D3DXVECTOR3( fXInner, fYInner, 0 );
			v[i*2+1].p = D3DXVECTOR3( fXOutter, fYOutter,	0 );
			v[i*2+0].color = PLAYER_COLOR[p];
			v[i*2+1].color = PLAYER_COLOR[p];
		}

		pVB->Unlock();

		pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 10 );
	}


}

void GrooveRadar::TweenOnScreen()
{
	SetZoom( 0.5f );
	SetRotation( D3DX_PI*4 );
	BeginTweening( 0.6f );
	SetTweenZoom( 1 );
	SetTweenRotationZ( 0 );
}

void GrooveRadar::TweenOffScreen()
{
	BeginTweening( 0.6f );
	SetTweenRotationZ( D3DX_PI*4 );
	SetTweenZoom( 0 );
}
