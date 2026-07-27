#ifndef ACTOR_H
#define ACTOR_H

#include "PlayerNumber.h"
#include "RageTypes.h"
#include "RageUtil_AutoPtr.h"
#include "LuaReference.h"
#include "EnumHelper.h"
class XNode;
struct lua_State;
class LuaClass;
#include "MessageManager.h"
#include "Tween.h"

#include <cstddef>
#include <map>
#include <vector>

typedef AutoPtrCopyOnWrite<LuaReference> apActorCommands;

/** @brief The background layer. */
#define DRAW_ORDER_BEFORE_EVERYTHING		-200
/** @brief The underlay layer. */
#define DRAW_ORDER_UNDERLAY			-100
/** @brief The decorations layer. */
#define DRAW_ORDER_DECORATIONS			   0
/** @brief The overlay layer.
 *
 * Normal screen elements go here. */
#define DRAW_ORDER_OVERLAY			+100
/** @brief The transitions layer. */
#define DRAW_ORDER_TRANSITIONS			+200
/** @brief The over everything layer. */
#define DRAW_ORDER_AFTER_EVERYTHING		+300

/** @brief The different horizontal alignments. */
enum HorizAlign
{
	HorizAlign_Left, /**< Align to the left. */
	HorizAlign_Center, /**< Align to the center. */
	HorizAlign_Right, /**< Align to the right. */
	NUM_HorizAlign, /**< The number of horizontal alignments. */
	HorizAlign_Invalid
};
LuaDeclareType( HorizAlign );

/** @brief The different vertical alignments. */
enum VertAlign
{
	VertAlign_Top, /**< Align to the top. */
	VertAlign_Middle, /**< Align to the middle. */
	VertAlign_Bottom, /**< Align to the bottom. */
	NUM_VertAlign, /**< The number of vertical alignments. */
	VertAlign_Invalid
};
LuaDeclareType( VertAlign );

/** @brief The left horizontal alignment constant. */
#define align_left 0.0f
/** @brief The center horizontal alignment constant. */
#define align_center 0.5f
/** @brief The right horizontal alignment constant. */
#define align_right 1.0f
/** @brief The top vertical alignment constant. */
#define align_top 0.0f
/** @brief The middle vertical alignment constant. */
#define align_middle 0.5f
/** @brief The bottom vertical alignment constant. */
#define align_bottom 1.0f

// This is the number of colors in Actor::diffuse.  Actor has multiple
// diffuse colors so that each edge can be a different color, and the actor
// is drawn with a gradient between them.
// I doubt I actually found all the places that touch diffuse and rely on the
// number of diffuse colors, so change this at your own risk. -Kyz
#define NUM_DIFFUSE_COLORS 4

// ssc futures:
/*
enum EffectAction
{
	EffectAction_None,			// no_effect
	// [Diffuse]
	EffectAction_DiffuseBlink,	// diffuse_blink
	EffectAction_DiffuseShift,	// diffuse_shift
	EffectAction_DiffuseRamp,	// diffuse_ramp
	EffectAction_Rainbow,		// rainbow
	// [Glow]
	EffectAction_GlowBlink,		// glow_blink
	EffectAction_GlowShift,		// glow_shift
	EffectAction_GlowRamp,		// glow_ramp
	// [Translate]
	EffectAction_Bob,
	EffectAction_Bounce,
	EffectAction_Vibrate,
	// [Rotate]
	EffectAction_Spin,
	EffectAction_Wag,
	// [Zoom]
	EffectAction_Pulse,
	NUM_EffectAction,
	EffectAction_Invalid
};
LuaDeclareType( EffectAction );
*/

/** @brief Base class for all objects that appear on the screen. */
class Actor : public MessageSubscriber
{
public:
	/** @brief Set up the Actor with its initial settings. */
	Actor();
	/**
	 * @brief Copy a new Actor to the old one.
	 * @param cpy the new Actor to use in place of this one. */
	Actor( const Actor &cpy );
	Actor &operator=( Actor other );
	virtual ~Actor();
	virtual Actor *Copy() const;
	virtual void InitState();
	virtual void LoadFromNode( const XNode* pNode );

