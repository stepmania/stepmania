/* Actor - Base class for all objects that appear on the screen. */

#ifndef ACTOR_H
#define ACTOR_H

#include "RageTypes.h"
#include "Command.h"
#include <deque>
#include <map>

#define DRAW_ORDER_BEFORE_EVERYTHING	-100
#define DRAW_ORDER_TRANSITIONS			100
#define DRAW_ORDER_AFTER_EVERYTHING		200

class Actor
{
public:
	Actor();
	virtual ~Actor() {}
	virtual void Reset();

	static void SetBGMTime( float fTime, float fBeat ) { g_fCurrentBGMTime = fTime; g_fCurrentBGMBeat = fBeat; }

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
				rainbow,
				wag,	bounce,		bob,	pulse,
				spin,	vibrate
				};

	struct TweenState
	{
		// start and end position for tweening
		RageVector3 pos;
		RageVector3 rotation;
		RageVector4 quat;
		RageVector3 scale;
		RectF		crop;	// 0 = no cropping, 1 = fully cropped
		RectF		fade;	// 0 = no fade
		RageColor   diffuse[4];
		RageColor   glow;
		Command	command;	// command to execute when this 

		void Init();
		static void MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween );
	};

	enum EffectClock { CLOCK_TIMER, CLOCK_BGM_TIME, CLOCK_BGM_BEAT, NUM_CLOCKS };

	void Draw();						// calls, NeedsDraw, BeginDraw, DrawPrimitives, EndDraw
	virtual bool EarlyAbortDraw() { return false; }	// return true to early abort drawing of this Actor
	virtual void BeginDraw();			// pushes transform onto world matrix stack
	virtual void SetRenderStates();		// Actor should call at beginning of their DrawPrimitives() after setting textures
	virtual void DrawPrimitives() {};	// Derivitives should override
	virtual void EndDraw();				// pops transform from world matrix stack
	
	bool IsFirstUpdate() const;
	virtual void Update( float fDeltaTime );
	void UpdateTweening( float fDeltaTime );
	void CopyTweening( const Actor &from );


	CString m_sName, m_sID;

	/* m_sName is the name actors use to look up internal metrics for themselves, and for
	 * filenames.  m_sID is the name parents use to look up their own metrics for an actor
	 * (usually via ActorUtil).  (This is experimental; see DifficultyMeter.cpp for more
	 * information.) */
	const CString &GetName() const		{ return m_sName; }
	const CString &GetID() const		{ return m_sID.empty() ? m_sName : m_sID; }
	void SetName( const CString &sName, const CString &sID = "" ) { m_sName = sName; m_sID = sID; }



	/* Do subclasses really need to override tweening?  Tween data should
	 * probably be private ... - glenn */
	/* Things like a "FocusingSprite" might, but probably not.  -Chris */

	/* Return the current coordinates, not the destination coordinates;
	 * that's what the old behavior was, at least, and it's what ScreenMusicScroll
	 * expects.  I could see uses for knowing the destination coords, though,
	 * especially now that setting parameters when tweening and when not tweening
	 * is somewhat abstracted.  Hmmm. -glenn */
	/* Things like the cursor on ScreenSelectDifficutly need to know the dest coordinates.
	 * -Chris */
	float GetX() const				{ return m_current.pos.x; };
	float GetY() const				{ return m_current.pos.y; };
	float GetZ() const				{ return m_current.pos.z; };
	float GetDestX()				{ return DestTweenState().pos.x; };
	float GetDestY()				{ return DestTweenState().pos.y; };
	float GetDestZ()				{ return DestTweenState().pos.z; };
	void  SetX( float x )			{ DestTweenState().pos.x = x; };
	void  SetY( float y )			{ DestTweenState().pos.y = y; };
	void  SetZ( float z )			{ DestTweenState().pos.z = z; };
	void  SetXY( float x, float y )	{ DestTweenState().pos.x = x; DestTweenState().pos.y = y; };
	void  AddX( float x )			{ SetX( GetDestX()+x ); }
	void  AddY( float y )			{ SetY( GetDestY()+y ); }
	void  AddZ( float z )			{ SetZ( GetDestZ()+z ); }

	// height and width vary depending on zoom
	float GetUnzoomedWidth()		{ return m_size.x; }
	float GetUnzoomedHeight()		{ return m_size.y; }
	float GetZoomedWidth()			{ return m_size.x * m_baseScale.x * DestTweenState().scale.x; }
	float GetZoomedHeight()			{ return m_size.y * m_baseScale.y * DestTweenState().scale.y; }
	void  SetWidth( float width )	{ m_size.x = width; }
	void  SetHeight( float height )	{ m_size.y = height; }

	void  SetBaseZoomX( float zoom )	{ m_baseScale.x = zoom;	}
	void  SetBaseZoomY( float zoom )	{ m_baseScale.y = zoom; }
	void  SetBaseZoomZ( float zoom )	{ m_baseScale.z = zoom; }
	void  SetBaseZoom( const RageVector3 &zoom ) { m_baseScale = zoom; }
	void  SetBaseRotationX( float rot )	{ m_baseRotation.x = rot; }
	void  SetBaseRotationY( float rot )	{ m_baseRotation.y = rot; }
	void  SetBaseRotationZ( float rot )	{ m_baseRotation.z = rot; }
	void  SetBaseRotation( const RageVector3 &rot )	{ m_baseRotation = rot; }

	float GetZoom()					{ return DestTweenState().scale.x; }	// not accurate in some cases
	float GetZoomX()				{ return DestTweenState().scale.x; }
	float GetZoomY()				{ return DestTweenState().scale.y; }
	float GetZoomZ()				{ return DestTweenState().scale.z; }
	void  SetZoom( float zoom )		{ DestTweenState().scale.x = zoom;	DestTweenState().scale.y = zoom; }
	void  SetZoomX( float zoom )	{ DestTweenState().scale.x = zoom;	}
	void  SetZoomY( float zoom )	{ DestTweenState().scale.y = zoom; }
	void  SetZoomZ( float zoom )	{ DestTweenState().scale.z = zoom; }
	void  ZoomToWidth( float zoom )	{ SetZoomX( zoom / GetUnzoomedWidth() ); }
	void  ZoomToHeight( float zoom ){ SetZoomY( zoom / GetUnzoomedHeight() ); }

	float GetRotationX()			{ return DestTweenState().rotation.x; }
	float GetRotationY()			{ return DestTweenState().rotation.y; }
	float GetRotationZ()			{ return DestTweenState().rotation.z; }
	void  SetRotationX( float rot )	{ DestTweenState().rotation.x = rot; }
	void  SetRotationY( float rot )	{ DestTweenState().rotation.y = rot; }
	void  SetRotationZ( float rot )	{ DestTweenState().rotation.z = rot; }
	void  AddRotationH( float rot );
	void  AddRotationP( float rot );
	void  AddRotationR( float rot );

	float GetCropLeft()					{ return DestTweenState().crop.left; }
	float GetCropTop()					{ return DestTweenState().crop.top;	}
	float GetCropRight()				{ return DestTweenState().crop.right;}
	float GetCropBottom()				{ return DestTweenState().crop.bottom;}
	void  SetCropLeft( float percent )	{ DestTweenState().crop.left = percent; }
	void  SetCropTop( float percent )	{ DestTweenState().crop.top = percent;	}
	void  SetCropRight( float percent )	{ DestTweenState().crop.right = percent;}
	void  SetCropBottom( float percent ){ DestTweenState().crop.bottom = percent;}

	void  SetFadeLeft( float percent )	{ DestTweenState().fade.left = percent; }
	void  SetFadeTop( float percent )	{ DestTweenState().fade.top = percent;	}
	void  SetFadeRight( float percent )	{ DestTweenState().fade.right = percent;}
	void  SetFadeBottom( float percent ){ DestTweenState().fade.bottom = percent;}

	void SetGlobalDiffuseColor( RageColor c );
	void SetGlobalX( float x );

	virtual void SetDiffuse( RageColor c ) { for(int i=0; i<4; i++) DestTweenState().diffuse[i] = c; };
	virtual void SetDiffuseAlpha( float f ) { for(int i = 0; i < 4; ++i) { RageColor c = GetDiffuses( i ); c.a = f; SetDiffuses( i, c ); } }
	void SetDiffuseColor( RageColor c );
	void SetDiffuses( int i, RageColor c )		{ DestTweenState().diffuse[i] = c; };
	void SetDiffuseUpperLeft( RageColor c )		{ DestTweenState().diffuse[0] = c; };
	void SetDiffuseUpperRight( RageColor c )	{ DestTweenState().diffuse[1] = c; };
	void SetDiffuseLowerLeft( RageColor c )		{ DestTweenState().diffuse[2] = c; };
	void SetDiffuseLowerRight( RageColor c )	{ DestTweenState().diffuse[3] = c; };
	void SetDiffuseTopEdge( RageColor c )		{ DestTweenState().diffuse[0] = DestTweenState().diffuse[1] = c; };
	void SetDiffuseRightEdge( RageColor c )		{ DestTweenState().diffuse[1] = DestTweenState().diffuse[3] = c; };
	void SetDiffuseBottomEdge( RageColor c )	{ DestTweenState().diffuse[2] = DestTweenState().diffuse[3] = c; };
	void SetDiffuseLeftEdge( RageColor c )		{ DestTweenState().diffuse[0] = DestTweenState().diffuse[2] = c; };
	RageColor GetDiffuse()						{ return DestTweenState().diffuse[0]; };
	RageColor GetDiffuses( int i )				{ return DestTweenState().diffuse[i]; };
	void SetGlow( RageColor c )					{ DestTweenState().glow = c; };
	RageColor GetGlow()							{ return DestTweenState().glow; };


	void BeginTweening( float time, TweenType tt = TWEEN_LINEAR );
	void StopTweening();
	void Sleep( float time );
	void QueueCommand( const Command& command );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );
	// Let ActorFrame and BGAnimation override
	virtual float GetTweenTimeLeft() const;	// Amount of time until all tweens have stopped
	TweenState& DestTweenState()	// where Actor will end when its tween finish
	{
		if( m_Tweens.empty() )	// not tweening
			return m_current;
		else
			return LatestTween();
	}
	void SetLatestTween( TweenState ts )	{ LatestTween() = ts; }

	
	enum StretchType { fit_inside, cover };

	void ScaleToCover( const RectF &rect )		{ ScaleTo( rect, cover ); }
	void ScaleToFitInside( const RectF &rect )	{ ScaleTo( rect, fit_inside); };
	void ScaleTo( const RectF &rect, StretchType st );

	void StretchTo( const RectF &rect );


	//
	// Alignment settings.  These need to be virtual for BitmapText
	//
	enum HorizAlign { align_left, align_center, align_right };
	virtual void SetHorizAlign( HorizAlign ha ) { m_HorizAlign = ha; }
	virtual void SetHorizAlign( const CString &s );

	enum VertAlign { align_top, align_middle, align_bottom };
	virtual void SetVertAlign( VertAlign va ) { m_VertAlign = va; }
	virtual void SetVertAlign( const CString &s );


	//
	// effects
	//
	void SetEffectNone()						{ m_Effect = no_effect; }
	Effect GetEffect()							{ return m_Effect; }
	void SetEffectColor1( RageColor c )			{ m_effectColor1 = c; }
	void SetEffectColor2( RageColor c )			{ m_effectColor2 = c; }
	void SetEffectPeriod( float fSecs )			{ m_fEffectPeriodSeconds = fSecs; } 
	void SetEffectDelay( float fTime )			{ m_fEffectDelay = fTime; }
	void SetEffectOffset( float fPercent )		{ m_fEffectOffset = fPercent; }
	void SetEffectClock( EffectClock c )		{ m_EffectClock = c; }
	void SetEffectClock( const CString &s );

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
	void SetEffectRainbow( 
		float fEffectPeriodSeconds = 2.0f );
	void SetEffectWag( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,20) );
	void SetEffectBounce( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,20) );
	void SetEffectBob( 
		float fPeriod = 2.f, 
		RageVector3 vect = RageVector3(0,0,20) );
	void SetEffectPulse( 
		float fPeriod = 2.f,
		float fMinZoom = 0.5f,
		float fMaxZoom = 1.f );
	void SetEffectSpin( 
		RageVector3 vect = RageVector3(0,0,180) );
	void SetEffectVibrate( 
		RageVector3 vect = RageVector3(10,10,10) );


	//
	// other properties
	//
	bool GetHidden() const				{ return m_bHidden; }
	void SetHidden( bool b )			{ m_bHidden = b; }
	void SetShadowLength( float fLength );
	// TODO: Implement hibernate as a tween type?
	void SetHibernate( float fSecs )	{ m_fHibernateSecondsLeft = fSecs; }
	void SetDrawOrder( int iOrder )		{ m_iDrawOrder = iOrder; }
	int GetDrawOrder() const			{ return m_iDrawOrder; }

	virtual void EnableAnimation( bool b ) 		{ m_bIsAnimating = b; }	// Sprite needs to overload this
	void StartAnimating()		{ this->EnableAnimation(true); };
	void StopAnimating()		{ this->EnableAnimation(false); };


	//
	// render states
	//
	void SetBlendMode( BlendMode mode )			{ m_BlendMode = mode; } 
	void SetBlendMode( const CString &s );
	void SetTextureWrapping( bool b ) 			{ m_bTextureWrapping = b; } 
	void SetClearZBuffer( bool b ) 				{ m_bClearZBuffer = b; } 
	void SetUseZBuffer( bool b ) 				{ SetZTestMode(b?ZTEST_WRITE_ON_PASS:ZTEST_OFF); SetZWrite(b); } 
	virtual void SetZTestMode( ZTestMode mode ) { m_ZTestMode = mode; } 
	virtual void SetZTestMode( const CString &s );
	virtual void SetZWrite( bool b ) 			{ m_bZWrite = b; } 
	virtual void SetCullMode( CullMode mode ) 	{ m_CullMode = mode; } 
	virtual void SetCullMode( const CString &s );

	//
	// Commands
	//
	void AddCommands( const CString sName, const Commands &cmds );
	void RunCommands( const Commands &cmds );
	virtual void HandleCommand( const Command &command );	// derivable
	static float GetCommandsLengthSeconds( const Commands &cmds );

	//
	// Animation
	//
	virtual int GetNumStates() const { return 1; }
	virtual void SetState( int iNewState ) {}
	virtual float GetAnimationLengthSeconds() const { return 0; }
	virtual void SetSecondsIntoAnimation( float fSeconds ) {}

	//
	// BGAnimation stuff
	//
	virtual void GainFocus( float fRate, bool bRewindMovie, bool bLoop ) {}
	virtual void LoseFocus() {}
	virtual void PlayCommand( const CString &sCommandName );

