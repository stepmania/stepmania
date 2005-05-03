#include "global.h"
#include "Actor.h"
#include "RageDisplay.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"
#include "Foreach.h"
#include "XmlFile.h"
#include "LuaBinding.h"
#include "Command.h"
#include "ActorCommands.h"
#include "ThemeManager.h"
#include "LuaReference.h"
#include "MessageManager.h"
#include "LightsManager.h" // for NUM_CABINET_LIGHTS


// lua start
LUA_REGISTER_CLASS( Actor )
// lua end


float Actor::g_fCurrentBGMTime = 0, Actor::g_fCurrentBGMBeat;

static float g_fCabinetLights[NUM_CABINET_LIGHTS];

void Actor::SetBGMTime( float fTime, float fBeat )
{
	g_fCurrentBGMTime = fTime;
	g_fCurrentBGMBeat = fBeat;
}

void Actor::SetBGMLight( int iLightNumber, float fCabinetLights )
{
	ASSERT( iLightNumber < NUM_CABINET_LIGHTS );
	g_fCabinetLights[iLightNumber] = fCabinetLights;
}

/* This is Reset instead of Init since many derived classes have Init() functions
 * that shouldn't change the position of the actor. */
void Actor::Reset()
{
	m_Tweens.clear();

	m_pTempState = NULL;

	m_baseRotation = RageVector3( 0, 0, 0 );
	m_baseScale = RageVector3( 1, 1, 1 );
	m_fBaseAlpha = 1;

	m_start.Init();
	m_current.Init();

	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_Effect =  no_effect;
	m_fSecsIntoEffect = 0;
	m_fEffectDelta = 0;
	m_fEffectPeriodSeconds = 1;
	m_fEffectDelay = 0;
	m_fEffectOffset = 0;
	m_EffectClock = CLOCK_TIMER;
	m_vEffectMagnitude = RageVector3(0,0,10);
	m_effectColor1 = RageColor(1,1,1,1);
	m_effectColor2 = RageColor(1,1,1,1);

	m_bVisible = true;
	m_fShadowLength = 0;
	m_bIsAnimating = true;
	m_fHibernateSecondsLeft = 0;
	m_iDrawOrder = 0;

	m_bTextureWrapping = false;
	m_BlendMode = BLEND_NORMAL;
	m_bClearZBuffer = false;
	m_ZTestMode = ZTEST_OFF;
	m_bZWrite = false;
	m_CullMode = CULL_NONE;

	UnsubcribeAndClearCommands();
}

Actor::Actor()
{
	m_size = RageVector2( 1, 1 );
	Reset();
	m_bFirstUpdate = true;
}

static bool GetMessageNameFromCommandName( const CString &sCommandName, CString &sMessageNameOut )
{
	if( sCommandName.Right(7) == "Message" )
	{
		sMessageNameOut = sCommandName.Left(sCommandName.size()-7);
		return true;
	}
	else
	{
		return false;
	}
}

void Actor::UnsubcribeAndClearCommands()
{
	FOREACH_CONST( CString, m_vsSubscribedTo, s )
		MESSAGEMAN->Unsubscribe( this, *s );
	m_vsSubscribedTo.clear();
}

Actor::~Actor()
{
	UnsubcribeAndClearCommands();
}

void Actor::LoadFromNode( const CString& sDir, const XNode* pNode )
{
	// Load Name, if any.
	pNode->GetAttrValue( "Name", m_sName );


	float f;
	if( pNode->GetAttrValue( "BaseRotationXDegrees", f ) )	SetBaseRotationX( f );
	if( pNode->GetAttrValue( "BaseRotationYDegrees", f ) )	SetBaseRotationY( f );
	if( pNode->GetAttrValue( "BaseRotationZDegrees", f ) )	SetBaseRotationZ( f );
	if( pNode->GetAttrValue( "BaseZoomX", f ) )			SetBaseZoomX( f );
	if( pNode->GetAttrValue( "BaseZoomY", f ) )			SetBaseZoomY( f );
	if( pNode->GetAttrValue( "BaseZoomZ", f ) )			SetBaseZoomZ( f );


	//
	// Load commands
	//
	FOREACH_CONST_Attr( pNode, a )
	{
		CString sKeyName = a->m_sName; /* "OnCommand" */

		if( sKeyName.Right(7).CompareNoCase("Command") != 0 )
			continue; /* not a command */

		CString sValue = a->m_sValue;
		THEME->EvaluateString( sValue );
		apActorCommands apac( new ActorCommands( sValue ) );

		CString sCmdName;
		/* Special case: "Command=foo" -> "OnCommand=foo" */
		if( sKeyName.size() == 7 )
			sCmdName="On";
		else
			sCmdName = sKeyName.Left( sKeyName.size()-7 );
		AddCommand( sCmdName, apac );
	}

	/* There's an InitCommand.  Run it now.  This can be used to eg. change Z to
	 * modify draw order between BGAs in a Foreground. */
	PlayCommand( "Init" );
}

