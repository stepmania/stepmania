/*
-----------------------------------------------------------------------------
 File: Actor.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _ACTOR_H_
#define _ACTOR_H_

#include "RageUtil.h"

#include <d3dx8math.h>




class Actor
{
public:
	Actor();

	enum TweenType { no_tween, tween_linear, tween_bias_begin, tweening_bias_end };

	virtual void Draw() = 0;
	virtual void Update( const FLOAT &fDeltaTime );

	virtual FLOAT GetX()					{ return m_pos.x; };
	virtual FLOAT GetY()					{ return m_pos.y; };
	virtual void  SetX( FLOAT x )			{ m_pos.x = x;					m_TweenType = no_tween; };
	virtual void  SetY( FLOAT y )			{				m_pos.y = y;	m_TweenType = no_tween; };
	virtual void  SetXY( FLOAT x, FLOAT y )	{ m_pos.x = x;	m_pos.y = y;	m_TweenType = no_tween; };

	// height and width vary depending on zoom
	virtual FLOAT  GetZoomedWidth()			{ return m_size.x * m_scale.x; }
	virtual FLOAT  GetZoomedHeight()		{ return m_size.y * m_scale.y; }
	virtual void   SetWidth( FLOAT width ){ m_size.x = width; }
	virtual void   SetHeight( FLOAT height ){ m_size.y = height; }

	virtual FLOAT GetZoom()				{ return m_scale.x; }
	virtual void SetZoom( FLOAT zoom )	{ m_scale.x = zoom;	m_scale.y = zoom; }

	virtual FLOAT GetRotation()				{ return m_rotation.z; }
	virtual void  SetRotation( FLOAT rot )	{ m_rotation.z = rot; }
	virtual FLOAT GetRotationX()			{ return m_rotation.x; }
	virtual void  SetRotationX( FLOAT rot )	{ m_rotation.x = rot; }
	virtual FLOAT GetRotationY()			{ return m_rotation.y; }
	virtual void  SetRotationY( FLOAT rot )	{ m_rotation.y = rot; }

	virtual void SetColor( D3DXCOLOR newColor ) { m_color = newColor; };
	virtual D3DXCOLOR GetColor()				{ return m_color; };

	virtual void TweenTo( FLOAT time, 
						  FLOAT x, FLOAT y, 
						  FLOAT zoom = 1.0, 
						  FLOAT rot = 0.0, 
						  D3DXCOLOR col = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ),
						  TweenType tt = tween_linear );

	virtual void SetTweening( FLOAT time, TweenType tt = tween_linear );
	virtual void SetTweenX( FLOAT x );
	virtual void SetTweenY( FLOAT y );
	virtual void SetTweenXY( FLOAT x, FLOAT y );
	virtual void SetTweenZoom( FLOAT zoom );
	virtual void SetTweenRotationX( FLOAT r );
	virtual void SetTweenRotationY( FLOAT r );
	virtual void SetTweenRotationZ( FLOAT r );
	virtual void SetTweenColor( D3DXCOLOR c );


	// NOTE: GetEdge functions don't consider rotation
	virtual FLOAT GetLeftEdge()		{ return m_pos.x - GetZoomedWidth()/2.0f; };
	virtual FLOAT GetRightEdge()	{ return m_pos.x + GetZoomedWidth()/2.0f; };
	virtual FLOAT GetTopEdge()		{ return m_pos.y - GetZoomedHeight()/2.0f; };
	virtual FLOAT GetBottomEdge()	{ return m_pos.y + GetZoomedHeight()/2.0f; };
	
	enum StretchType { fit_inside, cover };

	void ScaleToCover( LPRECT rect )		{ ScaleTo( rect, cover ); };
	void ScaleToFitInside( LPRECT rect )	{ ScaleTo( rect, fit_inside); };
	void ScaleTo( LPRECT rect, StretchType st );

	void StretchTo( LPRECT rect );

protected:

	void Init();

	D3DXVECTOR2 m_size;		// width, height
	D3DXVECTOR2 m_pos;		// X-Y coordinate of where the center point will appear on screen
	D3DXVECTOR3 m_rotation;	// X, Y, and Z m_rotation
	D3DXVECTOR2 m_scale;	// X and Y zooming
	D3DXCOLOR   m_color;

	// start and end position for tweening
	D3DXVECTOR2 m_start_pos,		m_end_pos;
	D3DXVECTOR3 m_start_rotation,	m_end_rotation;
	D3DXVECTOR2 m_start_scale,		m_end_scale;
	D3DXCOLOR   m_start_color,		m_end_color;

	// counters for tweening
	TweenType	m_TweenType;
	FLOAT		m_fTweenTime;		// seconds between Start and End positions/zooms
	FLOAT		m_fTimeIntoTween;	// how long we have been tweening for

};



#endif