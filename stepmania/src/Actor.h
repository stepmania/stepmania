/* Actor - Base class for all objects that appear on the screen. */

#ifndef ACTOR_H
#define ACTOR_H

#include "RageTypes.h"
#include "ActorCommands.h"
#include <deque>
#include <map>
struct XNode;
struct lua_State;
class LuaReference;
#include "LuaBinding.h"
#include "MessageManager.h"


#define DRAW_ORDER_BEFORE_EVERYTHING	-200
#define DRAW_ORDER_UNDERLAY				-100
// normal screen elements go here
#define DRAW_ORDER_OVERLAY				+100
#define DRAW_ORDER_TRANSITIONS			+110
#define DRAW_ORDER_AFTER_EVERYTHING		+200


class Actor : public IMessageSubscriber
{
public:
	Actor();
	virtual ~Actor();
	void UnsubcribeAndClearCommands();
	virtual void Reset();
	void LoadFromNode( const CString& sDir, const XNode* pNode );

	static void SetBGMTime( float fTime, float fBeat );
	static void SetBGMLights( const float *abCabinetLights );

	enum TweenType { 
		TWEEN_LINEAR, 
		TWEEN_ACCELERATE, 
		TWEEN_DECELERATE, 
		TWEEN_BOUNCE_BEGIN, 
		TWEEN_BOUNCE_END,
		TWEEN_SPRING,
	};
	enum Effect { no_effect, effect_lua,
				diffuse_blink,	diffuse_shift,	diffuse_ramp,
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
		CString sCommandName;	// command to execute when this TweenState goes into effect