	static void SetBGMTime( float fTime, float fBeat, float fTimeNoOffset, float fBeatNoOffset );
	static void SetPlayerBGMBeat( PlayerNumber pn, float fBeat, float fBeatNoOffset );
	static void SetBGMLight( int iLightNumber, float fCabinetLights );

	/**
	 * @brief The list of the different effects.
	 *
	 * todo: split out into diffuse effects and translation effects, or
	 * create an effect stack instead. -aj */
	enum Effect { no_effect,
			diffuse_blink, diffuse_shift, diffuse_ramp,
			glow_blink, glow_shift, glow_ramp, rainbow,
			wag, bounce, bob, pulse, spin, vibrate
	};

	/** @brief Various values an Actor's effect can be tied to. */
	enum EffectClock
	{
		CLOCK_TIMER,
		CLOCK_TIMER_GLOBAL,
		CLOCK_BGM_TIME,
		CLOCK_BGM_BEAT,
		CLOCK_BGM_TIME_NO_OFFSET,
		CLOCK_BGM_BEAT_NO_OFFSET,
		CLOCK_BGM_BEAT_PLAYER1,
		CLOCK_BGM_BEAT_PLAYER2,
		CLOCK_LIGHT_1 = 1000,
		CLOCK_LIGHT_LAST = 1100,
		NUM_CLOCKS
	};

	/*
	 * @brief What type of Effect this is.
	 *
	 * This is an internal enum for checking if an effect can be run;
	 * You can't have more than one of most EffectTypes in the Effect list. (You
	 * might be able to have mutliple EffectType_Translates; not sure yet.) -aj */
	/*
	enum EffectType {
		EffectType_Diffuse,
		EffectType_Glow,
		EffectType_Translate,
		EffectType_Rotate,
		EffectType_Zoom,
		NUM_EffectType,
		EffectType_Invalid
	};
	*/

	// todo: use this instead of the Effect enum -aj
	/*
	// This is similar to Attributes in BitmapText as far as implementation.
	struct Effect
	{
		Effect() : m_Action(EffectAction_None), m_Type(EffectType_Invalid), m_fSecsIntoEffect(0),
				m_fEffectDelta(0), m_fEffectRampUp(0.5f), m_fEffectHoldAtHalf(0),
				m_fEffectRampDown(0.5f), m_fEffectHoldAtZero(0), m_fEffectOffset(0),
				m_EffectClock(CLOCK_TIMER), m_vEffectMagnitude(RageVector3(0,0,10)),
				m_effectColor1(RageColor(1,1,1,1)), m_effectColor2(RageColor(1,1,1,1))
		{ }

		RString			m_sName; // friendly name
		EffectAction	m_Action; // replaces the old Effect enum
		EffectType		m_Type; // determined by EffectAction
		float			m_fSecsIntoEffect;
		float			m_fEffectDelta;
		RageColor		m_EffectColor1;
		RageColor		m_EffectColor2;
		RageVector3		m_vEffectMagnitude;
		EffectClock		m_EffectClock;
		// units depend on m_EffectClock
		float			m_fEffectRampUp;
		float			m_fEffectHoldAtHalf;
		float			m_fEffectRampDown;
		float			m_fEffectHoldAtZero;
		float			m_fEffectOffset;
	};
	*/

	/**
	 * @brief The present state for the Tween.
	 */
	struct TweenState
	{
		void Init();
		static void MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween );
		bool operator==( const TweenState &other ) const;
		bool operator!=( const TweenState &other ) const { return !operator==(other); }