void Actor::Draw()
{
	if( !m_bVisible )
		return;	// early abort
	if( m_fHibernateSecondsLeft > 0 )
		return;	// early abort
	if( this->EarlyAbortDraw() )
		return;

	// call the most-derived versions
	this->BeginDraw();	
	this->DrawPrimitives();	// call the most-derived version of DrawPrimitives();
	this->EndDraw();	
}

void Actor::BeginDraw()		// set the world matrix and calculate actor properties
{
	DISPLAY->PushMatrix();	// we're actually going to do some drawing in this function	

	// Somthing below may set m_pTempState to m_tempState
	m_pTempState = &m_current;


	//
	// set temporary drawing properties based on Effects 
	//
	if( m_Effect == no_effect )
	{
	}
	else if( m_Effect == effect_lua )
	{
		/* Allow a Lua function to set the frame's draw state.  This may be expensive
		 * and has not been well-benchmarked yet; use wisely.  This allows arbitrary
		 * effects, instead of the mess of parameters below.  (In fact, all this does
		 * is run a command, but to avoid calling RunCommand() all the time, and due
		 * to the fact that this is the only place where the TempState is meaningful,
		 * we treat this as an effect.) */
		m_pTempState = &m_tempState;
		m_tempState = m_current;

		PlayCommand( m_sEffectCommand );
	}
	else
	{
		m_pTempState = &m_tempState;
		m_tempState = m_current;

		/* EffectPeriodSeconds is the total time of the effect (including delay).
		 * m_fEffectDelay is the amount of time to stick on 0%.  Offset shifts the
		 * entire thing forwards.  For example, if m_fEffectPeriodSeconds is 1,
		 * the effect can happen from .40 to .55 by setting offset to .40 and
		 * delay to .85. */
		const float fTotalPeriod = m_fEffectPeriodSeconds + m_fEffectDelay;
		CHECKPOINT_M( ssprintf("%f = %f + %f", fTotalPeriod, m_fEffectPeriodSeconds, m_fEffectDelay) );
		const float fSecsIntoPeriod = fmodfp( m_fSecsIntoEffect+m_fEffectOffset, fTotalPeriod );
		CHECKPOINT_M( ssprintf("%f = fmodfp(%f + %f, %f)", fSecsIntoPeriod, m_fSecsIntoEffect, m_fEffectOffset, fTotalPeriod) );

		float fPercentThroughEffect = SCALE( fSecsIntoPeriod, 0, m_fEffectPeriodSeconds, 0, 1 );
		CHECKPOINT_M( ssprintf("%f = SCALE(%f, 0, %f, 0, 1)", fPercentThroughEffect, fSecsIntoPeriod, m_fEffectPeriodSeconds) );
		fPercentThroughEffect = clamp( fPercentThroughEffect, 0, 1 );
		ASSERT_M( fPercentThroughEffect >= 0 && fPercentThroughEffect <= 1,
			ssprintf("%f", fPercentThroughEffect) );

		bool bBlinkOn = fPercentThroughEffect > 0.5f;
		float fPercentBetweenColors = RageFastSin( (fPercentThroughEffect + 0.25f) * 2 * PI ) / 2 + 0.5f;
		ASSERT_M( fPercentBetweenColors >= 0 && fPercentBetweenColors <= 1,
			ssprintf("%f, %f", fPercentBetweenColors, fPercentThroughEffect) );
		float fOriginalAlpha = m_tempState.diffuse[0].a;

		switch( m_Effect )
		{
		case diffuse_blink:
			/* XXX: Should diffuse_blink and diffuse_shift multiply the tempState color? 
			 * (That would have the same effect with 1,1,1,1, and allow tweening the diffuse
			 * while blinking and shifting.) */
			for(int i=0; i<4; i++)
			{
				m_tempState.diffuse[i] = bBlinkOn ? m_effectColor1 : m_effectColor2;
				m_tempState.diffuse[i].a *= fOriginalAlpha;	// multiply the alphas so we can fade even while an effect is playing
			}
			break;
		case diffuse_shift:
			for(int i=0; i<4; i++)
			{
				m_tempState.diffuse[i] = m_effectColor1*fPercentBetweenColors + m_effectColor2*(1.0f-fPercentBetweenColors);
				m_tempState.diffuse[i].a *= fOriginalAlpha;	// multiply the alphas so we can fade even while an effect is playing
			}
			break;
		case diffuse_ramp:
			for(int i=0; i<4; i++)
			{
				m_tempState.diffuse[i] = m_effectColor1*fPercentThroughEffect + m_effectColor2*(1.0f-fPercentThroughEffect);
				m_tempState.diffuse[i].a *= fOriginalAlpha;	// multiply the alphas so we can fade even while an effect is playing
			}
			break;
		case glow_blink:
			m_tempState.glow = bBlinkOn ? m_effectColor1 : m_effectColor2;
			m_tempState.glow.a *= fOriginalAlpha;	// don't glow if the Actor is transparent!
			break;
		case glow_shift:
			m_tempState.glow = m_effectColor1*fPercentBetweenColors + m_effectColor2*(1.0f-fPercentBetweenColors);
			m_tempState.glow.a *= fOriginalAlpha;	// don't glow if the Actor is transparent!
			break;
		case rainbow:
			m_tempState.diffuse[0] = RageColor(
				RageFastCos( fPercentBetweenColors*2*PI ) * 0.5f + 0.5f,
				RageFastCos( fPercentBetweenColors*2*PI + PI * 2.0f / 3.0f ) * 0.5f + 0.5f,
				RageFastCos( fPercentBetweenColors*2*PI + PI * 4.0f / 3.0f) * 0.5f + 0.5f,
				fOriginalAlpha );
			for( int i=1; i<4; i++ )
				m_tempState.diffuse[i] = m_tempState.diffuse[0];
			break;
		case wag:
			m_tempState.rotation += m_vEffectMagnitude * RageFastSin( fPercentThroughEffect * 2.0f * PI );
			break;
		case spin:
			// nothing needs to be here
			break;
		case vibrate:
			m_tempState.pos.x += m_vEffectMagnitude.x * randomf(-1.0f, 1.0f) * GetZoom();
			m_tempState.pos.y += m_vEffectMagnitude.y * randomf(-1.0f, 1.0f) * GetZoom();
			m_tempState.pos.z += m_vEffectMagnitude.z * randomf(-1.0f, 1.0f) * GetZoom();
			break;
		case bounce:
			{
				float fPercentOffset = RageFastSin( fPercentThroughEffect*PI ); 
				m_tempState.pos += m_vEffectMagnitude * fPercentOffset;
				m_tempState.pos.x = roundf( m_tempState.pos.x );
				m_tempState.pos.y = roundf( m_tempState.pos.y );
				m_tempState.pos.z = roundf( m_tempState.pos.z );
			}
			break;
		case bob:
			{
				float fPercentOffset = RageFastSin( fPercentThroughEffect*PI*2 ); 
				m_tempState.pos += m_vEffectMagnitude * fPercentOffset;
				m_tempState.pos.x = roundf( m_tempState.pos.x );
				m_tempState.pos.y = roundf( m_tempState.pos.y );
				m_tempState.pos.z = roundf( m_tempState.pos.z );
			}
			break;
		case pulse:
			{
				float fMinZoom = m_vEffectMagnitude[0];
				float fMaxZoom = m_vEffectMagnitude[1];
				float fPercentOffset = RageFastSin( fPercentThroughEffect*PI ); 
				float fZoom = SCALE( fPercentOffset, 0.f, 1.f, fMinZoom, fMaxZoom );
				m_tempState.scale *= fZoom;
				
				// Use the color as a Vector3 to scale the effect for added control
				RageColor c = SCALE( fPercentOffset, 0.f, 1.f, m_effectColor1, m_effectColor2 );
				m_tempState.scale.x *= c.r;
				m_tempState.scale.y *= c.g;
				m_tempState.scale.z *= c.b;
			}
			break;
		default:
			ASSERT(0);	// invalid Effect
		}
	}


	if( m_fBaseAlpha != 1 )
	{
		if( m_pTempState != &m_tempState )
		{
			m_pTempState = &m_tempState;
			m_tempState = m_current;
		}

		for( int i=0; i<4; i++ )
			m_tempState.diffuse[i].a *= m_fBaseAlpha;
	}	

	
	{
		RageMatrix m;
		RageMatrixTranslateAndScale( &m, 
			m_pTempState->pos.x,
			m_pTempState->pos.y,
			m_pTempState->pos.z,
			m_pTempState->scale.x * m_baseScale.x,
			m_pTempState->scale.y * m_baseScale.y,
			m_pTempState->scale.z * m_baseScale.z );

		DISPLAY->PreMultMatrix( m );
	}

	{
		/* The only time rotation and quat should normally be used simultaneously
		 * is for m_baseRotation.  Most objects aren't rotated at all, so optimize
		 * that case. */
		const float fRotateX = m_pTempState->rotation.x + m_baseRotation.x;
		const float fRotateY = m_pTempState->rotation.y + m_baseRotation.y;
		const float fRotateZ = m_pTempState->rotation.z + m_baseRotation.z;

		if( fRotateX != 0 || fRotateY != 0 || fRotateZ != 0 )	
		{
			RageMatrix m;
			RageMatrixRotationXYZ( &m, fRotateX, fRotateY, fRotateZ );

			DISPLAY->PreMultMatrix( m );
		}
	}

	if( m_pTempState->quat.x != 0 ||  m_pTempState->quat.y != 0 ||  m_pTempState->quat.z != 0 || m_pTempState->quat.w != 1 )
	{
		RageMatrix mat;
		RageMatrixFromQuat( &mat, m_pTempState->quat );

		DISPLAY->MultMatrix(mat);
	}

	if( m_pTempState->fSkewX != 0 )
	{
		DISPLAY->SkewX( m_pTempState->fSkewX );
	}

}

