#include "global.h"
#include "Actor.h"
#include "RageDisplay.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "RageMath.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"
#include "arch/Dialog/Dialog.h"

static float g_fCurrentBGMTime = 0;

/* This is Reset instead of Init since many derived classes have Init() functions
 * that shouldn't change the position of the actor. */
void Actor::Reset()
{
	m_TweenStates.clear();
	m_TweenInfo.clear();

	m_pTempState = NULL;

	m_baseRotation = RageVector3( 0, 0, 0 );
	m_baseScale = RageVector3( 1, 1, 1 );

	m_start.Init();
	m_current.Init();

	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_Effect =  no_effect;
	m_fSecsIntoEffect = 0;
	m_fEffectPeriodSeconds = 1;
	m_fEffectDelay = 0;
	m_fEffectOffset = 0;
	m_EffectClock = CLOCK_TIMER;
	m_vEffectMagnitude = RageVector3(0,0,10);
	m_effectColor1 = RageColor(1,1,1,1);
	m_effectColor2 = RageColor(1,1,1,1);

	m_bHidden = false;
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
}

Actor::Actor()
{
	m_size = RageVector2( 1, 1 );
	Reset();
	m_bFirstUpdate = true;
}

void Actor::SetBGMTime( float fTime )
{
	g_fCurrentBGMTime = fTime;
}

void Actor::Draw()
{
	if( m_bHidden )
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



	//
	// set temporary drawing properties based on Effects 
	//
	if( m_Effect == no_effect )
	{
		m_pTempState = &m_current;
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
		float fPercentBetweenColors = sinf( (fPercentThroughEffect + 0.25f) * 2 * PI ) / 2 + 0.5f;
		ASSERT_M( fPercentBetweenColors >= 0 && fPercentBetweenColors <= 1,
			ssprintf("%f, %f", fPercentBetweenColors, fPercentThroughEffect) );
		float fOriginalAlpha = m_tempState.diffuse[0].a;

		int i;

		switch( m_Effect )
		{
		case diffuse_blink:
			/* XXX: Should diffuse_blink and diffuse_shift multiply the tempState color? 
			 * (That would have the same effect with 1,1,1,1, and allow tweening the diffuse
			 * while blinking and shifting.) */
			for(i=0; i<4; i++)
				m_tempState.diffuse[i] = bBlinkOn ? m_effectColor1 : m_effectColor2;
			break;
		case diffuse_shift:
			for(i=0; i<4; i++)
				m_tempState.diffuse[i] = m_effectColor1*fPercentBetweenColors + m_effectColor2*(1.0f-fPercentBetweenColors);
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
				cosf( fPercentBetweenColors*2*PI ) * 0.5f + 0.5f,
				cosf( fPercentBetweenColors*2*PI + PI * 2.0f / 3.0f ) * 0.5f + 0.5f,
				cosf( fPercentBetweenColors*2*PI + PI * 4.0f / 3.0f) * 0.5f + 0.5f,
				fOriginalAlpha );
			for( i=1; i<4; i++ )
				m_tempState.diffuse[i] = m_tempState.diffuse[0];
			break;
		case wag:
			m_tempState.rotation += m_vEffectMagnitude * sinf( fPercentThroughEffect * 2.0f * PI );
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
				float fPercentOffset = sinf( fPercentThroughEffect*PI ); 
				m_tempState.pos += m_vEffectMagnitude * fPercentOffset;
				m_tempState.pos.x = roundf( m_tempState.pos.x );
				m_tempState.pos.y = roundf( m_tempState.pos.y );
				m_tempState.pos.z = roundf( m_tempState.pos.z );
			}
			break;
		case bob:
			{
				float fPercentOffset = sinf( fPercentThroughEffect*PI*2 ); 
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
				float fPercentOffset = sinf( fPercentThroughEffect*PI ); 
				float fZoom = SCALE( fPercentOffset, 0.f, 1.f, fMinZoom, fMaxZoom );
				m_tempState.scale *= fZoom;
			}
			break;
		default:
			ASSERT(0);	// invalid Effect
		}
	}

	DISPLAY->Translate(
		m_pTempState->pos.x,
		m_pTempState->pos.y,
		m_pTempState->pos.z );
	DISPLAY->Scale( 
		m_pTempState->scale.x * m_baseScale.x,
		m_pTempState->scale.y * m_baseScale.y,
		m_pTempState->scale.z * m_baseScale.z );

	/* The only time rotation and quat should normally be used simultaneously
	 * is for m_baseRotation. */
	if( m_pTempState->rotation.x + m_baseRotation.x != 0 )	
		DISPLAY->RotateX( m_pTempState->rotation.x + m_baseRotation.x );
	if( m_pTempState->rotation.y + m_baseRotation.y != 0 )	
		DISPLAY->RotateY( m_pTempState->rotation.y + m_baseRotation.y );
	if( m_pTempState->rotation.z + m_baseRotation.z != 0 )	
		DISPLAY->RotateZ( m_pTempState->rotation.z + m_baseRotation.z );

	if( m_pTempState->quat.x != 0 ||  m_pTempState->quat.y != 0 ||  m_pTempState->quat.z != 0 || m_pTempState->quat.w != 1 )
	{
		RageMatrix mat;
		RageMatrixFromQuat( &mat, m_pTempState->quat );

		DISPLAY->MultMatrix(mat);
	}
}

