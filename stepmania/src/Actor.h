#ifndef ACTOR_H
#define ACTOR_H
/*
-----------------------------------------------------------------------------
 Class: Actor

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTypes.h"


class Actor
{
public:
	Actor();
	virtual ~Actor() { }

	enum TweenType { 
		TWEEN_LINEAR, 
		TWEEN_ACCELERATE, 
		TWEEN_DECELERATE, 
		TWEEN_BOUNCE_BEGIN, 
		TWEEN_BOUNCE_END,
		TWEEN_SPRING,
	};
	enum Effect { no_effect,
				diffuse_blink,	diffuse_shift,
				glow_blink,		glow_shift,
				wag,	bounce,		bob,
				spin,	vibrate,	
				};

	struct TweenState
	{
		// start and end position for tweening
		RageVector3 pos;
		RageVector3 rotation;
		RageVector3 scale;
		RageColor   diffuse[4];
		RageColor   glow;

		void Init()
		{
			pos	= RageVector3( 0, 0, 0 );
			rotation = RageVector3( 0, 0, 0 );
			scale = RageVector3( 1, 1, 1 );
			for(int i=0; i<4; i++) 
				diffuse[i] = RageColor( 1, 1, 1, 1 );
			glow = RageColor( 1, 1, 1, 0 );
		};

		static void MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween )
		{
			average_out.pos			= ts1.pos	  + (ts2.pos		- ts1.pos	  )*fPercentBetween;
			average_out.scale		= ts1.scale	  + (ts2.scale		- ts1.scale   )*fPercentBetween;
			average_out.rotation	= ts1.rotation+ (ts2.rotation	- ts1.rotation)*fPercentBetween;
			for(int i=0; i<4; i++) 
				average_out.diffuse[i]	= ts1.diffuse[i]+ (ts2.diffuse[i]	- ts1.diffuse[i])*fPercentBetween;
			average_out.glow			= ts1.glow      + (ts2.glow			- ts1.glow		)*fPercentBetween;
		}
	};

	// let subclasses override

	/* Do subclasses really need to override tweening?  Tween data should
	 * probably be private ... - glenn */
	virtual void Restore() {};
	virtual void Invalidate() {};

	virtual void Draw();		// calls, BeginDraw, DrawPrimitives, EndDraw
	virtual void BeginDraw();	// pushes transform onto world matrix stack
	virtual void DrawPrimitives() = 0;	// override with Actor specific action
	virtual void EndDraw();		// pops transform from world matrix stack
	bool IsFirstUpdate();
	virtual void Update( float fDeltaTime );
	virtual void UpdateTweening( float fDeltaTime );

	virtual float GetX()					{ return DestTweenState().pos.x; };
	virtual float GetY()					{ return DestTweenState().pos.y; };
	virtual float GetZ()					{ return DestTweenState().pos.z; };
	virtual void  SetX( float x )			{ DestTweenState().pos.x = x; };
	virtual void  SetY( float y )			{ DestTweenState().pos.y = y; };
	virtual void  SetZ( float z )			{ DestTweenState().pos.z = z; };
	virtual void  SetXY( float x, float y )	{ DestTweenState().pos.x = x; DestTweenState().pos.y = y; };

	// height and width vary depending on zoom
	virtual float GetUnzoomedWidth()		{ return m_size.x; }
	virtual float GetUnzoomedHeight()		{ return m_size.y; }
	virtual float GetZoomedWidth()			{ return m_size.x * DestTweenState().scale.x; }
	virtual float GetZoomedHeight()			{ return m_size.y * DestTweenState().scale.y; }