void Actor::SetGlobalRenderStates()
{
	// set Actor-defined render states
	DISPLAY->SetBlendMode( m_BlendMode );
	DISPLAY->SetZWrite( m_bZWrite );
	DISPLAY->SetZTestMode( m_ZTestMode );
	if( m_bClearZBuffer )
		DISPLAY->ClearZBuffer();
	DISPLAY->SetCullMode( m_CullMode );
}

void Actor::SetTextureRenderStates()
{
	DISPLAY->SetTextureWrapping( m_bTextureWrapping );
}

void Actor::EndDraw()
{
	DISPLAY->PopMatrix();
	m_pTempState = NULL;
}

void Actor::UpdateTweening( float fDeltaTime )
{
	while( 1 )
	{
		if( m_Tweens.empty() ) // nothing to do
			return;

		if( fDeltaTime == 0 )	// nothing will change
			return;

		// update current tween state
		// earliest tween
		TweenState &TS = m_Tweens[0].state;	
		TweenInfo  &TI = m_Tweens[0].info;

		if( TI.m_fTimeLeftInTween == TI.m_fTweenTime )	// we are just beginning this tween
		{
			m_start = m_current;		// set the start position

			// Execute the command in this tween (if any).
			if( TS.sCommandName.size() )
			{
				if( TS.sCommandName.Left(1) == "!" )
					MESSAGEMAN->Broadcast( TS.sCommandName.substr(1) );
				else
					this->PlayCommand( TS.sCommandName );
			}
		}

		float fSecsToSubtract = min( TI.m_fTimeLeftInTween, fDeltaTime );
		TI.m_fTimeLeftInTween -= fSecsToSubtract;
		fDeltaTime -= fSecsToSubtract;
	
		if( TI.m_fTimeLeftInTween == 0 )	// Current tween is over.  Stop.
		{
			m_current = TS;

			// don't inherit the queued state's command.  We keep having to do this.
			// Does sCommandName belong in TweenInfo, instead of TweenState?
			m_current.sCommandName = "";
			
			// delete the head tween
			m_Tweens.pop_front();
		}
		else		// in the middle of tweening.  Recalcute the current position.
		{
			const float fPercentThroughTween = 1-(TI.m_fTimeLeftInTween / TI.m_fTweenTime);

			// distort the percentage if appropriate
			float fPercentAlongPath = 0.f;
			switch( TI.m_TweenType )
			{
			case TWEEN_LINEAR:		fPercentAlongPath = fPercentThroughTween;													break;
			case TWEEN_ACCELERATE:	fPercentAlongPath = fPercentThroughTween * fPercentThroughTween;							break;
			case TWEEN_DECELERATE:	fPercentAlongPath = 1 - (1-fPercentThroughTween) * (1-fPercentThroughTween);				break;
			case TWEEN_BOUNCE_BEGIN:fPercentAlongPath = 1 - RageFastSin( 1.1f + fPercentThroughTween*(PI-1.1f) ) / 0.89f;				break;
			case TWEEN_BOUNCE_END:	fPercentAlongPath = RageFastSin( 1.1f + (1-fPercentThroughTween)*(PI-1.1f) ) / 0.89f;				break;
			case TWEEN_SPRING:		fPercentAlongPath = 1 - RageFastCos( fPercentThroughTween*PI*2.5f )/(1+fPercentThroughTween*3);	break;
			default:	ASSERT(0);
			}

			TweenState::MakeWeightedAverage( m_current, m_start, TS, fPercentAlongPath );
		}
	}
}

