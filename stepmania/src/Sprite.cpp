#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: Sprite.cpp

 Desc: A bitmap actor that animates and moves around.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#include "Sprite.h"
#include "RageTextureManager.h"
#include "IniFile.h"
#include <assert.h>
#include <math.h>


Sprite::Sprite()
{
	Init();
}

void Sprite::Init()
{
	Actor::Init();

	m_pTexture = NULL;
	m_uNumStates = 0;
	m_uCurState = 0;
	m_bIsAnimating = TRUE;
	m_fSecsIntoState = 0.0;
	m_bUsingCustomSrcRect = FALSE ;
	m_Effect =  no_effect ;
	m_fPercentBetweenColors = 0.0f ;
	m_bTweeningTowardEndColor = TRUE ;
	m_fDeltaPercentPerSecond = 1.0 ;
	m_fWagRadians =  0.2f ;
	m_fWagPeriod =  2.0f ;
	m_fWagTimer =  0.0f ;
	m_fSpinSpeed =  2.0f ;
	m_fVibrationDistance =  5.0f ;
	m_bVisibleThisFrame =  FALSE;
	m_bBlendAdd = FALSE;
}

Sprite::~Sprite()
{
//	RageLog( "Sprite Destructor" );

	TM->UnloadTexture( m_sTexturePath ); 
}


BOOL Sprite::LoadFromTexture( CString sTexturePath )
{
	RageLog( ssprintf("Sprite::LoadFromTexture(%s)", sTexturePath) );

	Init();
	return LoadTexture( sTexturePath );
}

// Sprite file has the format:
//
// [Sprite]
// Texture=Textures\Logo 1x1.bmp
// Frame0000=0
// Delay0000=1.0
// Frame0001=3
// Delay0000=2.0
BOOL Sprite::LoadFromSpriteFile( CString sSpritePath )
{
	RageLog( ssprintf("Sprite::LoadFromSpriteFile(%s)", sSpritePath) );

	Init();

	m_sSpritePath = sSpritePath;

	// read sprite file
	IniFile ini;
	ini.SetPath( m_sSpritePath );
	if( !ini.ReadFile() )
		RageError( ssprintf("Error opening Sprite file '%s'.", m_sSpritePath) );

	CString sTexturePath = ini.GetValue( "Sprite", "Texture" );
	if( sTexturePath == "" )
		RageError( ssprintf("Error reading  value 'Texture' from %s.", m_sSpritePath) );

	// Load the texture
	if( !LoadTexture( sTexturePath ) )
		return FALSE;


	// Read in frames and delays from the sprite file, 
	// overwriting the states that LoadFromTexture created.
	for( UINT i=0; i<MAX_SPRITE_STATES; i++ )
	{
		CString sStateNo;
		sStateNo.Format( "%u%u%u%u", (i%10000)/1000, (i%1000)/100, (i%100)/10, (i%10) );	// four digit state no

		CString sFrameKey( CString("Frame") + sStateNo );
		CString sDelayKey( CString("Delay") + sStateNo );
		
		m_uFrame[i] = ini.GetValueI( "Sprite", sFrameKey );
		if( m_uFrame[i] >= m_pTexture->GetNumFrames() )
			RageError( ssprintf("In '%s', %s is %d, but the texture %s only has %d frames.",
						m_sSpritePath, sFrameKey, m_uFrame[i], sTexturePath, m_pTexture->GetNumFrames()) );
		m_fDelay[i] = (float)ini.GetValueF( "Sprite", sDelayKey );

		if( m_uFrame[i] == 0  &&  m_fDelay[i] > -0.00001f  &&  m_fDelay[i] < 0.00001f )	// both values are empty
			break;

		m_uNumStates = i+1;
	}

	if( m_uNumStates == 0 )
		RageError( ssprintf("Failed to find at least one state in %s.", m_sSpritePath) );

	return TRUE;
}

BOOL Sprite::LoadTexture( CString sTexturePath )
{
	if( m_sTexturePath != "" )			// If there was a previous bitmap...
		TM->UnloadTexture( m_sTexturePath );	// Unload it.


	m_sTexturePath = sTexturePath;

	m_pTexture = TM->LoadTexture( m_sTexturePath );
	assert( m_pTexture != NULL );
	SetWidth( (FLOAT)m_pTexture->GetFrameWidth() );
	SetHeight( (FLOAT)m_pTexture->GetFrameHeight() );

	// Assume the frames of this animation play in sequential order with 0.2 second delay.
	for( UINT i=0; i<m_pTexture->GetNumFrames(); i++ )
	{
		m_uFrame[i] = i;
		m_fDelay[i] = 0.1f;
		m_uNumStates = i+1;
	}
		
	return TRUE;
}


void Sprite::PrintDebugInfo()
{
//	Actor::PrintDebugInfo();

	RageLog( "Sprite::PrintDebugInfo()" );
	RageLog( "m_uNumStates: %u, m_uCurState: %u, m_fSecsIntoState: %f", 
		      m_uNumStates, m_uCurState, m_fSecsIntoState );
}


