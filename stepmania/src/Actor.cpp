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


Actor::Actor()
{
	Init();
}

void Actor::Init()
{
	m_size		= D3DXVECTOR2( 0, 0 );
	m_pos		= D3DXVECTOR2( 0, 0 );
	m_rotation	= D3DXVECTOR3( 0, 0, 0 );
	m_scale		= D3DXVECTOR2( 1, 1 );
	m_color		= D3DXCOLOR( 1, 1, 1, 1 );

	m_start_pos		= m_end_pos			= D3DXVECTOR2( 0.0f, 0.0f );
	m_start_rotation= m_end_rotation	= D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	m_start_scale	= m_end_scale		= D3DXVECTOR2( 1.0f, 1.0f );
	m_start_color	= m_end_color		= D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );

	m_TweenType	= no_tween;
	m_fTweenTime = 0.0f;
	m_fTimeIntoTween = 0.0f;
}


void Actor::Update( const FLOAT &fDeltaTime )
{
//	RageLog( "Actor::Update( %f )", fDeltaTime );

	// update tweening
	if( m_TweenType != no_tween )		// we are performing some type of tweening
	{
		m_fTimeIntoTween += fDeltaTime;

		if( m_fTimeIntoTween > m_fTweenTime )	// The tweening is over.  Stop the tweening
		{
			m_pos = m_end_pos;
			m_scale = m_end_scale;
			m_rotation = m_end_rotation;
			m_color = m_end_color;
			m_TweenType = no_tween;
		}
		else		// Tweening.  Recalcute the curent position.
		{
			FLOAT fPercentThroughTween = m_fTimeIntoTween / m_fTweenTime;

			// distort the percentage if appropriate
			if( m_TweenType == tween_bias_begin )
				fPercentThroughTween = (FLOAT) sqrt( fPercentThroughTween );
			else if( m_TweenType == tweening_bias_end )
				fPercentThroughTween = fPercentThroughTween * fPercentThroughTween;


			m_pos		= m_start_pos	  + (m_end_pos		- m_start_pos	  )*fPercentThroughTween;
			m_scale		= m_start_scale	  + (m_end_scale	- m_start_scale	  )*fPercentThroughTween;
			m_rotation	= m_start_rotation+ (m_end_rotation - m_start_rotation)*fPercentThroughTween;
			m_color		= m_start_color*(1.0f-fPercentThroughTween) + m_end_color*(fPercentThroughTween);
		}
	
	}	// end if m_TweenType != no_tween


}


void Actor::TweenTo( FLOAT time, FLOAT x, FLOAT y, FLOAT zoom, FLOAT rot, D3DXCOLOR col, TweenType tt )
{
	// set our tweeen starting values to the current position
	m_start_pos			= m_pos;
	m_start_scale		= m_scale;
	m_start_rotation	= m_rotation;
	m_start_color		= m_color;

	// set the ending tweening position to what the user asked for
	m_end_pos.x = (FLOAT)x;
	m_end_pos.y = (FLOAT)y;
	m_end_scale.x = zoom;
	m_end_scale.y = zoom;
	m_end_rotation.z = rot;
	m_end_color = col;
	m_TweenType = tt;
	m_fTweenTime = time;
	m_fTimeIntoTween = 0;

}


void Actor::BeginTweening( FLOAT time, TweenType tt )
{
	// set our tweeen starting and ending values to the current position
	m_start_pos			= m_end_pos			= m_pos;
	m_start_scale		= m_end_scale		= m_scale;
	m_start_rotation	= m_end_rotation	= m_rotation;
	m_start_color		= m_end_color		= m_color;

	m_TweenType = tt;
	m_fTweenTime = time;
	m_fTimeIntoTween = 0;
}

void Actor::SetTweenX( FLOAT x )			{ m_end_pos.x = x; } 
void Actor::SetTweenY( FLOAT y )			{ m_end_pos.y = y; }
void Actor::SetTweenXY( FLOAT x, FLOAT y )	{ SetTweenX(x); SetTweenY(y); }
void Actor::SetTweenZoom( FLOAT zoom )		{ m_end_scale.x = zoom;  m_end_scale.y = zoom; }
void Actor::SetTweenRotationX( FLOAT r )	{ m_end_rotation.x = r; }
void Actor::SetTweenRotationY( FLOAT r )	{ m_end_rotation.y = r; }
void Actor::SetTweenRotationZ( FLOAT r )	{ m_end_rotation.z = r; }
void Actor::SetTweenColor( D3DXCOLOR c )	{ m_end_color = c; }


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
	FLOAT fNewZoomX = (FLOAT)fabs(rect_width  / m_size.x);
	FLOAT fNewZoomY = (FLOAT)fabs(rect_height / m_size.y);

	SetXY( (FLOAT)rect_cx, (FLOAT)rect_cy );
	m_scale.x = fNewZoomX;
	m_scale.y = fNewZoomY;
}