bool Actor::IsFirstUpdate() const
{
	return m_bFirstUpdate;
}

void Actor::Update( float fDeltaTime )
{
//	LOG->Trace( "Actor::Update( %f )", fDeltaTime );
	ASSERT_M( fDeltaTime >= 0, ssprintf("%f",fDeltaTime) );

	if( m_bFirstUpdate )
		m_bFirstUpdate = false;

	if( m_fHibernateSecondsLeft > 0 )
	{
		m_fHibernateSecondsLeft -= fDeltaTime;
		if( m_fHibernateSecondsLeft > 0 )
			return;

		/* Grab the leftover time. */
		fDeltaTime = -m_fHibernateSecondsLeft;
		m_fHibernateSecondsLeft = 0;
	}

	switch( m_EffectClock )
	{
	case CLOCK_TIMER:
		m_fSecsIntoEffect += fDeltaTime;
		m_fEffectDelta = fDeltaTime;

		/* Wrap the counter, so it doesn't increase indefinitely (causing loss of
		 * precision if a screen is left to sit for a day). */
		if( m_fSecsIntoEffect >= m_fEffectPeriodSeconds + m_fEffectDelay )
			m_fSecsIntoEffect -= m_fEffectPeriodSeconds + m_fEffectDelay;
		break;

	case CLOCK_BGM_BEAT:
		m_fEffectDelta = g_fCurrentBGMBeat - m_fSecsIntoEffect;
		m_fSecsIntoEffect = g_fCurrentBGMBeat;
		break;

	case CLOCK_BGM_TIME:
		m_fEffectDelta = g_fCurrentBGMTime - m_fSecsIntoEffect;
		m_fSecsIntoEffect = g_fCurrentBGMTime;
		break;
	default:
		if( m_EffectClock >= CLOCK_LIGHT_1 && m_EffectClock <= CLOCK_LIGHT_LAST )
		{
			int i = m_EffectClock - CLOCK_LIGHT_1;
			m_fEffectDelta = g_fCabinetLights[i] - m_fSecsIntoEffect;
			m_fSecsIntoEffect = g_fCabinetLights[i];
		}
		break;
	}

	// update effect
	switch( m_Effect )
	{
	case spin:
		m_current.rotation += fDeltaTime*m_vEffectMagnitude;
		wrap( m_current.rotation.x, 360 );
		wrap( m_current.rotation.y, 360 );
		wrap( m_current.rotation.z, 360 );
		break;
	}

	UpdateTweening( fDeltaTime );
}