		void Init();
		static void MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween );
	};

	enum EffectClock
	{
		CLOCK_TIMER,
		CLOCK_BGM_TIME,
		CLOCK_BGM_BEAT,
		CLOCK_LIGHT_1 = 1000,
		CLOCK_LIGHT_LAST = 1100,
		NUM_CLOCKS
	};

	void Draw();						// calls, NeedsDraw, BeginDraw, DrawPrimitives, EndDraw
	virtual bool EarlyAbortDraw() { return false; }	// return true to early abort drawing of this Actor
	virtual void BeginDraw();			// pushes transform onto world matrix stack
	virtual void SetGlobalRenderStates();		// Actor should call this at beginning of their DrawPrimitives()
	virtual void SetTextureRenderStates();		// Actor should call this after setting a texture
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
	virtual void SetName( const CString &sName, const CString &sID = "" ) { m_sName = sName; m_sID = sID; }



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

	float GetBaseZoomX()				{ return m_baseScale.x;	}
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
	void  ZoomTo( float fX, float fY )	{ ZoomToWidth(fX); ZoomToHeight(fY); }
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
	void QueueCommand( const CString& sCommandName );
	void QueueMessage( const CString& sMessageName );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );
	// Let ActorFrame and BGAnimation override
	virtual float GetTweenTimeLeft() const;	// Amount of time until all tweens have stopped
	TweenState& DestTweenState()	// where Actor will end when its tween finish
	{
		if( m_pTempState != NULL ) // effect_lua running
			return *m_pTempState;
		else if( m_Tweens.empty() )	// not tweening
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
	virtual void SetHorizAlignString( const CString &s );	// convenience

	enum VertAlign { align_top, align_middle, align_bottom };
	virtual void SetVertAlign( VertAlign va ) { m_VertAlign = va; }
	virtual void SetVertAlignString( const CString &s );	// convenience


	//
	// effects
	//
	void SetEffectNone()						{ m_Effect = no_effect; }
	Effect GetEffect() const					{ return m_Effect; }
	float GetSecsIntoEffect() const				{ return m_fSecsIntoEffect; }
	float GetEffectDelta() const				{ return m_fEffectDelta; }

	void SetEffectColor1( RageColor c )			{ m_effectColor1 = c; }
	void SetEffectColor2( RageColor c )			{ m_effectColor2 = c; }
	void SetEffectPeriod( float fSecs )			{ m_fEffectPeriodSeconds = fSecs; } 
	void SetEffectDelay( float fTime )			{ m_fEffectDelay = fTime; }
	void SetEffectOffset( float fPercent )		{ m_fEffectOffset = fPercent; }
	void SetEffectClock( EffectClock c )		{ m_EffectClock = c; }
	void SetEffectClockString( const CString &s );	// convenience

	void SetEffectMagnitude( RageVector3 vec )	{ m_vEffectMagnitude = vec; }

	void SetEffectLua( const CString &sCommand );
	void SetEffectDiffuseBlink( 
		float fEffectPeriodSeconds = 1.0f,
		RageColor c1 = RageColor(0.5f,0.5f,0.5f,1), 
		RageColor c2 = RageColor(1,1,1,1) );
	void SetEffectDiffuseShift( float fEffectPeriodSeconds = 1.f,
		RageColor c1 = RageColor(0,0,0,1), 
		RageColor c2 = RageColor(1,1,1,1) );
	void SetEffectDiffuseRamp( float fEffectPeriodSeconds = 1.f,
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
	bool GetVisible() const				{ return m_bVisible; }
	bool GetHidden() const				{ return !m_bVisible; }
	void SetVisible( bool b )			{ m_bVisible = b; }
	void SetHidden( bool b )			{ m_bVisible = !b; }
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
	void SetBlendModeString( const CString &s );	// convenience
	void SetTextureWrapping( bool b ) 			{ m_bTextureWrapping = b; } 
	void SetClearZBuffer( bool b ) 				{ m_bClearZBuffer = b; } 
	void SetUseZBuffer( bool b ) 				{ SetZTestMode(b?ZTEST_WRITE_ON_PASS:ZTEST_OFF); SetZWrite(b); } 
	virtual void SetZTestMode( ZTestMode mode ) { m_ZTestMode = mode; } 
	void SetZTestModeString( const CString &s );	// convenience
	virtual void SetZWrite( bool b ) 			{ m_bZWrite = b; } 
	virtual void SetCullMode( CullMode mode ) 	{ m_CullMode = mode; } 
	void SetCullModeString( const CString &s );	// convenience

	//
	// Commands
	//
	virtual void PushSelf( lua_State *L );
	void AddCommand( const CString &sCmdName, apActorCommands apac );
	bool HasCommand( const CString &sCmdName );
	virtual void PlayCommand( const CString &sCommandName );
	virtual void PlayCommand2( const CString &sCommandName, Actor *pParent );
	virtual void RunCommands( const LuaReference& cmds );
	virtual void RunCommands2( const LuaReference& cmds, Actor *pParent );
	void RunCommands( const apActorCommands& cmds ) { this->RunCommands( *cmds ); }	// convenience
	void RunCommands2( const apActorCommands& cmds, Actor *pParent ) { this->RunCommands2( *cmds, pParent ); }	// convenience
	void SubscribeToMessage( const CString &sMessageName ); // will automatically unsubscribe

	static float GetCommandsLengthSeconds( const LuaReference& cmds );
	static float GetCommandsLengthSeconds( const apActorCommands& cmds ) { return GetCommandsLengthSeconds( *cmds ); }	// convenience

	//
	// Messages
	//
	virtual void HandleMessage( const CString& sMessage );


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
	CString m_sEffectCommand; // effect_lua
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
	bool	m_bVisible;
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

private:
	//
	// commands
	//
	map<CString, apActorCommands> m_mapNameToCommands;
	vector<CString> m_vsSubscribedTo;
};

template<class T>
class LunaActor : public Luna<T>
{
public:
	LunaActor() { LUA->Register( Register ); }

	static int sleep( T* p, lua_State *L )			{ p->Sleep(FArg(1)); return 0; }
	static int linear( T* p, lua_State *L )			{ p->BeginTweening(FArg(1),Actor::TWEEN_LINEAR); return 0; }
	static int accelerate( T* p, lua_State *L )		{ p->BeginTweening(FArg(1),Actor::TWEEN_ACCELERATE); return 0; }
	static int decelerate( T* p, lua_State *L )		{ p->BeginTweening(FArg(1),Actor::TWEEN_DECELERATE); return 0; }
	static int bouncebegin( T* p, lua_State *L )	{ p->BeginTweening(FArg(1),Actor::TWEEN_BOUNCE_BEGIN); return 0; }
	static int bounceend( T* p, lua_State *L )		{ p->BeginTweening(FArg(1),Actor::TWEEN_BOUNCE_END); return 0; }
	static int spring( T* p, lua_State *L )			{ p->BeginTweening(FArg(1),Actor::TWEEN_SPRING); return 0; }
	static int stoptweening( T* p, lua_State *L )	{ p->StopTweening(); p->BeginTweening( 0.0001f, Actor::TWEEN_LINEAR ); return 0; }
	static int finishtweening( T* p, lua_State *L )	{ p->FinishTweening(); return 0; }
	static int hurrytweening( T* p, lua_State *L )	{ p->HurryTweening(FArg(1)); return 0; }
	static int x( T* p, lua_State *L )				{ p->SetX(FArg(1)); return 0; }
	static int y( T* p, lua_State *L )				{ p->SetY(FArg(1)); return 0; }
	static int z( T* p, lua_State *L )				{ p->SetZ(FArg(1)); return 0; }
	static int addx( T* p, lua_State *L )			{ p->AddX(FArg(1)); return 0; }
	static int addy( T* p, lua_State *L )			{ p->AddY(FArg(1)); return 0; }
	static int addz( T* p, lua_State *L )			{ p->AddZ(FArg(1)); return 0; }
	static int zoom( T* p, lua_State *L )			{ p->SetZoom(FArg(1)); return 0; }
	static int zoomx( T* p, lua_State *L )			{ p->SetZoomX(FArg(1)); return 0; }
	static int zoomy( T* p, lua_State *L )			{ p->SetZoomY(FArg(1)); return 0; }
	static int zoomz( T* p, lua_State *L )			{ p->SetZoomZ(FArg(1)); return 0; }
	static int zoomto( T* p, lua_State *L )			{ p->ZoomTo(FArg(1), FArg(2)); return 0; }
	static int zoomtowidth( T* p, lua_State *L )	{ p->ZoomToWidth(FArg(1)); return 0; }
	static int zoomtoheight( T* p, lua_State *L )	{ p->ZoomToHeight(FArg(1)); return 0; }
	static int stretchto( T* p, lua_State *L )		{ p->StretchTo( RectF(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int cropleft( T* p, lua_State *L )		{ p->SetCropLeft(FArg(1)); return 0; }
	static int croptop( T* p, lua_State *L )		{ p->SetCropTop(FArg(1)); return 0; }
	static int cropright( T* p, lua_State *L )		{ p->SetCropRight(FArg(1)); return 0; }
	static int cropbottom( T* p, lua_State *L )		{ p->SetCropBottom(FArg(1)); return 0; }
	static int fadeleft( T* p, lua_State *L )		{ p->SetFadeLeft(FArg(1)); return 0; }
	static int fadetop( T* p, lua_State *L )		{ p->SetFadeTop(FArg(1)); return 0; }
	static int faderight( T* p, lua_State *L )		{ p->SetFadeRight(FArg(1)); return 0; }
	static int fadebottom( T* p, lua_State *L )		{ p->SetFadeBottom(FArg(1)); return 0; }
	static int diffuse( T* p, lua_State *L )			{ p->SetDiffuse( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuseupperleft( T* p, lua_State *L )	{ p->SetDiffuseUpperLeft( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuseupperright( T* p, lua_State *L )	{ p->SetDiffuseUpperRight( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuselowerleft( T* p, lua_State *L )	{ p->SetDiffuseLowerLeft( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuselowerright( T* p, lua_State *L )	{ p->SetDiffuseLowerRight( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuseleftedge( T* p, lua_State *L )	{ p->SetDiffuseLeftEdge( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffuserightedge( T* p, lua_State *L )	{ p->SetDiffuseRightEdge( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffusetopedge( T* p, lua_State *L )		{ p->SetDiffuseTopEdge( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffusebottomedge( T* p, lua_State *L )	{ p->SetDiffuseBottomEdge( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int diffusealpha( T* p, lua_State *L )	{ p->SetDiffuseAlpha(FArg(1)); return 0; }
	static int diffusecolor( T* p, lua_State *L )	{ p->SetDiffuseColor( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int glow( T* p, lua_State *L )			{ p->SetGlow( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int rotationx( T* p, lua_State *L )		{ p->SetRotationX(FArg(1)); return 0; }
	static int rotationy( T* p, lua_State *L )		{ p->SetRotationY(FArg(1)); return 0; }
	static int rotationz( T* p, lua_State *L )		{ p->SetRotationZ(FArg(1)); return 0; }
	static int heading( T* p, lua_State *L )		{ p->AddRotationH(FArg(1)); return 0; }
	static int pitch( T* p, lua_State *L )			{ p->AddRotationP(FArg(1)); return 0; }
	static int roll( T* p, lua_State *L )			{ p->AddRotationR(FArg(1)); return 0; }
	static int shadowlength( T* p, lua_State *L )	{ p->SetShadowLength(FArg(1)); return 0; }
	static int horizalign( T* p, lua_State *L )		{ p->SetHorizAlignString(SArg(1)); return 0; }
	static int vertalign( T* p, lua_State *L )		{ p->SetVertAlignString(SArg(1)); return 0; }
	static int luaeffect( T* p, lua_State *L )		{ p->SetEffectLua(SArg(1)); return 0; }
	static int diffuseblink( T* p, lua_State *L )	{ p->SetEffectDiffuseBlink(); return 0; }
	static int diffuseshift( T* p, lua_State *L )	{ p->SetEffectDiffuseShift(); return 0; }
	static int diffuseramp( T* p, lua_State *L )	{ p->SetEffectDiffuseRamp(); return 0; }
	static int glowblink( T* p, lua_State *L )		{ p->SetEffectGlowBlink(); return 0; }
	static int glowshift( T* p, lua_State *L )		{ p->SetEffectGlowShift(); return 0; }
	static int rainbow( T* p, lua_State *L )		{ p->SetEffectRainbow(); return 0; }
	static int wag( T* p, lua_State *L )			{ p->SetEffectWag(); return 0; }
	static int bounce( T* p, lua_State *L )			{ p->SetEffectBounce(); return 0; }
	static int bob( T* p, lua_State *L )			{ p->SetEffectBob(); return 0; }
	static int pulse( T* p, lua_State *L )			{ p->SetEffectPulse(); return 0; }
	static int spin( T* p, lua_State *L )			{ p->SetEffectSpin(); return 0; }
	static int vibrate( T* p, lua_State *L )		{ p->SetEffectVibrate(); return 0; }
	static int stopeffect( T* p, lua_State *L )		{ p->SetEffectNone(); return 0; }
	static int effectcolor1( T* p, lua_State *L )		{ p->SetEffectColor1( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int effectcolor2( T* p, lua_State *L )		{ p->SetEffectColor2( RageColor(FArg(1),FArg(2),FArg(3),FArg(4)) ); return 0; }
	static int effectperiod( T* p, lua_State *L )		{ p->SetEffectPeriod(FArg(1)); return 0; }
	static int effectoffset( T* p, lua_State *L )		{ p->SetEffectOffset(FArg(1)); return 0; }
	static int effectdelay( T* p, lua_State *L )		{ p->SetEffectDelay(FArg(1)); return 0; }
	static int effectclock( T* p, lua_State *L )		{ p->SetEffectClockString(SArg(1)); return 0; }
	static int effectmagnitude( T* p, lua_State *L )	{ p->SetEffectMagnitude( RageVector3(FArg(1),FArg(2),FArg(3)) ); return 0; }
	static int scaletocover( T* p, lua_State *L )		{ p->ScaleToCover( RectF(FArg(1), FArg(2), FArg(3), FArg(4)) ); return 0; }
	static int scaletofit( T* p, lua_State *L )			{ p->ScaleToFitInside( RectF(FArg(1), FArg(2), FArg(3), FArg(4)) ); return 0; }
	static int animate( T* p, lua_State *L )			{ p->EnableAnimation(!!IArg(1)); return 0; }
	static int play( T* p, lua_State *L )				{ p->EnableAnimation(true); return 0; }
	static int pause( T* p, lua_State *L )				{ p->EnableAnimation(false); return 0; }
	static int setstate( T* p, lua_State *L )			{ p->SetState(IArg(1)); return 0; }
	static int texturewrapping( T* p, lua_State *L )	{ p->SetTextureWrapping(!!IArg(1)); return 0; }
	static int additiveblend( T* p, lua_State *L )		{ p->SetBlendMode(!!IArg(1) ? BLEND_ADD : BLEND_NORMAL); return 0; }
	static int blend( T* p, lua_State *L )				{ p->SetBlendModeString(SArg(1)); return 0; }
	static int zbuffer( T* p, lua_State *L )			{ p->SetUseZBuffer(!!IArg(1)); return 0; }
	static int ztest( T* p, lua_State *L )				{ p->SetZTestMode((!!IArg(1))?ZTEST_WRITE_ON_PASS:ZTEST_OFF); return 0; }
	static int ztestmode( T* p, lua_State *L )			{ p->SetZTestModeString(SArg(1)); return 0; }
	static int zwrite( T* p, lua_State *L )				{ p->SetZWrite(!!IArg(1)); return 0; }
	static int clearzbuffer( T* p, lua_State *L )		{ p->SetClearZBuffer(!!IArg(1)); return 0; }
	static int backfacecull( T* p, lua_State *L )		{ p->SetCullMode((!!IArg(1)) ? CULL_BACK : CULL_NONE); return 0; }
	static int cullmode( T* p, lua_State *L )			{ p->SetCullModeString(SArg(1)); return 0; }
	static int visible( T* p, lua_State *L )			{ p->SetVisible(!!IArg(1)); return 0; }
	static int hidden( T* p, lua_State *L )				{ p->SetHidden(!!IArg(1)); return 0; }
	static int hibernate( T* p, lua_State *L )			{ p->SetHibernate(FArg(1)); return 0; }
	static int draworder( T* p, lua_State *L )			{ p->SetDrawOrder(IArg(1)); return 0; }
	static int playcommand( T* p, lua_State *L )		{ p->PlayCommand(SArg(1)); return 0; }
	static int queuecommand( T* p, lua_State *L )		{ p->QueueCommand(SArg(1)); return 0; }
	static int queuemessage( T* p, lua_State *L )		{ p->QueueMessage(SArg(1)); return 0; }

	static int GetWidth( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetUnzoomedWidth() ); return 1; }
	static int GetHeight( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetUnzoomedHeight() ); return 1; }
	static int GetZoom( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetZoom() ); return 1; }
	static int GetZoomX( T* p, lua_State *L )		{ lua_pushnumber( L, p->GetZoomX() ); return 1; }
	static int GetBaseZoomX( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetBaseZoomX() ); return 1; }
	static int GetSecsIntoEffect( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetSecsIntoEffect() ); return 1; }
	static int GetEffectDelta( T* p, lua_State *L )	{ lua_pushnumber( L, p->GetEffectDelta() ); return 1; }


	static void Register(lua_State *L) {
  		ADD_METHOD( sleep )
		ADD_METHOD( linear )
		ADD_METHOD( accelerate )
		ADD_METHOD( decelerate )
		ADD_METHOD( bouncebegin )
		ADD_METHOD( bounceend )
		ADD_METHOD( spring )
		ADD_METHOD( stoptweening )
		ADD_METHOD( finishtweening )
		ADD_METHOD( hurrytweening )
		ADD_METHOD( x )
		ADD_METHOD( y )
		ADD_METHOD( z )
		ADD_METHOD( addx )
		ADD_METHOD( addy )
		ADD_METHOD( addz )
		ADD_METHOD( zoom )
		ADD_METHOD( zoomx )
		ADD_METHOD( zoomy )
		ADD_METHOD( zoomz )
		ADD_METHOD( zoomto )
		ADD_METHOD( zoomtowidth )
		ADD_METHOD( zoomtoheight )
		ADD_METHOD( stretchto )
		ADD_METHOD( cropleft )
		ADD_METHOD( croptop )
		ADD_METHOD( cropright )
		ADD_METHOD( cropbottom )
		ADD_METHOD( fadeleft )
		ADD_METHOD( fadetop )
		ADD_METHOD( faderight )
		ADD_METHOD( fadebottom )
		ADD_METHOD( diffuse )
		ADD_METHOD( diffuseupperleft )
		ADD_METHOD( diffuseupperright )
		ADD_METHOD( diffuselowerleft )
		ADD_METHOD( diffuselowerright )
		ADD_METHOD( diffuseleftedge )
		ADD_METHOD( diffuserightedge )
		ADD_METHOD( diffusetopedge )
		ADD_METHOD( diffusebottomedge )
		ADD_METHOD( diffusealpha )
		ADD_METHOD( diffusecolor )
		ADD_METHOD( glow )
		ADD_METHOD( rotationx )
		ADD_METHOD( rotationy )
		ADD_METHOD( rotationz )
		ADD_METHOD( heading )
		ADD_METHOD( pitch )
		ADD_METHOD( roll )
		ADD_METHOD( shadowlength )
		ADD_METHOD( horizalign )
		ADD_METHOD( vertalign )
		ADD_METHOD( luaeffect )
		ADD_METHOD( diffuseblink )
		ADD_METHOD( diffuseshift )
		ADD_METHOD( diffuseramp )
		ADD_METHOD( glowblink )
		ADD_METHOD( glowshift )
		ADD_METHOD( rainbow )
		ADD_METHOD( wag )
		ADD_METHOD( bounce )
		ADD_METHOD( bob )
		ADD_METHOD( pulse )
		ADD_METHOD( spin )
		ADD_METHOD( vibrate )
		ADD_METHOD( stopeffect )
		ADD_METHOD( effectcolor1 )
		ADD_METHOD( effectcolor2 )
		ADD_METHOD( effectperiod )
		ADD_METHOD( effectoffset )
		ADD_METHOD( effectdelay )
		ADD_METHOD( effectclock )
		ADD_METHOD( effectmagnitude )
		ADD_METHOD( scaletocover )
		ADD_METHOD( scaletofit )
		ADD_METHOD( animate )
		ADD_METHOD( play )
		ADD_METHOD( pause )
		ADD_METHOD( setstate )
		ADD_METHOD( texturewrapping )
		ADD_METHOD( additiveblend )
		ADD_METHOD( blend )
		ADD_METHOD( zbuffer )
		ADD_METHOD( ztest )
		ADD_METHOD( ztestmode )
		ADD_METHOD( zwrite )
		ADD_METHOD( clearzbuffer )
		ADD_METHOD( backfacecull )
		ADD_METHOD( cullmode )
		ADD_METHOD( visible )
		ADD_METHOD( hidden )
		ADD_METHOD( hibernate )
		ADD_METHOD( draworder )
		ADD_METHOD( playcommand )
		ADD_METHOD( queuecommand )
		ADD_METHOD( queuemessage )

		ADD_METHOD( GetWidth )
		ADD_METHOD( GetHeight )
		ADD_METHOD( GetZoom )
		ADD_METHOD( GetZoomX )
		ADD_METHOD( GetBaseZoomX )
		ADD_METHOD( GetSecsIntoEffect )
		ADD_METHOD( GetEffectDelta )

		Luna<T>::Register( L );
	}
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