protected:

	struct TweenInfo
	{
		// counters for tweening
		TweenType	m_TweenType;
		float		m_fTimeLeftInTween;	// how far into the tween are we?
		float		m_fTweenTime;		// seconds between Start and End positions/zooms
	};


	RageVector3	m_baseRotation;
	RageVector3	m_baseScale;


	RageVector2	m_size;
	TweenState	m_current;
	TweenState	m_start;
	struct TweenStateAndInfo
	{
		TweenState state;
		TweenInfo info;
	};
	deque<TweenStateAndInfo>	m_Tweens;	// use deque for contant time delete of the head

	TweenState& LatestTween() { ASSERT(m_Tweens.size()>0);	return m_Tweens.back().state; };

	//
	// Temporary variables that are filled just before drawing
	//
	TweenState m_tempState;
	TweenState *m_pTempState;

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
	float m_fSecsIntoEffect, m_fEffectDelta;
	float m_fEffectPeriodSeconds;
	float m_fEffectDelay;
	float m_fEffectOffset;
	EffectClock m_EffectClock;

	/* This can be used in lieu of the fDeltaTime parameter to Update() to
	 * follow the effect clock.  Actor::Update must be called first. */
	float GetEffectDeltaTime() const { return m_fEffectDelta; }

	RageColor   m_effectColor1;
	RageColor   m_effectColor2;
	RageVector3 m_vEffectMagnitude;


	//
	// other properties
	//
	bool	m_bHidden;
	float	m_fHibernateSecondsLeft;
	float	m_fShadowLength;	// 0 == no shadow
	bool	m_bIsAnimating;
	int		m_iDrawOrder;

	//
	// render states
	//
	bool		m_bTextureWrapping;
	BlendMode	m_BlendMode;
	bool		m_bClearZBuffer;
	ZTestMode	m_ZTestMode;
	bool		m_bZWrite;
	CullMode	m_CullMode;

	//
	// global state
	//
	static float g_fCurrentBGMTime, g_fCurrentBGMBeat;

	//
	// commands
	//
	map<CString, Commands> m_mapNameToCommands;
};

#endif

/*
 * (c) 2001-2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