void Actor::SetRenderStates()
{
	// set Actor-defined render states
	DISPLAY->SetTextureWrapping( m_bTextureWrapping );
	DISPLAY->SetBlendMode( m_BlendMode );
	DISPLAY->SetZWrite( m_bZWrite );
	DISPLAY->SetZTestMode( m_ZTestMode );
	if( m_bClearZBuffer )
		DISPLAY->ClearZBuffer();
	DISPLAY->SetCullMode( m_CullMode );
}

void Actor::EndDraw()
{
	DISPLAY->PopMatrix();
}

void Actor::UpdateTweening( float fDeltaTime )
{
	while( 1 )
	{
		if( m_TweenStates.empty() ) // nothing to do
			return;

		if( fDeltaTime == 0 )	// nothing will change
			return;

		// update current tween state
		TweenState &TS = m_TweenStates[0];	// earliest tween
		TweenInfo  &TI = m_TweenInfo[0];	// earliest tween

		if( TI.m_fTimeLeftInTween == TI.m_fTweenTime )	// we are just beginning this tween
		{
			m_start = m_current;		// set the start position

			// Execute the command in this tween (if any).
			const ParsedCommand &command = TS.command;
			if( !command.vTokens.empty() )
				this->HandleCommand( command );
		}

		float fSecsToSubtract = min( TI.m_fTimeLeftInTween, fDeltaTime );
		TI.m_fTimeLeftInTween -= fSecsToSubtract;
		fDeltaTime -= fSecsToSubtract;
	
		if( TI.m_fTimeLeftInTween == 0 )	// Current tween is over.  Stop.
		{
			m_current = TS;
			
			// delete the head tween
			m_TweenStates.pop_front();
			m_TweenInfo.pop_front();
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
			case TWEEN_BOUNCE_BEGIN:fPercentAlongPath = 1 - sinf( 1.1f + fPercentThroughTween*(PI-1.1f) ) / 0.89f;				break;
			case TWEEN_BOUNCE_END:	fPercentAlongPath = sinf( 1.1f + (1-fPercentThroughTween)*(PI-1.1f) ) / 0.89f;				break;
			case TWEEN_SPRING:		fPercentAlongPath = 1 - cosf( fPercentThroughTween*PI*2.5f )/(1+fPercentThroughTween*3);	break;
			default:	ASSERT(0);
			}

			TweenState::MakeWeightedAverage( m_current, m_start, TS, fPercentAlongPath );
		}
	}
}

bool Actor::IsFirstUpdate()
{
	return m_bFirstUpdate;
}

void Actor::Update( float fDeltaTime )
{
//	LOG->Trace( "Actor::Update( %f )", fDeltaTime );
	ASSERT_M( fDeltaTime >= 0, ssprintf("%f",fDeltaTime) );

	m_fHibernateSecondsLeft -= fDeltaTime;
	m_fHibernateSecondsLeft = max( 0, m_fHibernateSecondsLeft );

	if( m_fHibernateSecondsLeft > 0 )
		return;

	// update effect
	switch( m_Effect )
	{
	case no_effect:
		break;
	case diffuse_blink:
	case diffuse_shift:
	case glow_blink:
	case glow_shift:
	case rainbow:
	case wag:
	case bounce:
	case bob:
	case pulse:
		switch( m_EffectClock )
		{
		case CLOCK_TIMER:
			m_fSecsIntoEffect += fDeltaTime;
			m_fSecsIntoEffect = fmodfp( m_fSecsIntoEffect, m_fEffectPeriodSeconds + m_fEffectDelay );
			break;

		case CLOCK_BGM:
			m_fSecsIntoEffect = g_fCurrentBGMTime;
			break;
		}

		break;
	case spin:
		m_current.rotation += fDeltaTime*m_vEffectMagnitude;
		wrap( m_current.rotation.x, 360 );
		wrap( m_current.rotation.y, 360 );
		wrap( m_current.rotation.z, 360 );
		break;
	case vibrate:
		break;
	}

	UpdateTweening( fDeltaTime );

	if( m_bFirstUpdate )
		m_bFirstUpdate = false;

}