void Actor::BeginTweening( float time, TweenType tt )
{
	ASSERT( time >= 0 );

	time = max( time, 0 );

	// If the number of tweens to ever gets this large, there's probably an infinitely 
	// recursing ActorCommand.
	if( m_Tweens.size() > 50 )
	{
		CString sError = ssprintf( "Tween overflow: size = %u.  infinitely recursing ActorCommand?", unsigned(m_Tweens.size()) );
		LOG->Warn( sError );
		Dialog::OK( sError );
		FinishTweening();
	}

	// add a new TweenState to the tail, and initialize it
	m_Tweens.resize( m_Tweens.size()+1 );

	// latest
	TweenState &TS = m_Tweens.back().state;
	TweenInfo  &TI = m_Tweens.back().info;

	if( m_Tweens.size() >= 2 )		// if there was already a TS on the stack
	{
		// initialize the new TS from the last TS in the list
		TS = m_Tweens[m_Tweens.size()-2].state;

		// don't inherit the queued state's command
		TS.sCommandName = "";
	}
	else
	{
		// This new TS is the only TS.
		// Set our tween starting and ending values to the current position.
		TS = m_current;
	}

	TI.m_TweenType = tt;
	TI.m_fTweenTime = time;
	TI.m_fTimeLeftInTween = time;
}

void Actor::StopTweening()
{
	m_Tweens.clear();
}

void Actor::FinishTweening()
{
	m_current = DestTweenState();
	StopTweening();
}

void Actor::HurryTweening( float factor )
{
	for( unsigned i = 0; i < m_Tweens.size(); ++i )
	{
		m_Tweens[i].info.m_fTimeLeftInTween *= factor;
		m_Tweens[i].info.m_fTweenTime *= factor;
	}
}