void Sprite::Update( const FLOAT &fDeltaTime )
{
	//PrintDebugInfo();

	Actor::Update( fDeltaTime );	// do tweening


	// update animation
	if( m_bIsAnimating )
	{
		m_fSecsIntoState += fDeltaTime;

		if( m_fSecsIntoState > m_fDelay[m_uCurState] )		// it's time to switch frames
		{
			// increment frame and reset the counter
			m_fSecsIntoState -= m_fDelay[m_uCurState];		// leave the left over time for the next frame
			m_uCurState ++;
			if( m_uCurState >= m_uNumStates )
				m_uCurState = 0;
		}
	}

	// update SpriteEffect
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
	case camelion:
	case glowing:
		if( m_bTweeningTowardEndColor ) {
			m_fPercentBetweenColors += m_fDeltaPercentPerSecond * fDeltaTime;
			if( m_fPercentBetweenColors > 1.0f ) {
				m_fPercentBetweenColors = 1.0f;
				m_bTweeningTowardEndColor = FALSE;
			}
		}
		else {		// !m_bTweeningTowardEndColor
			m_fPercentBetweenColors -= m_fDeltaPercentPerSecond * fDeltaTime;
			if( m_fPercentBetweenColors < 0.0f ) {
				m_fPercentBetweenColors = 0.0f;
				m_bTweeningTowardEndColor = TRUE;
			}
		}
	case wagging:
		m_fWagTimer += fDeltaTime;
		if( m_fWagTimer > m_fWagPeriod )
			m_fWagTimer -= m_fWagPeriod;
		break;
	case spinning:
		FLOAT rotation;
		rotation = GetRotation();
		rotation += m_fSpinSpeed * fDeltaTime;
		if( rotation > 2.0f * D3DX_PI )
			rotation -= 2.0f * D3DX_PI;
		else if( rotation < 0.0f )
			rotation += 2.0f * D3DX_PI;
		SetRotation( rotation );
		break;
	case vibrating:
		break;
	case flickering:
		break;
	}


}


void Sprite::Draw()
{
	if( m_pTexture == NULL )
		return;

	UINT uFrameNo = m_uFrame[m_uCurState];
	
	LPRECT pRectSrc;
	if( m_bUsingCustomSrcRect )
		pRectSrc = &m_rectCustomSrcRect;
	else
		pRectSrc = m_pTexture->GetFrameRect( uFrameNo );

	//::SetRect( &rectSrc, 0, 0, m_pTexture->GetImageWidth(), m_pTexture->GetImageHeight() );

	D3DXVECTOR2 scaling;
	scaling.x = GetZoom();
	scaling.y = GetZoom();
	
	D3DXVECTOR2 translation;
	translation.x = (float)GetLeftEdge();
	translation.y = (float)GetTopEdge();
	
	D3DXVECTOR3 rotation = m_rotation;

	D3DXVECTOR2 rot_center;
	rot_center.x = GetZoomedWidth()/2.0f;
	rot_center.y = GetZoomedHeight()/2.0f;

	D3DXCOLOR color = m_color;


	// update SpriteEffect
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
		color = m_bTweeningTowardEndColor ? m_start_color : m_end_color;
		break;
	case camelion:
	case glowing:
		color = m_start_color*m_fPercentBetweenColors + m_end_color*(1.0f-m_fPercentBetweenColors);
		break;
	case wagging:
		rotation.z = m_fWagRadians * (FLOAT)sin( 
			(m_fWagTimer / m_fWagPeriod)	// percent through wag
			* 2.0 * D3DX_PI );
		break;
	case spinning:
		// nothing special needed
		break;
	case vibrating:
		translation.x += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		translation.y += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		break;
	case flickering:
		m_bVisibleThisFrame = !m_bVisibleThisFrame;
		if( m_bVisibleThisFrame )
			return;		// don't draw the frame
		break;
	}

  	SCREEN->DrawQuad( m_pTexture,
					  pRectSrc,
					  &scaling,		// scaling
					  &rot_center,	// rotation center
					  &rotation,	// rotation
					  &translation,	// translation
					  color,
					  m_Effect == glowing ? op_add : op_modulate,
					  m_bBlendAdd );

}


void Sprite::SetState( UINT uNewState )
{
	ASSERT( uNewState >= 0  &&  uNewState < m_uNumStates );
	m_uCurState = uNewState;
	m_fSecsIntoState = 0.0; 
}

void Sprite::SetCustomSrcRect( RECT newRect ) 
{ 
	m_bUsingCustomSrcRect = TRUE;
	m_rectCustomSrcRect = newRect; 

	SetWidth( (FLOAT) RECTWIDTH(newRect) );
	SetHeight( (FLOAT) RECTHEIGHT(newRect) );

};

VOID Sprite::SetEffectNone()
{
	m_Effect = no_effect;
	//m_color = D3DXCOLOR( 1.0,1.0,1.0,1.0 );
}

VOID Sprite::SetEffectBlinking( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = blinking;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

VOID Sprite::SetEffectCamelion( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = camelion;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

VOID Sprite::SetEffectGlowing( FLOAT fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = glowing;
	m_start_color = Color;
	m_end_color = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

VOID Sprite::SetEffectWagging(	FLOAT fWagRadians, FLOAT fWagPeriod )
{
	m_Effect = wagging;
	m_fWagRadians = fWagRadians;
	m_fWagPeriod = fWagPeriod;
}

VOID Sprite::SetEffectSpinning(	FLOAT fSpinSpeed /*radians per second*/ )
{
	m_Effect = spinning;
	m_fSpinSpeed = fSpinSpeed;
}

VOID Sprite::SetEffectVibrating( FLOAT fVibrationDistance )
{
	m_Effect = vibrating;
	m_fVibrationDistance = fVibrationDistance;
}

VOID Sprite::SetEffectFlickering()
{
	m_Effect = flickering;
}