void Actor::BeginTweening( float time, TweenType tt )
{
	ASSERT( time >= 0 );

	time = max( time, 0 );

	DEBUG_ASSERT( m_TweenStates.size() < 50 );	// there's no reason for the number of tweens to ever go this large

	// add a new TweenState to the tail, and initialize it
	m_TweenStates.resize( m_TweenStates.size()+1 );
	m_TweenInfo.resize( m_TweenInfo.size()+1 );

	ASSERT( m_TweenStates.size() == m_TweenInfo.size() );

	TweenState &TS = m_TweenStates.back();	// latest
	TweenInfo  &TI = m_TweenInfo.back();	// latest

	if( m_TweenStates.size() >= 2 )		// if there was already a TS on the stack
	{
		// initialize the new TS from the last TS in the list
		TS = m_TweenStates[m_TweenStates.size()-2];
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
	m_TweenStates.clear();
	m_TweenInfo.clear();
}

void Actor::FinishTweening()
{
	m_current = DestTweenState();
	StopTweening();
}

void Actor::HurryTweening( float factor )
{
	for( unsigned i = 0; i < m_TweenInfo.size(); ++i )
	{
		m_TweenInfo[i].m_fTimeLeftInTween *= factor;
		m_TweenInfo[i].m_fTweenTime *= factor;
	}
}

void Actor::ScaleTo( const RectI &rect, StretchType st )
{
	// width and height of rectangle
	float rect_width = (float) rect.GetWidth();
	float rect_height = (float) rect.GetHeight();

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

void Actor::SetHorizAlign( CString s )
{
	s.MakeLower();
	if     (s=="left")		SetHorizAlign( align_left ); /* call derived */
	else if(s=="center")	SetHorizAlign( align_center );
	else if(s=="right")		SetHorizAlign( align_right );
	else	ASSERT(0);
}

void Actor::SetVertAlign( CString s )
{
	s.MakeLower();
	if     (s=="top")		SetVertAlign( align_top ); /* call derived */
	else if(s=="middle")	SetVertAlign( align_middle );
	else if(s=="bottom")	SetVertAlign( align_bottom );
	else	ASSERT(0);
}

void Actor::SetEffectClock( CString s )
{
	s.MakeLower();
	if     (s=="timer")		SetEffectClock( CLOCK_TIMER );
	else if(s=="bgm")		SetEffectClock( CLOCK_BGM );
	else	ASSERT(0);
}

void Actor::StretchTo( const RectI &r )
{
	RectF r2( (float)r.left, (float)r.top, (float)r.right, (float)r.bottom );
	StretchTo( r2 );
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
	m_Effect = wag;
	m_fEffectPeriodSeconds = fPeriod;
	m_vEffectMagnitude = vect;
	m_fSecsIntoEffect = 0;
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

void Actor::Command( CString sCommands )
{
	sCommands.MakeLower();

	vector<ParsedCommand> vCommands;
	ParseCommands( sCommands, vCommands );

	for( unsigned i=0; i<vCommands.size(); i++ )
		this->HandleCommand( vCommands[i] );
}

void Actor::HandleCommand( const ParsedCommand &command )
{
	HandleParams;

	const CString& sName = sParam(0);

	// Commands that go in the tweening queue:
	if     ( sName=="sleep" )			Sleep( fParam(1) );
	else if( sName=="linear" )			BeginTweening( fParam(1), TWEEN_LINEAR );
	else if( sName=="accelerate" )		BeginTweening( fParam(1), TWEEN_ACCELERATE );
	else if( sName=="decelerate" )		BeginTweening( fParam(1), TWEEN_DECELERATE );
	else if( sName=="bouncebegin" )		BeginTweening( fParam(1), TWEEN_BOUNCE_BEGIN );
	else if( sName=="bounceend" )		BeginTweening( fParam(1), TWEEN_BOUNCE_END );
	else if( sName=="spring" )			BeginTweening( fParam(1), TWEEN_SPRING );
	else if( sName=="stoptweening" )	{ StopTweening(); BeginTweening( 0.0001f, TWEEN_LINEAR ); }	// Why BeginT again? -Chris
	else if( sName=="finishtweening" )	FinishTweening();
	else if( sName=="hurrytweening" )	HurryTweening( fParam(1) );
	else if( sName=="x" )				SetX( fParam(1) );
	else if( sName=="y" )				SetY( fParam(1) );
	else if( sName=="z" )				SetZ( fParam(1) );
	else if( sName=="addx" )			SetX( GetDestX()+fParam(1) );
	else if( sName=="addy" )			SetY( GetDestY()+fParam(1) );
	else if( sName=="addz" )			SetZ( GetDestZ()+fParam(1) );
	else if( sName=="zoom" )			SetZoom( fParam(1) );
	else if( sName=="zoomx" )			SetZoomX( fParam(1) );
	else if( sName=="zoomy" )			SetZoomY( fParam(1) );
	else if( sName=="zoomz" )			SetZoomZ( fParam(1) );
	else if( sName=="zoomtowidth" )		ZoomToWidth( fParam(1) );
	else if( sName=="zoomtoheight" )	ZoomToHeight( fParam(1) );
	else if( sName=="stretchto" )		StretchTo( RectF( fParam(1), fParam(2), fParam(3), fParam(4) ) );
	else if( sName=="cropleft" )		SetCropLeft( fParam(1) );
	else if( sName=="croptop" )			SetCropTop( fParam(1) );
	else if( sName=="cropright" )		SetCropRight( fParam(1) );
	else if( sName=="cropbottom" )		SetCropBottom( fParam(1) );
	else if( sName=="fadeleft" )		SetFadeLeft( fParam(1) );
	else if( sName=="fadetop" )			SetFadeTop( fParam(1) );
	else if( sName=="faderight" )		SetFadeRight( fParam(1) );
	else if( sName=="fadebottom" )		SetFadeBottom( fParam(1) );
	else if( sName=="fadecolor" )		SetFadeDiffuseColor( cParam(1) );
	else if( sName=="diffuse" )			SetDiffuse( cParam(1) );
	else if( sName=="diffuseleftedge" )		SetDiffuseLeftEdge( cParam(1) );
	else if( sName=="diffuserightedge" )	SetDiffuseRightEdge( cParam(1) );
	else if( sName=="diffusetopedge" )		SetDiffuseTopEdge( cParam(1) );
	else if( sName=="diffusebottomedge" )	SetDiffuseBottomEdge( cParam(1) );
	/* Add left/right/top/bottom for alpha if needed. */
	else if( sName=="diffusealpha" )	SetDiffuseAlpha( fParam(1) );
	else if( sName=="diffusecolor" )	SetDiffuseColor( cParam(1) );
	else if( sName=="glow" )			SetGlow( cParam(1) );
	else if( sName=="glowmode" ) {
		if(!sParam(1).CompareNoCase("whiten"))
			SetGlowMode( GLOW_WHITEN );
		else if(!sParam(1).CompareNoCase("brighten"))
			SetGlowMode( GLOW_BRIGHTEN );
		else ASSERT(0);
	}
	else if( sName=="rotationx" )		SetRotationX( fParam(1) );
	else if( sName=="rotationy" )		SetRotationY( fParam(1) );
	else if( sName=="rotationz" )		SetRotationZ( fParam(1) );
	else if( sName=="heading" )			AddRotationH( fParam(1) );
	else if( sName=="pitch" )			AddRotationP( fParam(1) );
	else if( sName=="roll" ) 			AddRotationR( fParam(1) );
	else if( sName=="shadowlength" )	SetShadowLength( fParam(1) );
	else if( sName=="horizalign" )		SetHorizAlign( sParam(1) );
	else if( sName=="vertalign" )		SetVertAlign( sParam(1) );
	else if( sName=="diffuseblink" )	SetEffectDiffuseBlink();
	else if( sName=="diffuseshift" )	SetEffectDiffuseShift();
	else if( sName=="glowblink" )		SetEffectGlowBlink();
	else if( sName=="glowshift" )		SetEffectGlowShift();
	else if( sName=="rainbow" )			SetEffectRainbow();
	else if( sName=="wag" )				SetEffectWag();
	else if( sName=="bounce" )			SetEffectBounce();
	else if( sName=="bob" )				SetEffectBob();
	else if( sName=="pulse" )			SetEffectPulse();
	else if( sName=="spin" )			SetEffectSpin();
	else if( sName=="vibrate" )			SetEffectVibrate();
	else if( sName=="stopeffect" )		SetEffectNone();
	else if( sName=="effectcolor1" )	SetEffectColor1( cParam(1) );
	else if( sName=="effectcolor2" )	SetEffectColor2( cParam(1) );
	else if( sName=="effectperiod" )	SetEffectPeriod( fParam(1) );
	else if( sName=="effectoffset" )	SetEffectOffset( fParam(1) );
	else if( sName=="effectdelay" )		SetEffectDelay( fParam(1) );
	else if( sName=="effectclock" )		SetEffectClock( sParam(1) );
	else if( sName=="effectmagnitude" )	SetEffectMagnitude( RageVector3(fParam(1),fParam(2),fParam(3)) );
	else if( sName=="scaletocover" )	{ RectI R(iParam(1), iParam(2), iParam(3), iParam(4));  ScaleToCover(R); }
	else if( sName=="scaletofit" )		{ RectI R(iParam(1), iParam(2), iParam(3), iParam(4));  ScaleToFitInside(R); }
	// Commands that take effect immediately (ignoring the tweening queue):
	else if( sName=="animate" )			EnableAnimation( bParam(1) );
	else if( sName=="setstate" )		SetState( iParam(1) );
	else if( sName=="texturewrapping" )	SetTextureWrapping( bParam(1) );
	else if( sName=="additiveblend" )	SetBlendMode( bParam(1) ? BLEND_ADD : BLEND_NORMAL );
	else if( sName=="blend" )			SetBlendMode( sParam(1) );
	else if( sName=="zbuffer" )			SetUseZBuffer( bParam(1) );
	else if( sName=="ztest" )			SetZTestMode( bParam(1)?ZTEST_WRITE_ON_PASS:ZTEST_OFF );
	else if( sName=="ztestmode" )		SetZTestMode( sParam(1) );
	else if( sName=="zwrite" )			SetZWrite( bParam(1) );
	else if( sName=="clearzbuffer" )	SetClearZBuffer( bParam(1) );
	else if( sName=="backfacecull" )	SetCullMode( bParam(1) ? CULL_BACK : CULL_NONE );
	else if( sName=="cullmode" )		SetCullMode( sParam(1) );
	else if( sName=="hidden" )			SetHidden( bParam(1) );
	else if( sName=="hibernate" )		SetHibernate( fParam(1) );
	else if( sName=="draworder" )		SetDrawOrder( iParam(1) );
	else if( sName=="playcommand" )		PlayCommand( sParam(1) );
	else if( sName=="queuecommand" )	
	{
		ParsedCommand newcommand = command;
		newcommand.vTokens.erase( newcommand.vTokens.begin() );
		QueueCommand( newcommand );
		return;	// don't do parameter number checking
	}

	/* These are commands intended for a Sprite commands, but they will get 
	 * sent to all sub-actors (which aren't necessarily Sprites) on 
	 * GainFocus and LoseFocus.  So, don't run CheckHandledParams 
	 * on these commands. */
	else if( sName=="customtexturerect" || sName=="texcoordvelocity" || sName=="scaletoclipped" ||
		 sName=="stretchtexcoords" || sName=="position" || sName=="loop" || sName=="play" ||
		 sName=="pause" || sName=="rate" )
		return;
	else
	{
		CString sError = ssprintf( "Actor::HandleCommand: Unrecognized command name '%s'.", sName.c_str() );
		LOG->Warn( sError );
		Dialog::OK( sError );
	}

	CheckHandledParams;
}

float Actor::GetCommandLength( CString command )
{
	Actor temp;
	temp.Command(command);

	return temp.GetTweenTimeLeft();
}

float Actor::GetTweenTimeLeft() const
{
	float tot = 0;

	tot += m_fHibernateSecondsLeft;

	for( unsigned i=0; i<m_TweenInfo.size(); ++i )
		tot += m_TweenInfo[i].m_fTimeLeftInTween;

	return tot;
}

/* This is a hack to change all tween states while leaving existing tweens alone.
 *
 * Perhaps the regular Set methods should also take an optional parameter, eg.
 * "SET_TWEEN" (normal behavior) or "SET_GLOBAL" to set regardless of tweens.
 * That might be ugly, too.
 */
void Actor::SetGlobalDiffuseColor( RageColor c )
{
	for(int i=0; i<4; i++) /* color, not alpha */
	{
		for( unsigned ts = 0; ts < m_TweenStates.size(); ++ts )
		{
			m_TweenStates[ts].diffuse[i].r = c.r; 
			m_TweenStates[ts].diffuse[i].g = c.g; 
			m_TweenStates[ts].diffuse[i].b = c.b; 
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
	for( unsigned ts = 0; ts < m_TweenStates.size(); ++ts )
		m_TweenStates[ts].pos.x = x; 
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
	crop = RectF( 0,0,0,0 );
	fade = RectF( 0,0,0,0 );
	fadecolor = RageColor( 1, 1, 1, 0 );
	for(int i=0; i<4; i++) 
		diffuse[i] = RageColor( 1, 1, 1, 1 );
	glow = RageColor( 1, 1, 1, 0 );
	glowmode = GLOW_WHITEN;
}

void Actor::TweenState::MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween )
{
	average_out.pos			= ts1.pos	   + (ts2.pos		- ts1.pos	  )*fPercentBetween;
	average_out.scale		= ts1.scale	   + (ts2.scale		- ts1.scale   )*fPercentBetween;
	average_out.rotation	= ts1.rotation + (ts2.rotation	- ts1.rotation)*fPercentBetween;
	RageQuatSlerp(&average_out.quat, ts1.quat, ts2.quat, fPercentBetween);
	
	average_out.crop.left	= ts1.crop.left  + (ts2.crop.left	- ts1.crop.left  )*fPercentBetween;
	average_out.crop.top	= ts1.crop.top   + (ts2.crop.top	- ts1.crop.top   )*fPercentBetween;
	average_out.crop.right	= ts1.crop.right + (ts2.crop.right	- ts1.crop.right )*fPercentBetween;
	average_out.crop.bottom	= ts1.crop.bottom+ (ts2.crop.bottom	- ts1.crop.bottom)*fPercentBetween;

	average_out.fade.left	= ts1.fade.left  + (ts2.fade.left	- ts1.fade.left  )*fPercentBetween;
	average_out.fade.top	= ts1.fade.top   + (ts2.fade.top	- ts1.fade.top   )*fPercentBetween;
	average_out.fade.right	= ts1.fade.right + (ts2.fade.right	- ts1.fade.right )*fPercentBetween;
	average_out.fade.bottom	= ts1.fade.bottom+ (ts2.fade.bottom	- ts1.fade.bottom)*fPercentBetween;
	average_out.fadecolor	= ts1.fadecolor  + (ts2.fadecolor	- ts1.fadecolor  )*fPercentBetween;

	for(int i=0; i<4; i++) 
		average_out.diffuse[i]	= ts1.diffuse[i]+ (ts2.diffuse[i]	- ts1.diffuse[i])*fPercentBetween;
	average_out.glow			= ts1.glow      + (ts2.glow			- ts1.glow		)*fPercentBetween;
}

void Actor::SetBlendMode( CString s )
{
	s.MakeLower();
	if     (s=="normal")	SetBlendMode( BLEND_NORMAL );
	else if(s=="add")		SetBlendMode( BLEND_ADD );
	else if(s=="noeffect")	SetBlendMode( BLEND_NO_EFFECT );
	else	ASSERT(0);
}

void Actor::SetCullMode( CString s )
{
	s.MakeLower();
	if     (s=="back")	SetCullMode( CULL_BACK );
	else if(s=="front")	SetCullMode( CULL_FRONT );
	else if(s=="none")	SetCullMode( CULL_NONE );
	else	ASSERT(0);
}

void Actor::SetZTestMode( CString s )
{
	s.MakeLower();
	
	// for metrics backward compatibility
	if(s=="off")			SetZTestMode( ZTEST_OFF );
	else if(s=="writeonpass")	SetZTestMode( ZTEST_WRITE_ON_PASS );
	else if(s=="writeonfail")	SetZTestMode( ZTEST_WRITE_ON_FAIL );
	else	ASSERT(0);
}

void Actor::CopyTweening( const Actor &from )
{
	m_current = from.m_current;
	m_start = from.m_start;
	m_TweenStates = from.m_TweenStates;
	m_TweenInfo = from.m_TweenInfo;
}

void Actor::Sleep( float time )
{
	BeginTweening( time, TWEEN_LINEAR );
	BeginTweening( 0, TWEEN_LINEAR ); 
}

void Actor::QueueCommand( ParsedCommand command )
{
	BeginTweening( 0, TWEEN_LINEAR ); 
	DestTweenState().command = command; 
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
