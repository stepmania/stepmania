/*
-----------------------------------------------------------------------------
 File: ActorFrame.h

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


#ifndef _ActorFrame_H_
#define _ActorFrame_H_

#include "RageUtil.h"

#include <d3dx8math.h>

#include "Actor.h"


class ActorFrame
{
protected:
	CArray<Actor*,Actor*>	m_SubActors;


public:
	ActorFrame();

	void AddActor( Actor* pActor ) { m_SubActors.Add(pActor); };


	enum TweenType { no_tween, tween_linear, tween_bias_begin, tweening_bias_end };

	// let subclasses override
	void Restore()					{ for(int i=0; i<m_SubActors.GetSize(); i++) m_SubActors[i]->Restore(); };
	void Invalidate()				{ for(int i=0; i<m_SubActors.GetSize(); i++) m_SubActors[i]->Invalidate(); };

	void Draw();
	void Update( float fDeltaTime );

	float GetX()					{ return m_pos.x; };
	float GetY()					{ return m_pos.y; };
	void  SetX( float x )			{ m_pos.x = x;					m_TweenType = no_tween; };
	void  SetY( float y )			{				m_pos.y = y;	m_TweenType = no_tween; };
	void  SetXY( float x, float y )	{ m_pos.x = x;	m_pos.y = y;	m_TweenType = no_tween; };

	float GetZoom()				{ return m_scale.x; }
	void  SetZoom( float zoom )	{ m_scale.x = zoom;	m_scale.y = zoom; }

	float GetRotation()				{ return m_rotation.z; }
	void  SetRotation( float rot )	{ m_rotation.z = rot; }
	float GetRotationX()			{ return m_rotation.x; }
	void  SetRotationX( float rot )	{ m_rotation.x = rot; }
	float GetRotationY()			{ return m_rotation.y; }
	void  SetRotationY( float rot )	{ m_rotation.y = rot; }

	void SetDiffuseColor( D3DXCOLOR colorDiffuse ) { for(int i=0; i<m_SubActors.GetSize(); i++) m_SubActors[i]->SetDiffuseColor(colorDiffuse); };
	void SetAddColor( D3DXCOLOR colorAdd ) { for(int i=0; i<m_SubActors.GetSize(); i++) m_SubActors[i]->SetAddColor(colorAdd); };


	void BeginTweening( float time, TweenType tt = tween_linear );
	void SetTweenX( float x );
	void SetTweenY( float y );
	void SetTweenXY( float x, float y );
	void SetTweenZoom( float zoom );
	void SetTweenRotationX( float r );
	void SetTweenRotationY( float r );
	void SetTweenRotationZ( float r );
	void SetTweenDiffuseColor( D3DXCOLOR c );
	void SetTweenAddColor( D3DXCOLOR c );


protected:
	D3DXVECTOR2 m_pos;		// X-Y coordinate of where the center point will appear on screen
	D3DXVECTOR3 m_rotation;	// X, Y, and Z m_rotation
	D3DXVECTOR2 m_scale;	// X and Y zooming

	// start and end position for tweening
	D3DXVECTOR2 m_start_pos,			m_end_pos;
	D3DXVECTOR3 m_start_rotation,		m_end_rotation;
	D3DXVECTOR2 m_start_scale,			m_end_scale;

	// counters for tweening
	TweenType	m_TweenType;
	float		m_fTweenTime;		// seconds between Start and End positions/zooms
	float		m_fTimeIntoTween;	// how long we have been tweening for
};



#endif