//	virtual void  SetWidth( float width )	{ DestTweenState().size.x = width; }
//	virtual void  SetHeight( float height )	{ DestTweenState().size.y = height; }

	virtual float GetZoom()					{ return DestTweenState().scale.x; }	// not accurate in some cases
	virtual float GetZoomX()				{ return DestTweenState().scale.x; }
	virtual float GetZoomY()				{ return DestTweenState().scale.y; }
	virtual void  SetZoom( float zoom )		{ DestTweenState().scale.x = zoom;	DestTweenState().scale.y = zoom; }
	virtual void  SetZoomX( float zoom )	{ DestTweenState().scale.x = zoom;	}
	virtual void  SetZoomY( float zoom )	{ DestTweenState().scale.y = zoom; }
	virtual void  ZoomToWidth( float zoom )	{ SetZoomX( zoom / GetUnzoomedWidth() ); }
	virtual void  ZoomToHeight( float zoom ){ SetZoomY( zoom / GetUnzoomedHeight() ); }

	virtual float GetRotationX()			{ return DestTweenState().rotation.x; }
	virtual float GetRotationY()			{ return DestTweenState().rotation.y; }
	virtual float GetRotationZ()			{ return DestTweenState().rotation.z; }
	virtual void  SetRotationX( float rot )	{ DestTweenState().rotation.x = rot; }
	virtual void  SetRotationY( float rot )	{ DestTweenState().rotation.y = rot; }
	virtual void  SetRotationZ( float rot )	{ DestTweenState().rotation.z = rot; }

	virtual void SetDiffuse( RageColor c ) { for(int i=0; i<4; i++) DestTweenState().diffuse[i] = c; };
	virtual void SetDiffuses( int i, RageColor c )		{ DestTweenState().diffuse[i] = c; };
	virtual void SetDiffuseUpperLeft( RageColor c )		{ DestTweenState().diffuse[0] = c; };
	virtual void SetDiffuseUpperRight( RageColor c )	{ DestTweenState().diffuse[1] = c; };
	virtual void SetDiffuseLowerLeft( RageColor c )		{ DestTweenState().diffuse[2] = c; };
	virtual void SetDiffuseLowerRight( RageColor c )	{ DestTweenState().diffuse[3] = c; };
	virtual void SetDiffuseTopEdge( RageColor c )		{ DestTweenState().diffuse[0] = DestTweenState().diffuse[1] = c; };
	virtual void SetDiffuseRightEdge( RageColor c )		{ DestTweenState().diffuse[1] = DestTweenState().diffuse[3] = c; };
	virtual void SetDiffuseBottomEdge( RageColor c )	{ DestTweenState().diffuse[2] = DestTweenState().diffuse[3] = c; };
	virtual void SetDiffuseLeftEdge( RageColor c )		{ DestTweenState().diffuse[0] = DestTweenState().diffuse[2] = c; };
	virtual RageColor GetDiffuse()						{ return DestTweenState().diffuse[0]; };
	virtual RageColor GetDiffuses( int i )				{ return DestTweenState().diffuse[i]; };
	virtual void SetGlow( RageColor c )					{ DestTweenState().glow = c; };
	virtual RageColor GetGlow()							{ return DestTweenState().glow; };


	// TODO: Get rid of these once we're sure everything is working
	virtual void SetTweenX( float x )		{ SetX(x); }
	virtual void SetTweenY( float y )		{ SetY(y); }
	virtual void SetTweenZ( float z )		{ SetZ(z); }
	virtual void SetTweenXY( float x, float y )	{ SetXY(x,y); }
	virtual void SetTweenZoom( float zoom )		{ SetZoom(zoom); }
	virtual void SetTweenZoomX( float zoom )	{ SetZoomX(zoom); }
	virtual void SetTweenZoomY( float zoom )	{ SetZoomY(zoom); }
	virtual void SetTweenZoomToWidth( float zoom )	{ ZoomToWidth(zoom); }
	virtual void SetTweenZoomToHeight( float zoom )	{ ZoomToHeight(zoom); }
	virtual void SetTweenRotationX( float r )	{ SetRotationX(r); }
	virtual void SetTweenRotationY( float r )	{ SetRotationY(r); }
	virtual void SetTweenRotationZ( float r )	{ SetRotationZ(r); }
	virtual void SetTweenDiffuse( RageColor c )				{ SetDiffuse(c); }
	virtual void SetTweenDiffuseUpperLeft( RageColor c )	{ SetDiffuseUpperLeft(c); }
	virtual void SetTweenDiffuseUpperRight( RageColor c )	{ SetDiffuseUpperRight(c); }
	virtual void SetTweenDiffuseLowerLeft( RageColor c )	{ SetDiffuseLowerLeft(c); }
	virtual void SetTweenDiffuseLowerRight( RageColor c )	{ SetDiffuseLowerRight(c); }
	virtual void SetTweenDiffuseTopEdge( RageColor c )		{ SetDiffuseTopEdge(c); }
	virtual void SetTweenDiffuseRightEdge( RageColor c )	{ SetDiffuseRightEdge(c); }
	virtual void SetTweenDiffuseBottomEdge( RageColor c )	{ SetDiffuseBottomEdge(c); }
	virtual void SetTweenDiffuseLeftEdge( RageColor c )		{ SetDiffuseLeftEdge(c); }
	virtual void SetTweenGlow( RageColor c )				{ SetGlow(c); }


	virtual void BeginTweening( float time, TweenType tt = TWEEN_LINEAR );
	virtual void StopTweening();
	virtual float GetTweenTimeLeft() const;	// Amount of time until all tweens have stopped
	virtual TweenState& DestTweenState()	// where Actor will end when its tween finish
	{
		if( m_TweenStates.empty() )	// not tweening
			return m_current;
		else
			return LatestTween();
	}
	virtual void SetLatestTween( TweenState ts )	{ LatestTween() = ts; }

	
	enum StretchType { fit_inside, cover };

	void ScaleToCover( const RectI &rect )		{ ScaleTo( rect, cover ); }
	void ScaleToFitInside( const RectI &rect )	{ ScaleTo( rect, fit_inside); };
	void ScaleTo( const RectI &rect, StretchType st );

	void StretchTo( const RectI &rect );
	void StretchTo( const RectF &rect );




	enum HorizAlign { align_left, align_center, align_right };
	virtual void SetHorizAlign( HorizAlign ha ) { m_HorizAlign = ha; }
	virtual void SetHorizAlign( CString s )
	{
		s.MakeLower();
		if     (s=="left")		m_HorizAlign = align_left;
		else if(s=="center")	m_HorizAlign = align_center;
		else if(s=="right")		m_HorizAlign = align_right;
		else	ASSERT(0);
	}

	enum VertAlign { align_top, align_middle, align_bottom };
	virtual void SetVertAlign( VertAlign va ) { m_VertAlign = va; }
	virtual void SetVertAlign( CString s )
	{
		s.MakeLower();
		if     (s=="top")		m_VertAlign = align_top;
		else if(s=="middle")	m_VertAlign = align_middle;
		else if(s=="bottom")	m_VertAlign = align_bottom;
		else	ASSERT(0);
	}



	// effects
	void SetEffectNone()						{ m_Effect = no_effect; }
	Effect GetEffect()							{ return m_Effect; }
	void SetEffectColor1( RageColor c )			{ m_effectColor1 = c; }
	void SetEffectColor2( RageColor c )			{ m_effectColor2 = c; }
	void SetEffectPeriod( float fSecs )			{ m_fEffectPeriodSeconds = fSecs; } 
	void SetEffectMagnitude( RageVector3 vec )	{ m_vEffectMagnitude = vec; }

	void SetEffectDiffuseBlink( 
		float fEffectPeriodSeconds = 1.0f,
		RageColor c1 = RageColor(0.5f,0.5f,0.5f,1), 
		RageColor c2 = RageColor(1,1,1,1) );
	void SetEffectDiffuseShift( float fEffectPeriodSeconds = 1.f,
		RageColor c1 = RageColor(0,0,0,1), 
		RageColor c2 = RageColor(1,1,1,1) );
	void SetEffectGlowBlink( float fEffectPeriodSeconds = 1.f,
		RageColor c1 = RageColor(1,1,1,0.2f),
		RageColor c2 = RageColor(1,1,1,0.8f) );
	void SetEffectGlowShift( 
		float fEffectPeriodSeconds = 1.0f,
		RageColor c1 = RageColor(1,1,1,0.2f),
		RageColor c2 = RageColor(1,1,1,0.8f) );
	void SetEffectWag( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,0.2f) );
	void SetEffectBounce( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,20) );
	void SetEffectBob( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,20) );
	void SetEffectSpin( 
		RageVector3 vect = RageVector3(0,0,1) );
	void SetEffectVibrate( 
		RageVector3 vect = RageVector3(10,10,10) );


	//
	// other properties
	//
	void SetShadowLength( float fLength )
	{
		if( fLength==0 )
			m_bShadow = false;
		else
		{
			m_fShadowLength = fLength;
			m_bShadow = true;
		}
	}
	void EnableShadow( bool b )	{ m_bShadow = b; };


	void EnableAdditiveBlend( bool b ) 		{ m_bBlendAdd = b; }; 


	//
	// fade command
	//
	void Fade( float fSleepSeconds, CString sFadeString, float fFadeSeconds, bool bFadingOff );
	void FadeOn( float fSleepSeconds, CString sFadeString, float fFadeSeconds )	{ Fade(fSleepSeconds,sFadeString,fFadeSeconds,false); };
	void FadeOff( float fSleepSeconds, CString sFadeString, float fFadeSeconds )	{ Fade(fSleepSeconds,sFadeString,fFadeSeconds,true); };

	void Command( CString sCommandString );