void Actor::ScaleTo( const RectF &rect, StretchType st )
{
	// width and height of rectangle
	float rect_width = rect.GetWidth();
	float rect_height = rect.GetHeight();

	if( rect_width < 0 )	SetRotationY( 180 );
	if( rect_height < 0 )	SetRotationX( 180 );

	// center of the rectangle
	float rect_cx = rect.left + rect_width/2;
	float rect_cy = rect.top  + rect_height/2;

	// zoom fActor needed to scale the Actor to fill the rectangle
	float fNewZoomX = fabsf(rect_width  / m_size.x);
	float fNewZoomY = fabsf(rect_height / m_size.y);

	float fNewZoom = 0.f;
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

void Actor::SetHorizAlignString( const CString &s )
{
	if     (s.CompareNoCase("left")==0)		this->SetHorizAlign( align_left ); /* call derived */
	else if(s.CompareNoCase("center")==0)	this->SetHorizAlign( align_center );
	else if(s.CompareNoCase("right")==0)	this->SetHorizAlign( align_right );
	else	ASSERT(0);
}

void Actor::SetVertAlignString( const CString &s )
{
	if     (s.CompareNoCase("top")==0)		this->SetVertAlign( align_top ); /* call derived */
	else if(s.CompareNoCase("middle")==0)	this->SetVertAlign( align_middle );
	else if(s.CompareNoCase("bottom")==0)	this->SetVertAlign( align_bottom );
	else	ASSERT(0);
}

void Actor::SetEffectClockString( const CString &s )
{
	if     (s.CompareNoCase("timer")==0)	this->SetEffectClock( CLOCK_TIMER );
	else if(s.CompareNoCase("beat")==0)		this->SetEffectClock( CLOCK_BGM_BEAT );
	else if(s.CompareNoCase("music")==0)	this->SetEffectClock( CLOCK_BGM_TIME );
	else if(s.CompareNoCase("bgm")==0)		this->SetEffectClock( CLOCK_BGM_BEAT ); // compat, deprecated
	else
	{
		CabinetLight cl = StringToCabinetLight( s );
		if( cl != LIGHT_INVALID )
		{
			this->SetEffectClock( (EffectClock) (cl + CLOCK_LIGHT_1) );
			return;
		}
		else
			ASSERT(0);
	}
}

void Actor::StretchTo( const RectF &r )
{
	// width and height of rectangle
	float width = r.GetWidth();
	float height = r.GetHeight();

	// center of the rectangle
	float cx = r.left + width/2.0f;
	float cy = r.top  + height/2.0f;

	// zoom fActor needed to scale the Actor to fill the rectangle
	float fNewZoomX = width  / m_size.x;
	float fNewZoomY = height / m_size.y;

	SetXY( cx, cy );
	SetZoomX( fNewZoomX );
	SetZoomY( fNewZoomY );
}




// effect "macros"

void Actor::SetEffectLua( const CString &sCommand )
{
	m_Effect = effect_lua;
	m_sEffectCommand = sCommand;
}

void Actor::SetEffectDiffuseBlink( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	if( m_Effect != diffuse_blink )
	{
		m_Effect = diffuse_blink;
		m_fEffectPeriodSeconds = fEffectPeriodSeconds;
		m_fSecsIntoEffect = 0;
	}
	m_effectColor1 = c1;
	m_effectColor2 = c2;
}

void Actor::SetEffectDiffuseShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	if( m_Effect != diffuse_shift )
	{
		m_Effect = diffuse_shift;
		m_fEffectPeriodSeconds = fEffectPeriodSeconds;
		m_fSecsIntoEffect = 0;
	}
	m_effectColor1 = c1;
	m_effectColor2 = c2;
}

void Actor::SetEffectDiffuseRamp( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	if( m_Effect != diffuse_ramp )
	{
		m_Effect = diffuse_ramp;
		m_fEffectPeriodSeconds = fEffectPeriodSeconds;
		m_fSecsIntoEffect = 0;
	}
	m_effectColor1 = c1;
	m_effectColor2 = c2;
}

void Actor::SetEffectGlowBlink( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	if( m_Effect != glow_blink )
	{
		m_Effect = glow_blink;
		m_fEffectPeriodSeconds = fEffectPeriodSeconds;
		m_fSecsIntoEffect = 0;
	}
	m_effectColor1 = c1;
	m_effectColor2 = c2;
}

void Actor::SetEffectGlowShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	if( m_Effect != glow_shift )
	{
		m_Effect = glow_shift;
		m_fEffectPeriodSeconds = fEffectPeriodSeconds;
		m_fSecsIntoEffect = 0;
	}
	m_effectColor1 = c1;
	m_effectColor2 = c2;
}

void Actor::SetEffectRainbow( float fEffectPeriodSeconds )
{
	m_Effect = rainbow;
	m_fEffectPeriodSeconds = fEffectPeriodSeconds;
	m_fSecsIntoEffect = 0;
}

void Actor::SetEffectWag( float fPeriod, RageVector3 vect )
{
	if( m_Effect != wag )
	{
		m_Effect = wag;
		m_fSecsIntoEffect = 0;
	}
	m_fEffectPeriodSeconds = fPeriod;
	m_vEffectMagnitude = vect;
}

void Actor::SetEffectBounce( float fPeriod, RageVector3 vect )
{
	m_Effect = bounce;
	m_fEffectPeriodSeconds = fPeriod;
	m_vEffectMagnitude = vect;
	m_fSecsIntoEffect = 0;
}

void Actor::SetEffectBob( float fPeriod, RageVector3 vect )
{
	if( m_Effect!=bob || m_fEffectPeriodSeconds!=fPeriod )
	{
		m_Effect = bob;
		m_fEffectPeriodSeconds = fPeriod;
		m_fSecsIntoEffect = 0;
	}
	m_vEffectMagnitude = vect;
}

void Actor::SetEffectSpin( RageVector3 vect )
{
	m_Effect = spin;
	m_vEffectMagnitude = vect;
}

void Actor::SetEffectVibrate( RageVector3 vect )
{
	m_Effect = vibrate;
	m_vEffectMagnitude = vect;
}

void Actor::SetEffectPulse( float fPeriod, float fMinZoom, float fMaxZoom )
{
	m_Effect = pulse;
	m_fEffectPeriodSeconds = fPeriod;
	m_vEffectMagnitude[0] = fMinZoom;
	m_vEffectMagnitude[1] = fMaxZoom;
}


void Actor::SetShadowLength( float fLength )
{
	m_fShadowLength = fLength;
}