		// start and end position for tweening
		RageVector3	pos;
		RageVector3	rotation;
		RageVector4	quat;
		RageVector3	scale;
		float		fSkewX, fSkewY;
		/**
		 * @brief The amount of cropping involved.
		 *
		 * If 0, there is no cropping. If 1, it's fully cropped. */
		RectF		crop;
		/**
		 * @brief The amount of fading involved.
		 *
		 * If 0, there is no fade. If 1, it's fully faded. */
		RectF		fade;
		/**
		 * @brief Four values making up the diffuse in this TweenState.
		 *
		 * 0 = UpperLeft, 1 = UpperRight, 2 = LowerLeft, 3 = LowerRight */
		RageColor	diffuse[NUM_DIFFUSE_COLORS];
		/** @brief The glow color for this TweenState. */
		RageColor	glow;
		/** @brief A magical value that nobody really knows the use for. ;) */
		float		aux;
	};

	// PartiallyOpaque broken out of Draw for reuse and clarity.
	bool PartiallyOpaque();
	/**
	 * @brief Calls multiple functions for drawing the Actors.
	 *
	 * It calls the following in order:
	 * -# EarlyAbortDraw
	 * -# BeginDraw
	 * -# DrawPrimitives
	 * -# EndDraw
	 */
	void Draw();
	/**
	 * @brief Allow the Actor to be aborted early.
	 *
	 * Subclasses may wish to overwrite this to allow for
	 * aborted actors.
	 * @return false, as by default Actors shouldn't be aborted on drawing. */
	virtual bool EarlyAbortDraw() const { return false; }
	/** @brief Calculate values that may be needed  for drawing. */
	virtual void PreDraw();
	/** @brief Reset internal diffuse and glow. */
	virtual void PostDraw();
	/** @brief Start the drawing and push the transform on the world matrix stack. */
	virtual void BeginDraw();
	/**
	 * @brief Set the global rendering states of this Actor.
	 *
	 * This should be called at the beginning of an Actor's DrawPrimitives() call. */
	virtual void SetGlobalRenderStates();
	/**
	 * @brief Set the texture rendering states of this Actor.
	 *
	 * This should be called after setting a texture for the Actor. */
	virtual void SetTextureRenderStates();
	/**
	 * @brief Draw the primitives of the Actor.
	 *
	 * Derivative classes should override this function. */
	virtual void DrawPrimitives() {};
	/** @brief Pop the transform from the world matrix stack. */
	virtual void EndDraw();

	// TODO: make Update non virtual and change all classes to override UpdateInternal
	// instead.
	bool IsFirstUpdate() const;
	virtual void Update( float fDeltaTime );		// this can short circuit UpdateInternal
	virtual void UpdateInternal( float fDeltaTime );	// override this
	void UpdateTweening( float fDeltaTime );
	void CalcPercentThroughTween();
	// These next functions should all be overridden by a derived class that has its own tweening states to handle.
	virtual void SetCurrentTweenStart() {}
	virtual void EraseHeadTween() {}
	virtual void UpdatePercentThroughTween( float PercentThroughTween ) {}
	bool get_tween_uses_effect_delta() { return m_tween_uses_effect_delta; }
	void set_tween_uses_effect_delta(bool t) { m_tween_uses_effect_delta= t; }

	/**
	 * @brief Retrieve the Actor's name.
	 * @return the Actor's name. */
	const RString &GetName() const			{ return m_sName; }
	/**
	 * @brief Set the Actor's name to a new one.
	 * @param sName the new name for the Actor. */
	virtual void SetName( const RString &sName )	{ m_sName = sName; }
	/**
	 * @brief Give this Actor a new parent.
	 * @param pParent the new parent Actor. */
	void SetParent( Actor *pParent );
	/**
	 * @brief Retrieve the Actor's parent.
	 * @return the Actor's parent. */
	Actor *GetParent() { return m_pParent; }
	/**
	 * @brief Retrieve the Actor's lineage.
	 * @return the Actor's lineage. */
	RString GetLineage() const;

	void SetFakeParent(Actor* mailman) { m_FakeParent= mailman; }
	Actor* GetFakeParent() { return m_FakeParent; }

	void AddWrapperState();
	void RemoveWrapperState(std::size_t i);
	Actor* GetWrapperState(std::size_t i);
	std::size_t GetNumWrapperStates() const { return m_WrapperStates.size(); }

	/**
	 * @brief Retrieve the Actor's x position.
	 * @return the Actor's x position. */
	float GetX() const				{ return m_current.pos.x; };
	/**
	 * @brief Retrieve the Actor's y position.
	 * @return the Actor's y position. */
	float GetY() const				{ return m_current.pos.y; };
	/**
	 * @brief Retrieve the Actor's z position.
	 * @return the Actor's z position. */
	float GetZ() const				{ return m_current.pos.z; };
	float GetDestX() const				{ return DestTweenState().pos.x; };
	float GetDestY() const				{ return DestTweenState().pos.y; };
	float GetDestZ() const				{ return DestTweenState().pos.z; };
	void  SetX( float x )				{ DestTweenState().pos.x = x; };
	void  SetY( float y )				{ DestTweenState().pos.y = y; };
	void  SetZ( float z )				{ DestTweenState().pos.z = z; };
	void  SetXY( float x, float y )			{ DestTweenState().pos.x = x; DestTweenState().pos.y = y; };
	/**
	 * @brief Add to the x position of this Actor.
	 * @param x the amount to add to the Actor's x position. */
	void  AddX( float x )				{ SetX( GetDestX()+x ); }
	/**
	 * @brief Add to the y position of this Actor.
	 * @param y the amount to add to the Actor's y position. */
	void  AddY( float y )				{ SetY( GetDestY()+y ); }
	/**
	 * @brief Add to the z position of this Actor.
	 * @param z the amount to add to the Actor's z position. */
	void  AddZ( float z )				{ SetZ( GetDestZ()+z ); }

	// height and width vary depending on zoom
	float GetUnzoomedWidth() const			{ return m_size.x; }
	float GetUnzoomedHeight() const			{ return m_size.y; }
	float GetZoomedWidth() const 			{ return m_size.x * m_baseScale.x * DestTweenState().scale.x; }
	float GetZoomedHeight() const			{ return m_size.y * m_baseScale.y * DestTweenState().scale.y; }
	void  SetWidth( float width )			{ m_size.x = width; }
	void  SetHeight( float height )			{ m_size.y = height; }

	// Base values
	float GetBaseZoomX() const			{ return m_baseScale.x;	}
	void  SetBaseZoomX( float zoom )		{ m_baseScale.x = zoom;	}
	float GetBaseZoomY() const			{ return m_baseScale.y;	}
	void  SetBaseZoomY( float zoom )		{ m_baseScale.y = zoom; }
	float GetBaseZoomZ() const			{ return m_baseScale.z;	}
	void  SetBaseZoomZ( float zoom )		{ m_baseScale.z = zoom; }
	void  SetBaseZoom( float zoom )			{ m_baseScale = RageVector3(zoom,zoom,zoom); }
	void  SetBaseRotationX( float rot )		{ m_baseRotation.x = rot; }
	void  SetBaseRotationY( float rot )		{ m_baseRotation.y = rot; }
	void  SetBaseRotationZ( float rot )		{ m_baseRotation.z = rot; }
	void  SetBaseRotation( const RageVector3 &rot )	{ m_baseRotation = rot; }
	virtual void  SetBaseAlpha( float fAlpha )	{ m_fBaseAlpha = fAlpha; }
	void  SetInternalDiffuse( const RageColor &c )	{ m_internalDiffuse = c; }
	void  SetInternalGlow( const RageColor &c )	{ m_internalGlow = c; }

	/**
	 * @brief Retrieve the general zoom factor, using the x coordinate of the Actor.
	 *
	 * Note that this is not accurate in some cases.
	 * @return the zoom factor for the x coordinate of the Actor. */
	float GetZoom() const				{ return DestTweenState().scale.x; }
	/**
	 * @brief Retrieve the zoom factor for the x coordinate of the Actor.
	 * @return the zoom factor for the x coordinate of the Actor. */
	float GetZoomX() const				{ return DestTweenState().scale.x; }
	/**
	 * @brief Retrieve the zoom factor for the y coordinate of the Actor.
	 * @return the zoom factor for the y coordinate of the Actor. */
	float GetZoomY() const				{ return DestTweenState().scale.y; }
	/**
	 * @brief Retrieve the zoom factor for the z coordinate of the Actor.
	 * @return the zoom factor for the z coordinate of the Actor. */
	float GetZoomZ() const				{ return DestTweenState().scale.z; }
	/**
	 * @brief Set the zoom factor for all dimensions of the Actor.
	 * @param zoom the zoom factor for all dimensions. */
	void  SetZoom( float zoom )
	{
		DestTweenState().scale.x = zoom;
		DestTweenState().scale.y = zoom;
		DestTweenState().scale.z = zoom;
	}
	/**
	 * @brief Set the zoom factor for the x dimension of the Actor.
	 * @param zoom the zoom factor for the x dimension. */
	void  SetZoomX( float zoom )			{ DestTweenState().scale.x = zoom; }
	/**
	 * @brief Set the zoom factor for the y dimension of the Actor.
	 * @param zoom the zoom factor for the y dimension. */
	void  SetZoomY( float zoom )			{ DestTweenState().scale.y = zoom; }
	/**
	 * @brief Set the zoom factor for the z dimension of the Actor.
	 * @param zoom the zoom factor for the z dimension. */
	void  SetZoomZ( float zoom )			{ DestTweenState().scale.z = zoom; }
	void  ZoomTo( float fX, float fY )		{ ZoomToWidth(fX); ZoomToHeight(fY); }
	void  ZoomToWidth( float zoom )			{ SetZoomX( zoom / GetUnzoomedWidth() ); }
	void  ZoomToHeight( float zoom )		{ SetZoomY( zoom / GetUnzoomedHeight() ); }

	float GetRotationX() const			{ return DestTweenState().rotation.x; }
	float GetRotationY() const			{ return DestTweenState().rotation.y; }
	float GetRotationZ() const			{ return DestTweenState().rotation.z; }
	void  SetRotationX( float rot )		{ DestTweenState().rotation.x = rot; }
	void  SetRotationY( float rot )		{ DestTweenState().rotation.y = rot; }
	void  SetRotationZ( float rot )		{ DestTweenState().rotation.z = rot; }
	// added in StepNXA, now available in sm-ssc:
	void  AddRotationX( float rot )		{ DestTweenState().rotation.x += rot; };
	void  AddRotationY( float rot )		{ DestTweenState().rotation.y += rot; };
	void  AddRotationZ( float rot )		{ DestTweenState().rotation.z += rot; };
	// and these were normally in SM:
	void  AddRotationH( float rot );
	void  AddRotationP( float rot );
	void  AddRotationR( float rot );

	void SetSkewX( float fAmount )			{ DestTweenState().fSkewX = fAmount; }
	float GetSkewX( float /* fAmount */ ) const		{ return DestTweenState().fSkewX; }
	void SetSkewY( float fAmount )			{ DestTweenState().fSkewY = fAmount; }
	float GetSkewY( float /* fAmount */ ) const		{ return DestTweenState().fSkewY; }

	float GetCropLeft() const			{ return DestTweenState().crop.left; }
	float GetCropTop() const			{ return DestTweenState().crop.top; }
	float GetCropRight() const			{ return DestTweenState().crop.right; }
	float GetCropBottom() const			{ return DestTweenState().crop.bottom; }
	void  SetCropLeft( float percent )		{ DestTweenState().crop.left = percent; }
	void  SetCropTop( float percent	) 		{ DestTweenState().crop.top = percent; }
	void  SetCropRight( float percent )		{ DestTweenState().crop.right = percent; }
	void  SetCropBottom( float percent )		{ DestTweenState().crop.bottom = percent; }

	void  SetFadeLeft( float percent )		{ DestTweenState().fade.left = percent; }
	void  SetFadeTop( float percent )		{ DestTweenState().fade.top = percent;	}
	void  SetFadeRight( float percent )		{ DestTweenState().fade.right = percent; }
	void  SetFadeBottom( float percent )		{ DestTweenState().fade.bottom = percent; }

	void SetGlobalDiffuseColor( RageColor c );

	virtual void SetDiffuse( RageColor c )		{ for(int i=0; i<NUM_DIFFUSE_COLORS; i++) DestTweenState().diffuse[i] = c; };
	virtual void SetDiffuseAlpha( float f )		{ for(int i = 0; i < NUM_DIFFUSE_COLORS; ++i) { RageColor c = GetDiffuses( i ); c.a = f; SetDiffuses( i, c ); } }
	float GetCurrentDiffuseAlpha() const		{ return m_current.diffuse[0].a; }
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
	RageColor GetDiffuse() const			{ return DestTweenState().diffuse[0]; };
	RageColor GetDiffuses( int i ) const		{ return DestTweenState().diffuse[i]; };
	float GetDiffuseAlpha() const			{ return DestTweenState().diffuse[0].a; };
	void SetGlow( RageColor c )			{ DestTweenState().glow = c; };
	RageColor GetGlow() const			{ return DestTweenState().glow; };

	void SetAux( float f )				{ DestTweenState().aux = f; }
	float GetAux() const				{ return m_current.aux; }

	virtual void BeginTweening( float time, ITween *pInterp );
	virtual void BeginTweening( float time, TweenType tt = TWEEN_LINEAR );
	virtual void StopTweening();
	void Sleep( float time );
	void QueueCommand( const RString& sCommandName );
	void QueueMessage( const RString& sMessageName );
	virtual void FinishTweening();
	virtual void HurryTweening( float factor );
	// Let ActorFrame and BGAnimation override
	virtual float GetTweenTimeLeft() const;	// Amount of time until all tweens have stopped
	TweenState& DestTweenState() // where Actor will end when its tween finish
	{
		if( m_Tweens.empty() )	// not tweening
			return m_current;
		else
			return m_Tweens.back()->state;
	}
	const TweenState& DestTweenState() const { return const_cast<Actor*>(this)->DestTweenState(); }

	/** @brief How do we handle stretching the Actor? */
	enum StretchType
	{
		fit_inside, /**< Have the Actor fit inside its parent, using the smaller zoom. */
		cover /**< Have the Actor cover its parent, using the larger zoom. */
	};

	void ScaleToCover( const RectF &rect )		{ ScaleTo( rect, cover ); }
	void ScaleToFitInside( const RectF &rect )	{ ScaleTo( rect, fit_inside); };
	void ScaleTo( const RectF &rect, StretchType st );

	void StretchTo( const RectF &rect );

	// Alignment settings.  These need to be virtual for BitmapText
	virtual void SetHorizAlign( float f ) { m_fHorizAlign = f; }
	virtual void SetVertAlign( float f ) { m_fVertAlign = f; }
	void SetHorizAlign( HorizAlign ha ) { SetHorizAlign( (ha == HorizAlign_Left)? 0.0f: (ha == HorizAlign_Center)? 0.5f: +1.0f ); }
	void SetVertAlign( VertAlign va ) { SetVertAlign( (va == VertAlign_Top)? 0.0f: (va == VertAlign_Middle)? 0.5f: +1.0f ); }
	virtual float GetHorizAlign() { return m_fHorizAlign; }
	virtual float GetVertAlign() { return m_fVertAlign; }

	// effects
