/*
-----------------------------------------------------------------------------
 File: MotionBlurSprite.h

 Desc: A graphic that appears to blur and come into focus.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#ifndef _MotionBlurSprite_H_
#define _MotionBlurSprite_H_


#include "Sprite.h"
#include "ActorFrame.h"


const int NUM_BLUR_GHOSTS	=	2;

class MotionBlurSprite : public Actor
{
public:

	virtual bool Load( const CString &sFilePath ) { for( int i=0; i<NUM_BLUR_GHOSTS; i++ ) m_sprites[i].Load( sFilePath ); return true; };

	virtual void Update( float fDeltaTime ) { for( int i=0; i<NUM_BLUR_GHOSTS; i++ ) m_sprites[i].Update( fDeltaTime ); };
	virtual void Draw()						{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].Draw(); };
	virtual void DrawPrimitives()			{};

	virtual float GetX()					{ return m_sprites[0].GetX(); };
	virtual float GetY()					{ return m_sprites[0].GetY(); };
	virtual float GetZ()					{ return m_sprites[0].GetZ(); };
	virtual void  SetX( float x )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetX( x ); };
	virtual void  SetY( float y )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetY( y ); };
	virtual void  SetZ( float z )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetZ( z ); };
	virtual void  SetXY( float x, float y )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetXY(x, y); };

	// height and width vary depending on zoom
	virtual float GetUnzoomedWidth()		{ return m_sprites[0].GetUnzoomedWidth(); }
	virtual float GetUnzoomedHeight()		{ return m_sprites[0].GetUnzoomedHeight(); }
	virtual float GetZoomedWidth()			{ return m_sprites[0].GetZoomedWidth(); }
	virtual float GetZoomedHeight()			{ return m_sprites[0].GetZoomedHeight(); }
	virtual void  SetWidth( float width )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetWidth(width); }
	virtual void  SetHeight( float height )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetHeight(height); }

	virtual float GetZoom()				{ return m_sprites[0].GetZoom(); }
	virtual float GetZoomX()			{ return m_sprites[0].GetZoomX(); }
	virtual float GetZoomY()			{ return m_sprites[0].GetZoomY(); }
	virtual void  SetZoom( float zoom )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetZoom(zoom); }
	virtual void  SetZoomX( float zoom ){ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetZoomX(zoom); }
	virtual void  SetZoomY( float zoom ){ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetZoomY(zoom); }

	virtual float GetRotation()				{ return m_sprites[0].GetRotation(); }
	virtual float GetRotationX()			{ return m_sprites[0].GetRotationX(); }
	virtual float GetRotationY()			{ return m_sprites[0].GetRotationY(); }
	virtual void  SetRotation( float rot )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetRotation(rot); }
	virtual void  SetRotationX( float rot )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetRotationX(rot); }
	virtual void  SetRotationY( float rot )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetRotationY(rot); }

	virtual void SetDiffuseColor( D3DXCOLOR c )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetDiffuseColor(c); };
	virtual D3DXCOLOR GetDiffuseColor()			{ return m_sprites[0].GetDiffuseColor(); };
	virtual void SetAddColor( D3DXCOLOR c )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetAddColor(c); };
	virtual D3DXCOLOR GetAddColor()				{ return m_sprites[0].GetAddColor(); };


	// THE BLUR IS MADE BY DELAYING THE TWEENS
	virtual void BeginBlurredTweening( float time, TweenType tt = TWEEN_LINEAR )
	{
		for(int i=0; i<NUM_BLUR_GHOSTS; i++) 
		{
			m_sprites[i].BeginTweeningQueued( i*0.1f+0.01f, tt );	// delay
			m_sprites[i].BeginTweeningQueued( time, tt );	// original tween
		}
	};
	virtual void BeginTweening( float time, TweenType tt = TWEEN_LINEAR )
	{
		for(int i=0; i<NUM_BLUR_GHOSTS; i++) 
		{
			m_sprites[i].BeginTweeningQueued( time, tt );	// original tween
		}
	};
	virtual void BeginTweeningQueued( float time, TweenType tt = TWEEN_LINEAR )	
	{ 
		for(int i=0; i<NUM_BLUR_GHOSTS; i++) 
		{
			m_sprites[i].BeginTweeningQueued( time, tt );	// original tween
		}
	};
	virtual void StopTweening()						{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].StopTweening(); };
	virtual void SetTweenX( float x )				{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenX(x); };
	virtual void SetTweenY( float y )				{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenY(y); };
	virtual void SetTweenZ( float z )				{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenZ(z); };
	virtual void SetTweenXY( float x, float y )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenXY(x,y); };
	virtual void SetTweenZoom( float zoom )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenZoom(zoom); };
	virtual void SetTweenZoomX( float zoom )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenZoomX(zoom); };
	virtual void SetTweenZoomY( float zoom )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenZoomY(zoom); };
	virtual void SetTweenRotationX( float r )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenRotationX(r); };
	virtual void SetTweenRotationY( float r )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenRotationY(r); };
	virtual void SetTweenRotationZ( float r )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenRotationZ(r); };
	virtual void SetTweenDiffuseColor( D3DXCOLOR c ){ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenDiffuseColor(c); };
	virtual void SetTweenAddColor( D3DXCOLOR c )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetTweenAddColor(c); };


	void ScaleToCover( LPRECT rect )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].ScaleToCover(rect); };
	void ScaleToFitInside( LPRECT rect )		{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].ScaleToFitInside(rect); };
	void ScaleTo( LPRECT rect, StretchType st )	{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].ScaleTo(rect,st); };
	void StretchTo( LPRECT rect )				{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].StretchTo(rect); };

	void SetHorizAlign( HorizAlign ha )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetHorizAlign(ha); };
	void SetVertAlign( VertAlign va )			{ for(int i=0; i<NUM_BLUR_GHOSTS; i++) m_sprites[i].SetVertAlign(va); };


protected:
	Sprite m_sprites[NUM_BLUR_GHOSTS];
};




#endif