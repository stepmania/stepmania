#include "stdafx.h"	// testing updates
/*
-----------------------------------------------------------------------------
 File: Actor.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include <math.h>
#include "RageScreen.h"


Actor::Actor()
{
	Init();
}

void Actor::Init()
{
	m_size			= m_start_pos			= m_end_pos			= D3DXVECTOR2( 0, 0 );
	m_pos			= m_start_pos			= m_end_pos			= D3DXVECTOR2( 0, 0 );
	m_rotation		= m_start_rotation		= m_end_rotation	= D3DXVECTOR3( 0, 0, 0 );
	m_scale			= m_start_scale			= m_end_scale		= D3DXVECTOR2( 1, 1 );
	for(int i=0; i<4; i++) m_colorDiffuse[i]	= m_start_colorDiffuse[i]	= m_end_colorDiffuse[i]= D3DXCOLOR( 1, 1, 1, 1 );
	m_colorAdd		= m_start_colorAdd		= m_end_colorAdd	= D3DXCOLOR( 0, 0, 0, 0 );

	m_TweenType	= no_tween;
	m_fTweenTime = 0.0f;
	m_fTimeIntoTween = 0.0f;
}


void Actor::Draw()
{
	D3DXVECTOR2 pos				= m_pos;
	D3DXVECTOR3 rotation		= m_rotation;
	D3DXVECTOR2 scale			= m_scale;

	// update properties based on SpriteEffects 
	switch( m_Effect )
	{
	case no_effect:
		break;
	case blinking:
		break;
	case camelion:
		break;
	case glowing:
		break;
	case wagging:
		rotation.z = m_fWagRadians * (float)sin( 
			(m_fWagTimer / m_fWagPeriod)	// percent through wag
			* 2.0 * D3DX_PI );
		break;
	case spinning:
		// nothing special needed
		break;
	case vibrating:
		pos.x += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		pos.y += m_fVibrationDistance * randomf(-1.0f, 1.0f) * GetZoom();
		break;
	case flickering:
		break;
	}

	
	LPDIRECT3DDEVICE8 pd3dDevice = SCREEN->GetDevice();

	// calculate and apply world transform
	D3DXMATRIX matWorld, matTemp;
	D3DXMatrixIdentity( &matWorld );	// pass this along to each thing we draw and let it set it's own world matrix
	D3DXMatrixScaling( &matTemp, scale.x, scale.y, 1 );	// add in the zoom
	matWorld *= matTemp;
	D3DXMatrixRotationYawPitchRoll( &matTemp, rotation.y, rotation.x, rotation.z );	// add in the rotation
	matWorld *= matTemp;
	D3DXMatrixTranslation( &matTemp, pos.x, pos.y, 0 );	// add in the translation
	matWorld *= matTemp;
	D3DXMatrixTranslation( &matTemp, -0.5f, -0.5f, 0 );		// shift to align texels with pixels
	matWorld *= matTemp;
    pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

}


void Actor::Update( const float &fDeltaTime )
{
//	RageLog( "Actor::Update( %f )", fDeltaTime );

	// update effect
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
		RageLog( "Actor::m_fPercentBetweenColors %f", m_fPercentBetweenColors );
		break;
	case wagging:
		m_fWagTimer += fDeltaTime;
		if( m_fWagTimer > m_fWagPeriod )
			m_fWagTimer -= m_fWagPeriod;
		break;
	case spinning:
		float rotation;
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


	// update tweening
	if( m_TweenType != no_tween )		// we are performing some type of tweening
	{
		m_fTimeIntoTween += fDeltaTime;

		if( m_fTimeIntoTween > m_fTweenTime )	// The tweening is over.  Stop the tweening
		{
			m_pos = m_end_pos;
			m_scale = m_end_scale;
			m_rotation = m_end_rotation;
			for(int i=0; i<4; i++) m_colorDiffuse[i] = m_end_colorDiffuse[i];
			m_colorAdd = m_end_colorAdd;
			m_TweenType = no_tween;
		}
		else		// Tweening.  Recalcute the curent position.
		{
			float fPercentThroughTween = m_fTimeIntoTween / m_fTweenTime;

			// distort the percentage if appropriate
			if( m_TweenType == tween_bias_begin )
				fPercentThroughTween = (float) sqrt( fPercentThroughTween );
			else if( m_TweenType == tweening_bias_end )
				fPercentThroughTween = fPercentThroughTween * fPercentThroughTween;


			m_pos			= m_start_pos	  + (m_end_pos		- m_start_pos	  )*fPercentThroughTween;
			m_scale			= m_start_scale	  + (m_end_scale	- m_start_scale	  )*fPercentThroughTween;
			m_rotation		= m_start_rotation+ (m_end_rotation - m_start_rotation)*fPercentThroughTween;
			for(int i=0; i<4; i++) m_colorDiffuse[i]	= m_start_colorDiffuse[i]*(1.0f-fPercentThroughTween) + m_end_colorDiffuse[i]*(fPercentThroughTween);
			m_colorAdd		= m_start_colorAdd    *(1.0f-fPercentThroughTween) + m_end_colorAdd    *(fPercentThroughTween);
		}
	
	}	// end if m_TweenType != no_tween

}


void Actor::TweenTo( float time, float x, float y, float zoom, float rot, D3DXCOLOR col, TweenType tt )
{
	// set our tweeen starting values to the current position
	m_start_pos			= m_pos;
	m_start_scale		= m_scale;
	m_start_rotation	= m_rotation;
	for(int i=0; i<4; i++) m_start_colorDiffuse[i]		= m_colorDiffuse[i];

	// set the ending tweening position to what the user asked for
	m_end_pos.x = (float)x;
	m_end_pos.y = (float)y;
	m_end_scale.x = zoom;
	m_end_scale.y = zoom;
	m_end_rotation.z = rot;
	for(i=0; i<4; i++) m_end_colorDiffuse[i] = col;
	m_TweenType = tt;
	m_fTweenTime = time;
	m_fTimeIntoTween = 0;

}


void Actor::BeginTweening( float time, TweenType tt )
{
	// set our tweeen starting and ending values to the current position
	m_start_pos				= m_end_pos				= m_pos;
	m_start_scale			= m_end_scale			= m_scale;
	m_start_rotation		= m_end_rotation		= m_rotation;
	for(int i=0; i<4; i++)m_start_colorDiffuse[i]	= m_end_colorDiffuse[i]	= m_colorDiffuse[i];
	m_start_colorAdd		= m_end_colorAdd		= m_colorAdd;

	m_TweenType = tt;
	m_fTweenTime = time;
	m_fTimeIntoTween = 0;
}

void Actor::SetTweenX( float x )			{ m_end_pos.x = x; } 
void Actor::SetTweenY( float y )			{ m_end_pos.y = y; }
void Actor::SetTweenXY( float x, float y )	{ SetTweenX(x); SetTweenY(y); }
void Actor::SetTweenZoom( float zoom )		{ m_end_scale.x = zoom;  m_end_scale.y = zoom; }
void Actor::SetTweenRotationX( float r )	{ m_end_rotation.x = r; }
void Actor::SetTweenRotationY( float r )	{ m_end_rotation.y = r; }
void Actor::SetTweenRotationZ( float r )	{ m_end_rotation.z = r; }
void Actor::SetTweenDiffuseColor( D3DXCOLOR c )	{ for(int i=0; i<4; i++) m_end_colorDiffuse[i] = c; }
void Actor::SetTweenAddColor( D3DXCOLOR c )	{ m_end_colorAdd = c; }


void Actor::ScaleTo( LPRECT pRect, StretchType st )
{
	// width and height of rectangle
	float rect_width = RECTWIDTH(*pRect);
	float rect_height = RECTHEIGHT(*pRect);

	if( rect_width < 0 )	SetRotationY( D3DX_PI );
	if( rect_height < 0 )	SetRotationX( D3DX_PI );

	// center of the rectangle
	float rect_cx = pRect->left + rect_width/2;
	float rect_cy = pRect->top  + rect_height/2;

	// zoom factor needed to scale the Actor to fill the rectangle
	float fNewZoomX = (float)fabs(rect_width  / m_size.x);
	float fNewZoomY = (float)fabs(rect_height / m_size.y);

	float fNewZoom;
	switch( st )
	{
	case cover:
		fNewZoom = fNewZoomX>fNewZoomY ? fNewZoomX : fNewZoomY;	// use larger zoom
		break;
	case fit_inside:
		fNewZoom = fNewZoomX>fNewZoomY ? fNewZoomY : fNewZoomX; // use smaller zoom
		break;
	}

	SetXY( rect_cx, rect_cy );
	SetZoom( fNewZoom );
}


void Actor::StretchTo( LPRECT pRect )
{
	// width and height of rectangle
	int rect_width = RECTWIDTH(*pRect);
	int rect_height = RECTHEIGHT(*pRect);

	if( rect_width < 0 )	SetRotationY( D3DX_PI );
	if( rect_height < 0 )	SetRotationX( D3DX_PI );

	// center of the rectangle
	int rect_cx = pRect->left + rect_width/2;
	int rect_cy = pRect->top  + rect_height/2;

	// zoom factor needed to scale the Actor to fill the rectangle
	float fNewZoomX = (float)fabs(rect_width  / m_size.x);
	float fNewZoomY = (float)fabs(rect_height / m_size.y);

	SetXY( (float)rect_cx, (float)rect_cy );
	m_scale.x = fNewZoomX;
	m_scale.y = fNewZoomY;
}




// effects

void Actor::SetEffectNone()
{
	m_Effect = no_effect;
	//m_color = D3DXCOLOR( 1.0,1.0,1.0,1.0 );
}

void Actor::SetEffectBlinking( float fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = blinking;
	for(int i=0; i<4; i++) {
		m_start_colorDiffuse[i] = Color;
		m_end_colorDiffuse[i] = Color2;
	}
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Actor::SetEffectCamelion( float fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = camelion;
	for(int i=0; i<4; i++) {
		m_start_colorDiffuse[i] = Color;
		m_end_colorDiffuse[i] = Color2;
	}
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Actor::SetEffectGlowing( float fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = glowing;
	m_start_colorAdd = Color;
	m_end_colorAdd = Color2;
	//m_fPercentBetweenColors = 0.0;
	//m_bTweeningTowardEndColor = TRUE;
	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Actor::SetEffectWagging(	float fWagRadians, float fWagPeriod )
{
	m_Effect = wagging;
	m_fWagRadians = fWagRadians;
	m_fWagPeriod = fWagPeriod;
}

void Actor::SetEffectSpinning(	float fSpinSpeed /*radians per second*/ )
{
	m_Effect = spinning;
	m_fSpinSpeed = fSpinSpeed;
}

void Actor::SetEffectVibrating( float fVibrationDistance )
{
	m_Effect = vibrating;
	m_fVibrationDistance = fVibrationDistance;
}

void Actor::SetEffectFlickering()
{
	m_Effect = flickering;
}