#if defined(SSC_FUTURES)
	void StopEffects();
	Effect GetEffect( int i ) const			{ return m_Effects[i]; }
#else
	void StopEffect()				{ m_Effect = no_effect; }
	Effect GetEffect() const			{ return m_Effect; }
#endif
	float GetSecsIntoEffect() const			{ return m_fSecsIntoEffect; }
	float GetEffectDelta() const			{ return m_fEffectDelta; }

	// todo: account for SSC_FUTURES by adding an effect as an arg to each one -aj
	void SetEffectColor1( RageColor c )		{ m_effectColor1 = c; }
	void SetEffectColor2( RageColor c )		{ m_effectColor2 = c; }
	void RecalcEffectPeriod();
	void SetEffectPeriod( float fTime );
	float GetEffectPeriod() const { return m_effect_period; }
	bool SetEffectTiming(float ramp_toh, float at_half, float ramp_tof,
		float at_zero, float at_full, RString& err);
	bool SetEffectHoldAtFull(float haf, RString& err);
	void SetEffectOffset( float fTime )		{ m_fEffectOffset = fTime; }
	void SetEffectClock( EffectClock c )		{ m_EffectClock = c; }
	void SetEffectClockString( const RString &s );	// convenience

	void SetEffectMagnitude( RageVector3 vec )	{ m_vEffectMagnitude = vec; }
	RageVector3 GetEffectMagnitude() const		{ return m_vEffectMagnitude; }

	void ResetEffectTimeIfDifferent(Effect new_effect);
	void SetEffectDiffuseBlink( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectDiffuseShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectDiffuseRamp( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectGlowBlink( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectGlowShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectGlowRamp( float fEffectPeriodSeconds, RageColor c1, RageColor c2 );
	void SetEffectRainbow( float fEffectPeriodSeconds );
	void SetEffectWag( float fPeriod, RageVector3 vect );
	void SetEffectBounce( float fPeriod, RageVector3 vect );
	void SetEffectBob( float fPeriod, RageVector3 vect );
	void SetEffectPulse( float fPeriod, float fMinZoom, float fMaxZoom );
	void SetEffectSpin( RageVector3 vect );
	void SetEffectVibrate( RageVector3 vect );


	// other properties
	/**
	 * @brief Determine if the Actor is visible at this time.
	 * @return true if it's visible, false otherwise. */
	bool GetVisible() const				{ return m_bVisible; }
	void SetVisible( bool b )			{ m_bVisible = b; }
	void SetShadowLength( float fLength )		{ m_fShadowLengthX = fLength; m_fShadowLengthY = fLength; }
	void SetShadowLengthX( float fLengthX )		{ m_fShadowLengthX = fLengthX; }
	void SetShadowLengthY( float fLengthY )		{ m_fShadowLengthY = fLengthY; }
	void SetShadowColor( RageColor c )		{ m_ShadowColor = c; }
	// TODO: Implement hibernate as a tween type?
	void SetHibernate( float fSecs )		{ m_fHibernateSecondsLeft = fSecs; }
	void SetDrawOrder( int iOrder )			{ m_iDrawOrder = iOrder; }
	int GetDrawOrder() const			{ return m_iDrawOrder; }

	virtual void EnableAnimation( bool b ) 		{ m_bIsAnimating = b; }	// Sprite needs to overload this
	void StartAnimating()				{ this->EnableAnimation(true); }
	void StopAnimating()				{ this->EnableAnimation(false); }

	// render states
	void SetBlendMode( BlendMode mode )		{ m_BlendMode = mode; }
	void SetTextureTranslate( float x, float y )	{ m_texTranslate.x = x; m_texTranslate.y = y; }
	void SetTextureWrapping( bool b ) 			{ m_bTextureWrapping = b; }
	void SetTextureFiltering( bool b ) 		{ m_bTextureFiltering = b; }
	void SetClearZBuffer( bool b ) 			{ m_bClearZBuffer = b; }
	void SetUseZBuffer( bool b ) 				{ SetZTestMode(b?ZTEST_WRITE_ON_PASS:ZTEST_OFF); SetZWrite(b); }
	virtual void SetZTestMode( ZTestMode mode )	{ m_ZTestMode = mode; }
	virtual void SetZWrite( bool b ) 			{ m_bZWrite = b; }
	void SetZBias( float f )					{ m_fZBias = f; }
	virtual void SetCullMode( CullMode mode ) { m_CullMode = mode; }

	// Lua
	virtual void PushSelf( lua_State *L );
	virtual void PushContext( lua_State *L );

	// Named commands
	void AddCommand( const RString &sCmdName, apActorCommands apac, bool warn= true );
	bool HasCommand( const RString &sCmdName ) const;
	const apActorCommands *GetCommand( const RString &sCommandName ) const;
	void PlayCommand( const RString &sCommandName ) { HandleMessage( Message(sCommandName) ); } // convenience
	void PlayCommandNoRecurse( const Message &msg );

	// Commands by reference
	virtual void RunCommands( const LuaReference& cmds, const LuaReference *pParamTable = nullptr );
	void RunCommands( const apActorCommands& cmds, const LuaReference *pParamTable = nullptr ) { this->RunCommands( *cmds, pParamTable ); }	// convenience
	virtual void RunCommandsRecursively( const LuaReference& cmds, const LuaReference *pParamTable = nullptr ) { RunCommands(cmds, pParamTable); }
	// If we're a leaf, then execute this command.
	virtual void RunCommandsOnLeaves( const LuaReference& cmds, const LuaReference *pParamTable = nullptr ) { RunCommands(cmds, pParamTable); }

	// Messages
	virtual void HandleMessage( const Message &msg );

	// Animation
	virtual int GetNumStates() const { return 1; }
	virtual void SetState( int /* iNewState */ ) {}
	virtual float GetAnimationLengthSeconds() const { return 0; }
	virtual void SetSecondsIntoAnimation( float ) {}
	virtual void SetUpdateRate( float ) {}
	virtual float GetUpdateRate() { return 1.0f; }

	HiddenPtr<LuaClass> m_pLuaInstance;

protected:
	/** @brief the name of the Actor. */
	RString m_sName;
	/** @brief the current parent of this Actor if it exists. */
	Actor *m_pParent;
	// m_FakeParent exists to provide a way to render the actor inside another's
	// state without making that actor the parent.  It's like having multiple
	// parents. -Kyz
	Actor* m_FakeParent;
	// WrapperStates provides a way to wrap the actor inside ActorFrames,
	// applicable to any actor, not just ones the theme creates.
	std::vector<Actor*> m_WrapperStates;

	/** @brief Some general information about the Tween. */
	struct TweenInfo
	{
		// counters for tweening
		TweenInfo();
		~TweenInfo();
		TweenInfo( const TweenInfo &cpy );
		TweenInfo &operator=( const TweenInfo &rhs );

		ITween		*m_pTween;
		/** @brief How far into the tween are we? */
		float		m_fTimeLeftInTween;
		/** @brief The number of seconds between Start and End positions/zooms. */
		float		m_fTweenTime;
		/** @brief The command to execute when this TweenState goes into effect. */
		RString		m_sCommandName;
	};

	RageVector3	m_baseRotation;
	RageVector3	m_baseScale;
	float m_fBaseAlpha;
	RageColor m_internalDiffuse;
	RageColor m_internalGlow;

	RageVector2	m_size;
	TweenState	m_current;
	TweenState	m_start;
	TweenState	m_current_with_effects;
	struct TweenStateAndInfo
	{
		TweenState state;
		TweenInfo info;
	};
	std::vector<TweenStateAndInfo *>	m_Tweens;

	/** @brief Temporary variables that are filled just before drawing */
	TweenState *m_pTempState;

	bool	m_bFirstUpdate;

	// Stuff for alignment
	/** @brief The particular horizontal alignment.
	 *
	 * Use the defined constant values for best effect. */
	float	m_fHorizAlign;
	/** @brief The particular vertical alignment.
	 *
	 * Use the defined constant values for best effect. */
	float	m_fVertAlign;

	// Stuff for effects
#if defined(SSC_FUTURES) // be able to stack effects
	std::vector<Effect> m_Effects;
#else // compatibility
	Effect m_Effect;
#endif
	float m_fSecsIntoEffect;
	float m_fEffectDelta;

	// units depend on m_EffectClock
	float m_effect_ramp_to_half;
	float m_effect_hold_at_half;
	float m_effect_ramp_to_full;
	float m_effect_hold_at_full;
	float m_effect_hold_at_zero;
	float m_fEffectOffset;
	// Anything changing ramp_up, hold_at_half, ramp_down, or hold_at_zero must
	// also update the period so the period is only calculated when changed.
	// -Kyz
	float m_effect_period;
	EffectClock m_EffectClock;
	bool m_tween_uses_effect_delta;

	/* This can be used in lieu of the fDeltaTime parameter to Update() to
	 * follow the effect clock.  Actor::Update must be called first. */
	float GetEffectDeltaTime() const		{ return m_fEffectDelta; }

	// todo: account for SSC_FUTURES by having these be vectors too -aj
	RageColor	m_effectColor1;
	RageColor	m_effectColor2;
	RageVector3	m_vEffectMagnitude;

	// other properties
	bool		m_bVisible;
	bool		m_bIsAnimating;
	float		m_fHibernateSecondsLeft;
	float		m_fShadowLengthX;
	float		m_fShadowLengthY;
	RageColor	m_ShadowColor;
	/** @brief The draw order priority.
	 *
	 * The lower this number is, the sooner it is drawn. */
	int		m_iDrawOrder;

	// render states
	BlendMode	m_BlendMode;
	ZTestMode	m_ZTestMode;
	CullMode	m_CullMode;
	RageVector2	m_texTranslate;
	bool		m_bTextureWrapping;
	bool		m_bTextureFiltering;
	bool		m_bClearZBuffer;
	bool		m_bZWrite;
	/**
	 * @brief The amount of bias.
	 *
	 * If 0, there is no bias. If 1, there is a full bias. */
	float		m_fZBias;

	// global state
	static float g_fCurrentBGMTime, g_fCurrentBGMBeat;
	static float g_fCurrentBGMTimeNoOffset, g_fCurrentBGMBeatNoOffset;
	static std::vector<float> g_vfCurrentBGMBeatPlayer;
	static std::vector<float> g_vfCurrentBGMBeatPlayerNoOffset;

private:
	// commands
	std::map<RString, apActorCommands> m_mapNameToCommands;
};

#endif

/**
 * @file
 * @author Chris Danford (c) 2001-2004
 * @section LICENSE
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