void Actor::AddRotationH( float rot )
{
	RageQuatMultiply( &DestTweenState().quat, DestTweenState().quat, RageQuatFromH(rot) );
}

void Actor::AddRotationP( float rot )
{
	RageQuatMultiply( &DestTweenState().quat, DestTweenState().quat, RageQuatFromP(rot) );
}

void Actor::AddRotationR( float rot )
{
	RageQuatMultiply( &DestTweenState().quat, DestTweenState().quat, RageQuatFromR(rot) );
}

void Actor::RunCommands( const LuaReference& cmds )
{
	RunCommands2( cmds, NULL );
}

void Actor::RunCommands2( const LuaReference& cmds, Actor *pParent )
{
	// function
	cmds.PushSelf( LUA->L );
	ASSERT( !lua_isnil(LUA->L, -1) );

	// 1st parameter
	this->PushSelf( LUA->L );
	
	// 2nd parameter
	if( pParent )
		pParent->PushSelf( LUA->L );
	else
		lua_pushnil( LUA->L );

	// call function with 1 argument and 0 results
	lua_call(LUA->L, 2, 0); 
}

float Actor::GetCommandsLengthSeconds( const LuaReference& cmds )
{
	Actor temp;
	temp.RunCommands(cmds);

	return temp.GetTweenTimeLeft();
}

float Actor::GetTweenTimeLeft() const
{
	float tot = 0;

	tot += m_fHibernateSecondsLeft;

	for( unsigned i=0; i<m_Tweens.size(); ++i )
		tot += m_Tweens[i].info.m_fTimeLeftInTween;

	return tot;
}

/*
 * This is a hack to change all tween states while leaving existing tweens alone.
 *
 * Hmm.  Most commands actually act on a TweenStateAndInfo, not the Actor itself.
 * Conceptually, it wouldn't be hard to give TweenState a presence in Lua, so
 * we can simply say eg. "for x in states(Actor) do x.SetDiffuseColor(c) end".
 * However, we'd then have to give every TweenState a userdata in Lua while it's
 * being manipulated, which would add overhead ...
 */
void Actor::SetGlobalDiffuseColor( RageColor c )
{
	for(int i=0; i<4; i++) /* color, not alpha */
	{
		for( unsigned ts = 0; ts < m_Tweens.size(); ++ts )
		{
			m_Tweens[ts].state.diffuse[i].r = c.r; 
			m_Tweens[ts].state.diffuse[i].g = c.g; 
			m_Tweens[ts].state.diffuse[i].b = c.b; 
		}
		m_current.diffuse[i].r = c.r;
		m_current.diffuse[i].g = c.g;
		m_current.diffuse[i].b = c.b;
		m_start.diffuse[i].r = c.r;
		m_start.diffuse[i].g = c.g;
		m_start.diffuse[i].b = c.b;
	}
}

void Actor::SetGlobalX( float x )
{
	for( unsigned ts = 0; ts < m_Tweens.size(); ++ts )
		m_Tweens[ts].state.pos.x = x; 
	m_current.pos.x = x;
	m_start.pos.x = x;
}

void Actor::SetDiffuseColor( RageColor c )
{
	for(int i=0; i<4; i++)
	{
		DestTweenState().diffuse[i].r = c.r;
		DestTweenState().diffuse[i].g = c.g;
		DestTweenState().diffuse[i].b = c.b;
	}
}


void Actor::TweenState::Init()
{
	pos	= RageVector3( 0, 0, 0 );
	rotation = RageVector3( 0, 0, 0 );
	quat = RageVector4( 0, 0, 0, 1 );
	scale = RageVector3( 1, 1, 1 );
	fSkewX = 0;
	crop = RectF( 0,0,0,0 );
	fade = RectF( 0,0,0,0 );
	for(int i=0; i<4; i++) 
		diffuse[i] = RageColor( 1, 1, 1, 1 );
	glow = RageColor( 1, 1, 1, 0 );
	aux = 0;
}

