#pragma once
/*
-----------------------------------------------------------------------------
 Class: Actor

 Desc: Base class for all objects that appear on the screen.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include <d3dx8math.h>


const int MAX_TWEEN_STATES = 10;

class Actor
{
public:
	Actor();



	enum TweenType { 
		TWEEN_LINEAR, 
		TWEEN_BIAS_BEGIN, 
		TWEEN_BIAS_END, 
		TWEEN_BOUNCE_BEGIN, 
		TWEEN_BOUNCE_END,
		TWEEN_SPRING,
	};
	enum Effect { no_effect,
				blinking,	camelion,   glowing,
				wagging,	spinning,
				vibrating,	flickering,
				bouncing,	bobbing
				};

	// let subclasses override
	virtual void Restore() {};
	virtual void Invalidate() {};

	virtual void Draw();		// calls, BeginDraw, DrawPrimitives, EndDraw
	virtual void BeginDraw();	// pushes transform onto world matrix stack
	virtual void DrawPrimitives() = 0;	// override with Actor specific action
	virtual void EndDraw();		// pops transform from world matrix stack
	virtual void Update( float fDeltaTime );

	virtual float GetX()					{ return m_pos.x; };
	virtual float GetY()					{ return m_pos.y; };
	virtual float GetZ()					{ return m_pos.z; };
	virtual void  SetX( float x )			{ m_pos.x = x; };
	virtual void  SetY( float y )			{ m_pos.y = y; };
	virtual void  SetZ( float z )			{ m_pos.z = z; };
	virtual void  SetXY( float x, float y )	{ m_pos.x = x;	m_pos.y = y; };

	// height and width vary depending on zoom
	virtual float GetUnzoomedWidth()		{ return m_size.x; }
	virtual float GetUnzoomedHeight()		{ return m_size.y; }
	virtual float GetZoomedWidth()			{ return m_size.x * m_scale.x; }
	virtual float GetZoomedHeight()		{ return m_size.y * m_scale.y; }
	virtual void  SetWidth( float width ){ m_size.x = width; }
	virtual void  SetHeight( float height ){ m_size.y = height; }

	virtual float GetZoom()				{ return m_scale.x; }
	virtual float GetZoomX()			{ return m_scale.x; }
	virtual float GetZoomY()			{ return m_scale.y; }
	virtual void  SetZoom( float zoom )	{ m_scale.x = zoom;	m_scale.y = zoom; }
	virtual void  SetZoomX( float zoom ){ m_scale.x = zoom;	}
	virtual void  SetZoomY( float zoom ){ m_scale.y = zoom; }

	virtual float GetRotation()				{ return m_rotation.z; }
	virtual float GetRotationX()			{ return m_rotation.x; }
	virtual float GetRotationY()			{ return m_rotation.y; }
	virtual void  SetRotation( float rot )	{ m_rotation.z = rot; }
	virtual void  SetRotationX( float rot )	{ m_rotation.x = rot; }
	virtual void  SetRotationY( float rot )	{ m_rotation.y = rot; }

	virtual void SetDiffuseColor( D3DXCOLOR colorDiffuse ) { for(int i=0; i<4; i++) m_colorDiffuse[i] = colorDiffuse; };
	virtual void SetDiffuseColors( int i, D3DXCOLOR colorDiffuse ) { m_colorDiffuse[i] = colorDiffuse; };
	virtual void SetDiffuseColorUpperLeft( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[0] = colorDiffuse; };
	virtual void SetDiffuseColorUpperRight( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[1] = colorDiffuse; };
	virtual void SetDiffuseColorLowerLeft( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[2] = colorDiffuse; };
	virtual void SetDiffuseColorLowerRight( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[3] = colorDiffuse; };
	virtual void SetDiffuseColorTopEdge( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[0] = m_colorDiffuse[1] = colorDiffuse; };
	virtual void SetDiffuseColorRightEdge( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[1] = m_colorDiffuse[3] = colorDiffuse; };
	virtual void SetDiffuseColorBottomEdge( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[2] = m_colorDiffuse[3] = colorDiffuse; };
	virtual void SetDiffuseColorLeftEdge( D3DXCOLOR colorDiffuse ) { m_colorDiffuse[0] = m_colorDiffuse[2] = colorDiffuse; };
	virtual D3DXCOLOR GetDiffuseColor()				{ return m_colorDiffuse[0]; };
	virtual D3DXCOLOR GetDiffuseColors( int i )		{ return m_colorDiffuse[i]; };
	virtual void SetGlowColor( D3DXCOLOR colorGlow ) { m_colorGlow = colorGlow; };
	virtual D3DXCOLOR GetGlowColor()				{ return m_colorGlow; };



	virtual void BeginTweening( float time, TweenType tt = TWEEN_LINEAR );
	virtual void BeginTweeningQueued( float time, TweenType tt = TWEEN_LINEAR );
	virtual void StopTweening();
	virtual void SetTweenX( float x );
	virtual void SetTweenY( float y );
	virtual void SetTweenZ( float z );
	virtual void SetTweenXY( float x, float y );
	virtual void SetTweenZoom( float zoom );
	virtual void SetTweenZoomX( float zoom );
	virtual void SetTweenZoomY( float zoom );
	virtual void SetTweenRotationX( float r );
	virtual void SetTweenRotationY( float r );
	virtual void SetTweenRotationZ( float r );
	virtual void SetTweenDiffuseColor( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorUpperLeft( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorUpperRight( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorLowerLeft( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorLowerRight( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorTopEdge( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorRightEdge( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorBottomEdge( D3DXCOLOR colorDiffuse );
	virtual void SetTweenDiffuseColorLeftEdge( D3DXCOLOR colorDiffuse );
	virtual void SetTweenAddColor( D3DXCOLOR c );


	
	enum StretchType { fit_inside, cover };

	void ScaleToCover( LPRECT rect )		{ ScaleTo( rect, cover ); };
	void ScaleToFitInside( LPRECT rect )	{ ScaleTo( rect, fit_inside); };
	void ScaleTo( LPRECT rect, StretchType st );

	void StretchTo( LPRECT rect );




	enum HorizAlign { align_left, align_center, align_right };
	void SetHorizAlign( HorizAlign ha ) { m_HorizAlign = ha; };

	enum VertAlign { align_top, align_middle, align_bottom };
	void SetVertAlign( VertAlign va ) { m_VertAlign = va; };



	// effects
	void SetEffectNone();
	void SetEffectBlinking( float fDeltaPercentPerSecond = 2.5,
						    D3DXCOLOR Color  = D3DXCOLOR(0.5f,0.5f,0.5f,1), 
						    D3DXCOLOR Color2 = D3DXCOLOR(1,1,1,1) );
	void SetEffectCamelion( float fDeltaPercentPerSecond = 2.5,
						    D3DXCOLOR Color  = D3DXCOLOR(0,0,0,1), 
						    D3DXCOLOR Color2 = D3DXCOLOR(1,1,1,1) );
	void SetEffectGlowing( float fDeltaPercentPerSecond = 2.5,
						   D3DXCOLOR Color  = D3DXCOLOR(1,1,1,0.2f),
						   D3DXCOLOR Color2 = D3DXCOLOR(1,1,1,0.8f) );
	void SetEffectWagging( float fWagRadians =  0.2,
						   float fWagPeriod = 2.0 );
	void SetEffectSpinning( float fRadsPerSpeed = 2.0 );
	void SetEffectVibrating( float fVibrationDistance = 5.0 );
	void SetEffectFlickering();
	void SetEffectBouncing( D3DXVECTOR3 vectBounceDir, float fPeriod );
	void SetEffectBobbing( D3DXVECTOR3 vectBobDir, float fPeriod );
	Effect GetEffect() { return m_Effect; };


	// other properties
	void TurnShadowOn()		{ m_bShadow = true; };
	void TurnShadowOff()	{ m_bShadow = false; };
	void SetShadowLength( const float fLength )	{ m_fShadowLength = fLength; };

	void SetBlendModeAdd() 		{ m_bBlendAdd = true; }; 
	void SetBlendModeNormal() 	{ m_bBlendAdd = false; };


protected:

	D3DXVECTOR2 m_size;		// width, height
	D3DXVECTOR3 m_pos;		// X-Y coordinate of where the center point will appear on screen
	D3DXVECTOR3 m_rotation;	// X, Y, and Z m_rotation
	D3DXVECTOR2 m_scale;	// X and Y zooming
	D3DXCOLOR   m_colorDiffuse[4];	// 4 corner colors - left to right, top to bottom
	D3DXCOLOR   m_colorGlow;

	//
	// Stuff for tweening
	//
	D3DXVECTOR3 m_start_pos;
	D3DXVECTOR3 m_start_rotation;
	D3DXVECTOR2 m_start_scale;
	D3DXCOLOR   m_start_colorDiffuse[4];
	D3DXCOLOR   m_start_colorGlow;

	struct TweenState
	{
		// start and end position for tweening
		D3DXVECTOR3 m_end_pos;
		D3DXVECTOR3 m_end_rotation;
		D3DXVECTOR2 m_end_scale;
		D3DXCOLOR   m_end_colorDiffuse[4];
		D3DXCOLOR   m_end_colorGlow;

		// counters for tweening
		TweenType	m_TweenType;
		float		m_fTimeLeftInTween;	// how far into the tween are we?
		float		m_fTweenTime;		// seconds between Start and End positions/zooms

		TweenState()
		{
			m_end_pos		= D3DXVECTOR3( 0, 0, 0 );
			m_end_rotation	= D3DXVECTOR3( 0, 0, 0 );
			m_end_scale		= D3DXVECTOR2( 1, 1 );
			for(int i=0; i<4; i++) m_end_colorDiffuse[i]= D3DXCOLOR( 1, 1, 1, 1 );
			m_end_colorGlow	= D3DXCOLOR( 0, 0, 0, 0 );
		};
	};

	TweenState	m_QueuedTweens[MAX_TWEEN_STATES];
	int			m_iNumTweenStates;
	TweenState& GetLatestTween() { ASSERT(m_iNumTweenStates>0 && m_iNumTweenStates<MAX_TWEEN_STATES);	return m_QueuedTweens[m_iNumTweenStates-1]; };


	//
	// Temporary variables that are filled just before drawing
	//
	D3DXVECTOR2 m_temp_size;
	D3DXVECTOR3 m_temp_pos;
	D3DXVECTOR3 m_temp_rotation;
	D3DXVECTOR2 m_temp_scale;
	D3DXCOLOR   m_temp_colorDiffuse[4];
	D3DXCOLOR   m_temp_colorGlow;


	//
	// Stuff for alignment
	//
	HorizAlign	m_HorizAlign;
	VertAlign	m_VertAlign;


	//
	// Stuff for effects
	//
	Effect m_Effect;


	// Counting variables for camelion and glowing:
	D3DXCOLOR   m_effect_colorDiffuse1;
	D3DXCOLOR   m_effect_colorDiffuse2;
	D3DXCOLOR   m_effect_colorGlow1;
	D3DXCOLOR   m_effect_colorGlow2;
	float m_fPercentBetweenColors;
	bool  m_bTweeningTowardEndColor;	// TRUE is fading toward end_color, FALSE if fading toward start_color
	float m_fDeltaPercentPerSecond;	// percentage change in tweening per second

	// wagging:
	float m_fWagRadians;
	float m_fWagPeriod;		// seconds to complete a wag (back and forth)
	float m_fWagTimer;		// num of seconds into this wag

	// spinning:
	float m_fSpinSpeed;		// radians per second

	// vibrating:
	float m_fVibrationDistance;

	// flickering:
	bool m_bVisibleThisFrame;

	// bouncing:
	D3DXVECTOR3 m_vectBounce;
	float m_fBouncePeriod;
	float m_fTimeIntoBounce;


	//
	// other properties
	//
	bool	m_bShadow;
	float	m_fShadowLength;
	bool	m_bBlendAdd;

};
