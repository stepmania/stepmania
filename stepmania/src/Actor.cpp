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
	m_size									= D3DXVECTOR2( 1, 1 );
	m_pos			= m_start_pos			= D3DXVECTOR3( 0, 0, 0 );
	m_rotation		= m_start_rotation		= D3DXVECTOR3( 0, 0, 0 );
	m_scale			= m_start_scale			= D3DXVECTOR2( 1, 1 );
	for(int i=0; i<4; i++) m_colorDiffuse[i]	= m_start_colorDiffuse[i] = D3DXCOLOR( 1, 1, 1, 1 );
	m_colorAdd		= m_start_colorAdd		= D3DXCOLOR( 0, 0, 0, 0 );
}


void Actor::Draw()		// set the world matrix and calculate actor properties, the call RenderPrimitives
{
	SCREEN->PushMatrix();	// we're actually going to do some drawing in this function	

	
	D3DXVECTOR3 pos				= m_pos;
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
		rotation.z = m_fWagRadians * sinf( 
			(m_fWagTimer / m_fWagPeriod)	// percent through wag
			* 2.0f * D3DX_PI );
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
	case bouncing:
		float fPercentThroughBounce = m_fTimeIntoBounce / m_fBouncePeriod;
		float fPercentOffset = sinf( fPercentThroughBounce*D3DX_PI ); 
		pos += m_vectBounce * fPercentOffset;
		break;
	}

	
	SCREEN->Translate( pos.x, pos.y, pos.z );	// offset so that pixels are aligned to texels
	SCREEN->Scale( scale.x, scale.y, 1 );

	// super slow!	
	//	D3DXMatrixRotationYawPitchRoll( &matTemp, rotation.y, rotation.x, rotation.z );	// add in the rotation
	//	matNewWorld = matTemp * matNewWorld;

	// replace with...
	if( rotation.z != 0 )
	{
		SCREEN->RotateZ( rotation.z );
	}    



	this->RenderPrimitives();


	SCREEN->PopMatrix();

}


void Actor::Update( float fDeltaTime )
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
		//RageLog( "Actor::m_fPercentBetweenColors %f", m_fPercentBetweenColors );
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
	case bouncing:
		m_fTimeIntoBounce += fDeltaTime;
		if( m_fTimeIntoBounce >= m_fBouncePeriod )
			m_fTimeIntoBounce -= m_fBouncePeriod;
		break;
	}


	// update tweening
	if( m_QueuedTweens.GetSize() > 0 )		// we are performing some type of tweening
	{
		TweenState &TS = m_QueuedTweens[0];

		if( TS.m_fTimeLeftInTween == TS.m_fTweenTime )	// we are just beginning this tween
		{
			// set the start position
			m_start_pos				= m_pos;
			m_start_rotation		= m_rotation;
			m_start_scale			= m_scale;
			for( int i=0; i<4; i++) m_start_colorDiffuse[i] = m_colorDiffuse[i];
			m_start_colorAdd		= m_colorAdd;
		}
		
		TS.m_fTimeLeftInTween -= fDeltaTime;

		if( TS.m_fTimeLeftInTween <= 0 )	// The tweening is over.  Stop the tweening
		{
			m_pos			= TS.m_end_pos;
			m_scale			= TS.m_end_scale;
			m_rotation		= TS.m_end_rotation;
			for(int i=0; i<4; i++) m_colorDiffuse[i] = TS.m_end_colorDiffuse[i];
			m_colorAdd		= TS.m_end_colorAdd;
			
			m_QueuedTweens.RemoveAt( 0 );
			return;
		}
		else		// in the middle of tweening.  Recalcute the curent position.
		{
			float fPercentThroughTween = 1-(TS.m_fTimeLeftInTween / TS.m_fTweenTime);

			// distort the percentage if appropriate
			switch( TS.m_TweenType )
			{
			case TWEEN_BIAS_BEGIN:
				fPercentThroughTween = (float) sqrt( fPercentThroughTween );
				break;
			case TWEEN_BIAS_END:
				fPercentThroughTween = fPercentThroughTween * fPercentThroughTween;
				break;
			}


			m_pos			= m_start_pos	  + (TS.m_end_pos		- m_start_pos	  )*fPercentThroughTween;
			m_scale			= m_start_scale	  + (TS.m_end_scale		- m_start_scale	  )*fPercentThroughTween;
			m_rotation		= m_start_rotation+ (TS.m_end_rotation	- m_start_rotation)*fPercentThroughTween;
			for(int i=0; i<4; i++) m_colorDiffuse[i]	= m_start_colorDiffuse[i]*(1.0f-fPercentThroughTween) + TS.m_end_colorDiffuse[i]*(fPercentThroughTween);
			m_colorAdd		= m_start_colorAdd    *(1.0f-fPercentThroughTween) + TS.m_end_colorAdd    *(fPercentThroughTween);
		}
		

	}	// end if m_TweenType != no_tween

}



void Actor::BeginTweening( float time, TweenType tt )
{
	StopTweening();
	BeginTweeningQueued( time, tt );
}

void Actor::BeginTweeningQueued( float time, TweenType tt )
{
	ASSERT( time > 0 );

	m_QueuedTweens.Add( TweenState() );

	TweenState &TS = m_QueuedTweens[m_QueuedTweens.GetSize()-1];

	// set our tween starting and ending values to the current position
	TS.m_end_pos			= m_pos;
	TS.m_end_scale			= m_scale;
	TS.m_end_rotation		= m_rotation;
	for(int i=0; i<4; i++)	TS.m_end_colorDiffuse[i]	= m_colorDiffuse[i];
	TS.m_end_colorAdd		= m_colorAdd;

	TS.m_TweenType = tt;
	TS.m_fTweenTime = time;
	TS.m_fTimeLeftInTween = time;
}