void Actor::TweenState::MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween )
{
	average_out.pos			= ts1.pos	   + (ts2.pos		- ts1.pos	  )*fPercentBetween;
	average_out.scale		= ts1.scale	   + (ts2.scale		- ts1.scale   )*fPercentBetween;
	average_out.rotation	= ts1.rotation + (ts2.rotation	- ts1.rotation)*fPercentBetween;
	RageQuatSlerp(&average_out.quat, ts1.quat, ts2.quat, fPercentBetween);
	average_out.fSkewX		= ts1.fSkewX + (ts2.fSkewX	- ts1.fSkewX)*fPercentBetween;
	
	average_out.crop.left	= ts1.crop.left  + (ts2.crop.left	- ts1.crop.left  )*fPercentBetween;
	average_out.crop.top	= ts1.crop.top   + (ts2.crop.top	- ts1.crop.top   )*fPercentBetween;
	average_out.crop.right	= ts1.crop.right + (ts2.crop.right	- ts1.crop.right )*fPercentBetween;
	average_out.crop.bottom	= ts1.crop.bottom+ (ts2.crop.bottom	- ts1.crop.bottom)*fPercentBetween;

	average_out.fade.left	= ts1.fade.left  + (ts2.fade.left	- ts1.fade.left  )*fPercentBetween;
	average_out.fade.top	= ts1.fade.top   + (ts2.fade.top	- ts1.fade.top   )*fPercentBetween;
	average_out.fade.right	= ts1.fade.right + (ts2.fade.right	- ts1.fade.right )*fPercentBetween;
	average_out.fade.bottom	= ts1.fade.bottom+ (ts2.fade.bottom	- ts1.fade.bottom)*fPercentBetween;

	for(int i=0; i<4; i++) 
		average_out.diffuse[i]	= ts1.diffuse[i]+ (ts2.diffuse[i]	- ts1.diffuse[i])*fPercentBetween;
	average_out.glow			= ts1.glow      + (ts2.glow			- ts1.glow		)*fPercentBetween;
	average_out.aux				= ts1.aux       + (ts2.aux			- ts1.aux		)*fPercentBetween;
}

void Actor::SetBlendModeString( const CString &s )
{
	if     (s.CompareNoCase("normal")==0)	this->SetBlendMode( BLEND_NORMAL );
	else if(s.CompareNoCase("add")==0)		this->SetBlendMode( BLEND_ADD );
	else if(s.CompareNoCase("noeffect")==0)	this->SetBlendMode( BLEND_NO_EFFECT );
	else	ASSERT(0);
}

void Actor::SetCullModeString( const CString &s )
{
	if     (s.CompareNoCase("back")==0)		this->SetCullMode( CULL_BACK );
	else if(s.CompareNoCase("front")==0)	this->SetCullMode( CULL_FRONT );
	else if(s.CompareNoCase("none")==0)		this->SetCullMode( CULL_NONE );
	else	ASSERT(0);
}

void Actor::SetZTestModeString( const CString &s )
{
	// for metrics backward compatibility
	if(s.CompareNoCase("off")==0)				this->SetZTestMode( ZTEST_OFF );
	else if(s.CompareNoCase("writeonpass")==0)	this->SetZTestMode( ZTEST_WRITE_ON_PASS );
	else if(s.CompareNoCase("writeonfail")==0)	this->SetZTestMode( ZTEST_WRITE_ON_FAIL );
	else	ASSERT(0);
}

void Actor::CopyTweening( const Actor &from )
{
	m_current = from.m_current;
	m_start = from.m_start;
	m_Tweens = from.m_Tweens;
}

void Actor::Sleep( float time )
{
	BeginTweening( time, TWEEN_LINEAR );
	BeginTweening( 0, TWEEN_LINEAR ); 
}

void Actor::QueueCommand( const CString& sCommandName )
{
	BeginTweening( 0, TWEEN_LINEAR );
	DestTweenState().sCommandName = sCommandName;
}

void Actor::QueueMessage( const CString& sMessageName )
{
	// Hack: use "!" as a marker to broadcast a command, instead of playing a
	// command, so we don't have to add yet another element to every tween
	// state for this rarely-used command.
	BeginTweening( 0, TWEEN_LINEAR );
	DestTweenState().sCommandName = "!" + sMessageName;
}

void Actor::AddCommand( const CString &sCmdName, apActorCommands apac )
{
	CString sMessage;
	if( GetMessageNameFromCommandName(sCmdName, sMessage) )
	{
		SubscribeToMessage( sMessage );
		m_mapNameToCommands[sMessage] = apac;	// sCmdName w/o "Message" at the end
	}
	else
	{
		m_mapNameToCommands[sCmdName] = apac;
	}
}

bool Actor::HasCommand( const CString &sCmdName )
{
	map<CString, apActorCommands>::const_iterator it = m_mapNameToCommands.find( sCmdName );
	return it != m_mapNameToCommands.end();
}

void Actor::PlayCommand( const CString &sCommandName )
{
	PlayCommand2( sCommandName, NULL );
}

void Actor::PlayCommand2( const CString &sCommandName, Actor *pParent )
{
	map<CString, apActorCommands>::const_iterator it = m_mapNameToCommands.find( sCommandName );

	if( it == m_mapNameToCommands.end() )
		return;

	RunCommands( *it->second );
}

void Actor::HandleMessage( const CString& sMessage )
{
	PlayCommand( sMessage );
}

void Actor::SubscribeToMessage( const CString &sMessageName )
{
	MESSAGEMAN->Subscribe( this, sMessageName );
	m_vsSubscribedTo.push_back( sMessageName );
}

void Actor::SubscribeToMessage( Message message )
{
	MESSAGEMAN->Subscribe( this, message );
	m_vsSubscribedTo.push_back( MessageToString(message) );
}

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
