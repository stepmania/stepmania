#include "global.h"	// testing updates
/*
-----------------------------------------------------------------------------
 Class: Actor

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "Actor.h"
#include <math.h>
#include "RageDisplay.h"
#include "PrefsManager.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"


Actor::Actor()
{
	m_bFirstUpdate = true;

	m_baseRotation = RageVector3( 0, 0, 0 );
	m_baseScale = RageVector3( 1, 1, 1 );
	m_size = RageVector2( 1, 1 );

	m_start.Init();
	m_current.Init();

	m_HorizAlign = align_center;
	m_VertAlign = align_middle;

	m_Effect =  no_effect;
	m_fSecsIntoEffect = 0;
	m_fEffectPeriodSeconds = 1;
	m_vEffectMagnitude = RageVector3(0,0,10);
	m_effectColor1 = RageColor(1,1,1,1);
	m_effectColor2 = RageColor(1,1,1,1);

	m_bShadow = false;
	m_fShadowLength = 4;
	m_bIsAnimating = true;
	m_bBlendAdd = false;
}

void Actor::Draw()
{
	// call the most-derived versions
	this->BeginDraw();	
	this->DrawPrimitives();	// call the most-derived version of DrawPrimitives();
	this->EndDraw();	
}

void Actor::BeginDraw()		// set the world matrix and calculate actor properties
{
	DISPLAY->PushMatrix();	// we're actually going to do some drawing in this function	

	int i;

	m_temp = m_current;


	//
	// set temporary drawing properties based on Effects 
	//
	float fPercentThroughEffect = m_fSecsIntoEffect / m_fEffectPeriodSeconds;
	bool bBlinkOn = fPercentThroughEffect > 0.5f;
	float fPercentBetweenColors = sinf( fPercentThroughEffect * 2 * PI ) / 2 + 0.5f;
	ASSERT( fPercentBetweenColors >= 0  &&  fPercentBetweenColors <= 1 );
	float fOriginalAlpha = m_temp.diffuse[0].a;

	switch( m_Effect )
	{
	case no_effect:
		break;
	case diffuse_blink:
		for(i=0; i<4; i++)
			m_temp.diffuse[i] = bBlinkOn ? m_effectColor1 : m_effectColor2;
		break;
	case diffuse_shift:
		for(i=0; i<4; i++)
			m_temp.diffuse[i] = m_effectColor1*fPercentBetweenColors + m_effectColor2*(1.0f-fPercentBetweenColors);
		break;
	case glow_blink:
		m_temp.glow = bBlinkOn ? m_effectColor1 : m_effectColor2;
		m_temp.glow.a *= fOriginalAlpha;	// don't glow if the Actor is transparent!
		break;
	case glow_shift:
		m_temp.glow = m_effectColor1*fPercentBetweenColors + m_effectColor2*(1.0f-fPercentBetweenColors);
		m_temp.glow.a *= fOriginalAlpha;	// don't glow if the Actor is transparent!
		break;
	case rainbow:
		m_temp.diffuse[0] = RageColor(
			cosf( fPercentBetweenColors*2*PI ) * 0.5f + 0.5f,
			cosf( fPercentBetweenColors*2*PI + PI * 2.0f / 3.0f ) * 0.5f + 0.5f,
			cosf( fPercentBetweenColors*2*PI + PI * 4.0f / 3.0f) * 0.5f + 0.5f,
			fOriginalAlpha );
		for( i=1; i<4; i++ )
			m_temp.diffuse[i] = m_temp.diffuse[0];
		break;
	case wag:
		m_temp.rotation = m_vEffectMagnitude * sinf( fPercentThroughEffect * 2.0f * PI );
		break;
	case spin:
		// nothing needs to be here
		break;
	case vibrate:
		m_temp.pos.x += m_vEffectMagnitude.x * randomf(-1.0f, 1.0f) * GetZoom();
		m_temp.pos.y += m_vEffectMagnitude.y * randomf(-1.0f, 1.0f) * GetZoom();
		m_temp.pos.z += m_vEffectMagnitude.z * randomf(-1.0f, 1.0f) * GetZoom();
		break;
	case bounce:
		{
			float fPercentOffset = sinf( fPercentThroughEffect*PI ); 
			m_temp.pos += m_vEffectMagnitude * fPercentOffset;
			m_temp.pos.x = roundf( m_temp.pos.x );
			m_temp.pos.y = roundf( m_temp.pos.y );
			m_temp.pos.z = roundf( m_temp.pos.z );
		}
		break;
	case bob:
		{
			float fPercentOffset = sinf( fPercentThroughEffect*PI*2 ); 
			m_temp.pos += m_vEffectMagnitude * fPercentOffset;
			m_temp.pos.x = roundf( m_temp.pos.x );
			m_temp.pos.y = roundf( m_temp.pos.y );
			m_temp.pos.z = roundf( m_temp.pos.z );
		}
		break;
	case pulse:
		{
			float fMinZoom = m_vEffectMagnitude[0];
			float fMaxZoom = m_vEffectMagnitude[1];
			float fPercentOffset = sinf( fPercentThroughEffect*PI ); 
			float fZoom = SCALE( fPercentOffset, 0.f, 1.f, fMinZoom, fMaxZoom );
			m_temp.scale = RageVector3( fZoom, fZoom, fZoom );
		}
		break;
	default:
		ASSERT(0);	// invalid Effect
	}


	m_temp.scale.x *= m_baseScale.x;
	m_temp.scale.y *= m_baseScale.y;
	m_temp.scale.z *= m_baseScale.z;
	m_temp.rotation.x += m_baseRotation.x;
	m_temp.rotation.y += m_baseRotation.y;
	m_temp.rotation.z += m_baseRotation.z;
	

	DISPLAY->Translate( m_temp.pos.x, m_temp.pos.y, m_temp.pos.z );
	DISPLAY->Scale( m_temp.scale.x, m_temp.scale.y, m_temp.scale.z );


	if( m_temp.rotation.x != 0 )	
		DISPLAY->RotateX( m_temp.rotation.x );
	if( m_temp.rotation.y != 0 )	
		DISPLAY->RotateY( m_temp.rotation.y );
	if( m_temp.rotation.z != 0 )	
		DISPLAY->RotateZ( m_temp.rotation.z );
}

void Actor::EndDraw()
{
	DISPLAY->PopMatrix();
}

void Actor::UpdateTweening( float fDeltaTime )
{
	// update tweening
	if( m_TweenStates.empty() )
		return;

	// we are performing a tween
	TweenState &TS = m_TweenStates[0];		// earliest tween
	TweenInfo  &TI = m_TweenInfo[0];		// earliest tween

	if( TI.m_fTimeLeftInTween == TI.m_fTweenTime )	// we are just beginning this tween
	{
		// set the start position
		m_start = m_current;
	}
	
	TI.m_fTimeLeftInTween -= fDeltaTime;

	if( TI.m_fTimeLeftInTween <= 0 )	// The tweening is over.  Stop the tweening
	{
		m_current = TS;
		
		// delete the head tween
		m_TweenStates.erase( m_TweenStates.begin() );
		m_TweenInfo.erase( m_TweenInfo.begin() );
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

bool Actor::IsFirstUpdate()
{
	return m_bFirstUpdate;
}

void Actor::Update( float fDeltaTime )
{
//	LOG->Trace( "Actor::Update( %f )", fDeltaTime );

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
		m_fSecsIntoEffect += fDeltaTime;
		while( m_fSecsIntoEffect >= m_fEffectPeriodSeconds )
			m_fSecsIntoEffect -= m_fEffectPeriodSeconds;
		break;
	case spin:
		m_current.rotation += fDeltaTime*m_vEffectMagnitude;
		if( m_current.rotation.x > PI*2 )	m_current.rotation.x -= PI*2;
		if( m_current.rotation.y > PI*2 )	m_current.rotation.y -= PI*2;
		if( m_current.rotation.z > PI*2 )	m_current.rotation.z -= PI*2;
		if( m_current.rotation.x < -PI*2 )	m_current.rotation.x += PI*2;
		if( m_current.rotation.y < -PI*2 )	m_current.rotation.y += PI*2;
		if( m_current.rotation.z < -PI*2 )	m_current.rotation.z += PI*2;
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

	ASSERT( m_TweenStates.size() < 50 );	// there's no reason for the number of tweens to ever go this large

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
	ASSERT( m_TweenStates.empty() );
	ASSERT( m_TweenInfo.empty() );
}



void Actor::ScaleTo( const RectI &rect, StretchType st )
{
	// width and height of rectangle
	float rect_width = (float) rect.GetWidth();
	float rect_height = (float) rect.GetHeight();

	if( rect_width < 0 )	SetRotationY( PI );
	if( rect_height < 0 )	SetRotationX( PI );

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
	m_Effect = diffuse_blink;
	m_effectColor1 = c1;
	m_effectColor2 = c2;
	m_fEffectPeriodSeconds = fEffectPeriodSeconds;
	m_fSecsIntoEffect = 0;
}

void Actor::SetEffectDiffuseShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	m_Effect = diffuse_shift;
	m_effectColor1 = c1;
	m_effectColor2 = c2;
	m_fEffectPeriodSeconds = fEffectPeriodSeconds;
	m_fSecsIntoEffect = 0;
}

void Actor::SetEffectGlowBlink( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	m_Effect = glow_blink;
	m_effectColor1 = c1;
	m_effectColor2 = c2;
	m_fEffectPeriodSeconds = fEffectPeriodSeconds;
	m_fSecsIntoEffect = 0;
}

void Actor::SetEffectGlowShift( float fEffectPeriodSeconds, RageColor c1, RageColor c2 )
{
	m_Effect = glow_shift;
	m_effectColor1 = c1;
	m_effectColor2 = c2;
	m_fEffectPeriodSeconds = fEffectPeriodSeconds;
	m_fSecsIntoEffect = 0;
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
	m_Effect = bob;
	m_fEffectPeriodSeconds = fPeriod;
	m_vEffectMagnitude = vect;
	m_fSecsIntoEffect = 0;
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


void Actor::Fade( float fSleepSeconds, CString sFadeString, float fFadeSeconds, bool bFadingOff )
{
	sFadeString.MakeLower();
	sFadeString.Replace( ' ', ',' );

	TweenState original = m_current;
	TweenState mod = m_current;

	CStringArray asBits;
	split( sFadeString, ",", asBits );
	
#define CONTAINS(needle)	(find( asBits.begin(), asBits.end(), needle ) != asBits.end())

	TweenType tt;
	if( CONTAINS("linear") )			tt = TWEEN_LINEAR;
	else if( CONTAINS("accelerate") )	tt = bFadingOff ? TWEEN_ACCELERATE : TWEEN_DECELERATE;
	else if( CONTAINS("bounce") )		tt = bFadingOff ? TWEEN_BOUNCE_BEGIN : TWEEN_BOUNCE_END;
	else if( CONTAINS("spring") )		tt = TWEEN_SPRING;
	else								tt = TWEEN_LINEAR;

	
	float fDeltaX	= (float)(CONTAINS("left")?-SCREEN_WIDTH:0) + (CONTAINS("right")?+SCREEN_HEIGHT:0);
	float fDeltaY	= (float)(CONTAINS("top")?-SCREEN_WIDTH:0)  + (CONTAINS("bottom")?+SCREEN_HEIGHT:0);
	float fDeltaZ	= (float)0;
	if( CONTAINS("far") )
	{
		fDeltaX *= 2;
		fDeltaY *= 2;
		fDeltaZ *= 2;
	}
	mod.pos.x		+= fDeltaX;
	mod.pos.y		+= fDeltaY;
	mod.pos.z		+= fDeltaZ;
	mod.rotation.x	+= (CONTAINS("spinx")?-PI*2:0);
	mod.rotation.y	+= (CONTAINS("spiny")?-PI*2:0);
	mod.rotation.z	+= (CONTAINS("spinz")?-PI*2:0);
	mod.scale.x		*= (CONTAINS("foldx")?0:1) * (CONTAINS("zoomx")||CONTAINS("zoom")?3:1);
	mod.scale.y		*= (CONTAINS("foldy")?0:1) * (CONTAINS("zoomy")||CONTAINS("zoom")?3:1);
	for( int i=0; i<4; i++ )
	{
		mod.diffuse[i] = GetDiffuse();
		mod.diffuse[i].a *= CONTAINS("fade")?0:1;
	}
	mod.glow = GetGlow();
	mod.glow.a *= CONTAINS("glow")?1:0;



	TweenState& start = bFadingOff ? original : mod;
	TweenState& end = bFadingOff ? mod : original;

	// apply tweens
	StopTweening();

	if( CONTAINS("ghost") )
	{
		// start with no glow, no alpha
		start.glow.a = 0;
		int i;
		for( i=0; i<4; i++ )
			start.diffuse[i].a = 0;
		
		m_current = start;


		TweenState mid;
		TweenState::MakeWeightedAverage( mid, start, end, 0.5 );

		// tween to full glow, no alpha
		mid.glow.a = 1;
		for( i=0; i<4; i++ )
			mid.diffuse[i].a = 0;
		BeginTweening( fFadeSeconds/2, tt );
		LatestTween() = mid;

		// snap to full alpha
		for( i=0; i<4; i++ )
			mid.diffuse[i].a = 1;
		BeginTweening( 0.0001f, tt );
		LatestTween() = mid;

		// tween to no glow
		mid.glow.a = 0;
		BeginTweening( fFadeSeconds/2, tt );
		LatestTween() = mid;
	}
	else
	{
		m_current = start;
		BeginTweening( fSleepSeconds );
		BeginTweening( fFadeSeconds, tt );
		LatestTween() = end;
	}
}

void Actor::Command( CString sCommandString )
{
	// OPTIMIZATION OPPORTUNITY:  sCommandString could be parsed much more efficiently.

	CStringArray asCommands;
	split( sCommandString, ";", asCommands, true );
	
	for( unsigned c=0; c<asCommands.size(); c++ )
	{
		CStringArray asTokens;
		split( asCommands[c], ",", asTokens, true );
		asTokens.resize(5);	// make this big so we don't go out of bounds below
							// if the user doesn't provide enough parameters, tough.

		CString& sName = asTokens[0];
		sName.MakeLower();

#define sParam(i) (asTokens[(unsigned)i+1])
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

		// Act on command
		if     ( sName=="sleep" )			BeginTweening( fParam(0), TWEEN_LINEAR );
		else if( sName=="linear" )			BeginTweening( fParam(0), TWEEN_LINEAR );
		else if( sName=="accelerate" )		BeginTweening( fParam(0), TWEEN_ACCELERATE );
		else if( sName=="decelerate" )		BeginTweening( fParam(0), TWEEN_DECELERATE );
		else if( sName=="bouncebegin" )		BeginTweening( fParam(0), TWEEN_BOUNCE_BEGIN );
		else if( sName=="bounceend" )		BeginTweening( fParam(0), TWEEN_BOUNCE_END );
		else if( sName=="spring" )			BeginTweening( fParam(0), TWEEN_SPRING );
		else if( sName=="stoptweening" )	{ StopTweening(); BeginTweening( 0.0001f, TWEEN_LINEAR ); }
		else if( sName=="x" )				SetX( fParam(0) );
		else if( sName=="y" )				SetY( fParam(0) );
		else if( sName=="xoffset" )			SetX( GetX()+fParam(0) );
		else if( sName=="yoffset" )			SetY( GetY()+fParam(0) );
		else if( sName=="zoom" )			SetZoom( fParam(0) );
		else if( sName=="zoomx" )			SetZoomX( fParam(0) );
		else if( sName=="zoomy" )			SetZoomY( fParam(0) );
		else if( sName=="zoomtowidth" )		ZoomToWidth( fParam(0) );
		else if( sName=="zoomtoheight" )	ZoomToHeight( fParam(0) );
		else if( sName=="diffuse" )			SetDiffuse( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="diffuseleftedge" )		SetDiffuseLeftEdge( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="diffuserightedge" )	SetDiffuseRightEdge( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="diffusetopedge" )		SetDiffuseTopEdge( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="diffusebottomedge" )	SetDiffuseBottomEdge( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="glow" )			SetGlow( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="rotationx" )		SetRotationX( fParam(0) );
		else if( sName=="rotationy" )		SetRotationY( fParam(0) );
		else if( sName=="rotationz" )		SetRotationZ( fParam(0) );
		else if( sName=="shadowlength" )	SetShadowLength( fParam(0) );
		else if( sName=="horizalign" )		SetHorizAlign( sParam(0) );
		else if( sName=="vertalign" )		SetVertAlign( sParam(0) );
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
		else if( sName=="effectcolor1" )	SetEffectColor1( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="effectcolor2" )	SetEffectColor2( RageColor(fParam(0),fParam(1),fParam(2),fParam(3)) );
		else if( sName=="effectperiod" )	SetEffectPeriod( fParam(0) );
		else if( sName=="effectmagnitude" )	SetEffectMagnitude( RageVector3(fParam(0),fParam(1),fParam(2)) );
		else if( sName=="startanimating" )	this->StartAnimating();
		else if( sName=="stopanimating" )	this->StopAnimating();
		else if( sName=="additiveblend" )	EnableAdditiveBlend( iParam(0)!=0 );
		else
		{
			CString sError = ssprintf( "Unrecognized command name '%s' in command string '%s'.", sName.GetString(), sCommandString.GetString() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "Actor::Command", MB_OK);
#endif

		}
	}
}

float Actor::GetTweenTimeLeft() const
{
	float tot = 0;

	for( unsigned i=0; i<m_TweenInfo.size(); ++i )
		tot += m_TweenInfo[i].m_fTimeLeftInTween;

	return tot;
}