void Actor::SetTweenX( float x )			{ GetLatestTween().m_end_pos.x = x; } 
void Actor::SetTweenY( float y )			{ GetLatestTween().m_end_pos.y = y; }
void Actor::SetTweenZ( float z )			{ GetLatestTween().m_end_pos.z = z; }
void Actor::SetTweenXY( float x, float y )	{ GetLatestTween().m_end_pos.x = x; GetLatestTween().m_end_pos.y = y; }
void Actor::SetTweenZoom( float zoom )		{ GetLatestTween().m_end_scale.x = zoom;  GetLatestTween().m_end_scale.y = zoom; }
void Actor::SetTweenZoomX( float zoom )		{ GetLatestTween().m_end_scale.x = zoom; }
void Actor::SetTweenZoomY( float zoom )		{ GetLatestTween().m_end_scale.y = zoom; }
void Actor::SetTweenRotationX( float r )	{ GetLatestTween().m_end_rotation.x = r; }
void Actor::SetTweenRotationY( float r )	{ GetLatestTween().m_end_rotation.y = r; }
void Actor::SetTweenRotationZ( float r )	{ GetLatestTween().m_end_rotation.z = r; }
void Actor::SetTweenDiffuseColor( D3DXCOLOR colorDiffuse )				{ for(int i=0; i<4; i++) GetLatestTween().m_end_colorDiffuse[i] = colorDiffuse; };
void Actor::SetTweenDiffuseColorUpperLeft( D3DXCOLOR colorDiffuse )		{ GetLatestTween().m_end_colorDiffuse[0] = colorDiffuse; };
void Actor::SetTweenDiffuseColorUpperRight( D3DXCOLOR colorDiffuse )	{ GetLatestTween().m_end_colorDiffuse[1] = colorDiffuse; };
void Actor::SetTweenDiffuseColorLowerLeft( D3DXCOLOR colorDiffuse )		{ GetLatestTween().m_end_colorDiffuse[2] = colorDiffuse; };
void Actor::SetTweenDiffuseColorLowerRight( D3DXCOLOR colorDiffuse )	{ GetLatestTween().m_end_colorDiffuse[3] = colorDiffuse; };
void Actor::SetTweenDiffuseColorTopEdge( D3DXCOLOR colorDiffuse )		{ GetLatestTween().m_end_colorDiffuse[0] = GetLatestTween().m_end_colorDiffuse[1] = colorDiffuse; };
void Actor::SetTweenDiffuseColorRightEdge( D3DXCOLOR colorDiffuse )		{ GetLatestTween().m_end_colorDiffuse[1] = GetLatestTween().m_end_colorDiffuse[3] = colorDiffuse; };
void Actor::SetTweenDiffuseColorBottomEdge( D3DXCOLOR colorDiffuse )	{ GetLatestTween().m_end_colorDiffuse[2] = GetLatestTween().m_end_colorDiffuse[3] = colorDiffuse; };
void Actor::SetTweenDiffuseColorLeftEdge( D3DXCOLOR colorDiffuse )		{ GetLatestTween().m_end_colorDiffuse[0] = GetLatestTween().m_end_colorDiffuse[2] = colorDiffuse; };
void Actor::SetTweenAddColor( D3DXCOLOR c )			{ GetLatestTween().m_end_colorAdd = c; };


void Actor::ScaleTo( LPRECT pRect, StretchType st )
{
	// width and height of rectangle
	float rect_width = (float)RECTWIDTH(*pRect);
	float rect_height = (float)RECTHEIGHT(*pRect);

	if( rect_width < 0 )	SetRotationY( D3DX_PI );
	if( rect_height < 0 )	SetRotationX( D3DX_PI );

	// center of the rectangle
	float rect_cx = pRect->left + rect_width/2;
	float rect_cy = pRect->top  + rect_height/2;

	// zoom fActor needed to scale the Actor to fill the rectangle
	float fNewZoomX = fabsf(rect_width  / m_size.x);
	float fNewZoomY = fabsf(rect_height / m_size.y);

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

	// center of the rectangle
	int rect_cx = pRect->left + rect_width/2;
	int rect_cy = pRect->top  + rect_height/2;

	// zoom fActor needed to scale the Actor to fill the rectangle
	float fNewZoomX = rect_width  / m_size.x;
	float fNewZoomY = rect_height / m_size.y;

	SetXY( (float)rect_cx, (float)rect_cy );
	SetZoomX( fNewZoomX );
	SetZoomY( fNewZoomY );
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
	m_effect_colorDiffuse1 = Color;
	m_effect_colorDiffuse2 = Color2;

	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Actor::SetEffectCamelion( float fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = camelion;
	m_effect_colorDiffuse1 = Color;
	m_effect_colorDiffuse2 = Color2;

	m_fDeltaPercentPerSecond = fDeltaPercentPerSecond;
}

void Actor::SetEffectGlowing( float fDeltaPercentPerSecond, D3DXCOLOR Color, D3DXCOLOR Color2 )
{
	m_Effect = glowing;
	m_effect_colorAdd1 = Color;
	m_effect_colorAdd2 = Color2;

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

void Actor::SetEffectBouncing( D3DXVECTOR3 vectBounce, float fPeriod )
{
	m_Effect = bouncing;
	
	m_vectBounce = vectBounce;
	m_fBouncePeriod = fPeriod;
	m_fTimeIntoBounce = 0;

}