protected:

	struct TweenInfo
	{
		// counters for tweening
		TweenType	m_TweenType;
		float		m_fTimeLeftInTween;	// how far into the tween are we?
		float		m_fTweenTime;		// seconds between Start and End positions/zooms
	};


	// only called by Sprite
	void  SetBaseZoomX( float zoom )	{ m_baseScale.x = zoom;	}
	void  SetBaseZoomY( float zoom )	{ m_baseScale.y = zoom; }
	void  SetBaseZoomZ( float zoom )	{ m_baseScale.z = zoom; }
	virtual void  SetBaseRotationX( float rot )	{ m_baseRotation.x = rot; }
	virtual void  SetBaseRotationY( float rot )	{ m_baseRotation.y = rot; }
	virtual void  SetBaseRotationZ( float rot )	{ m_baseRotation.z = rot; }

	RageVector3	m_baseRotation;
	RageVector3	m_baseScale;


	RageVector2	m_size;
	TweenState	m_current;
	TweenState	m_start;
	vector<TweenState>	m_TweenStates;
	vector<TweenInfo>	m_TweenInfo;
	TweenState& LatestTween() { ASSERT(m_TweenStates.size()>0);	return m_TweenStates.back(); };

	//
	// Temporary variables that are filled just before drawing
	//
	TweenState m_temp;



	bool	m_bFirstUpdate;

	//
	// Stuff for alignment
	//
	HorizAlign	m_HorizAlign;
	VertAlign	m_VertAlign;


	//
	// Stuff for effects
	//
	Effect m_Effect;
	float m_fSecsIntoEffect;
	float m_fEffectPeriodSeconds;
	RageColor   m_effectColor1;
	RageColor   m_effectColor2;
	RageVector3 m_vEffectMagnitude;


	//
	// other properties
	//
	bool	m_bShadow;
	float	m_fShadowLength;
	bool	m_bBlendAdd;

};

#endif
