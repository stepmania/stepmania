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

	// let subclasses override
	virtual void Restore() {};
	virtual void Invalidate() {};

	virtual void Draw() PURE;
	virtual void Update( const float &fDeltaTime );

	virtual float GetX()					{ return m_pos.x; };
	virtual float GetY()					{ return m_pos.y; };
	virtual void  SetX( float x )			{ m_pos.x = x;					m_TweenType = no_tween; };
	virtual void  SetY( float y )			{				m_pos.y = y;	m_TweenType = no_tween; };
	virtual void  SetXY( float x, float y )	{ m_pos.x = x;	m_pos.y = y;	m_TweenType = no_tween; };

	// height and width vary depending on zoom
	virtual float  GetZoomedWidth()			{ return m_size.x * m_scale.x; }
	virtual float  GetZoomedHeight()		{ return m_size.y * m_scale.y; }
	virtual void   SetWidth( float width ){ m_size.x = width; }
	virtual void   SetHeight( float height ){ m_size.y = height; }

	virtual float GetZoom()				{ return m_scale.x; }
	virtual void SetZoom( float zoom )	{ m_scale.x = zoom;	m_scale.y = zoom; }

	virtual float GetRotation()				{ return m_rotation.z; }
	virtual void  SetRotation( float rot )	{ m_rotation.z = rot; }
	virtual float GetRotationX()			{ return m_rotation.x; }
	virtual void  SetRotationX( float rot )	{ m_rotation.x = rot; }
	virtual float GetRotationY()			{ return m_rotation.y; }
	virtual void  SetRotationY( float rot )	{ m_rotation.y = rot; }

	virtual void SetColor( D3DXCOLOR newColor ) { m_color = newColor; };
	virtual D3DXCOLOR GetColor()				{ return m_color; };

	virtual void TweenTo( float time, 
						  float x, float y, 
						  float zoom = 1.0, 
						  float rot = 0.0, 
						  D3DXCOLOR col = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ),
						  TweenType tt = tween_linear );

	virtual void BeginTweening( float time, TweenType tt = tween_linear );
	virtual void SetTweenX( float x );
	virtual void SetTweenY( float y );
	virtual void SetTweenXY( float x, float y );
	virtual void SetTweenZoom( float zoom );
	virtual void SetTweenRotationX( float r );
	virtual void SetTweenRotationY( float r );
	virtual void SetTweenRotationZ( float r );
	virtual void SetTweenColor( D3DXCOLOR c );


	// NOTE: GetEdge functions don't consider rotation
	virtual float GetLeftEdge()		{ return m_pos.x - GetZoomedWidth()/2.0f; };
	virtual float GetRightEdge()	{ return m_pos.x + GetZoomedWidth()/2.0f; };
	virtual float GetTopEdge()		{ return m_pos.y - GetZoomedHeight()/2.0f; };
	virtual float GetBottomEdge()	{ return m_pos.y + GetZoomedHeight()/2.0f; };
	
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
	float		m_fTweenTime;		// seconds between Start and End positions/zooms
	float		m_fTimeIntoTween;	// how long we have been tweening for

};



#endif