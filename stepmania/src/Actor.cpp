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
#include "RageMath.h"
#include "GameConstantsAndTypes.h"
#include "RageLog.h"

/* This is Reset instead of Init since many derived classes have Init() functions
 * that shouldn't change the position of the actor. */
void Actor::Reset()
{
	m_TweenStates.clear();
	m_TweenInfo.clear();

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
	m_bTextureWrapping = false;
	m_bIsAnimating = true;
	m_bBlendAdd = false;
}

Actor::Actor()
{
	Reset();
	m_bFirstUpdate = true;
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


	/* XXX: recheck rotations in here (all in degrees?) */
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

	/* The only time rotation and quat should normally be used simultaneously
	 * is for m_baseRotation. */
	if( m_temp.rotation.x != 0 )	
		DISPLAY->RotateX( m_temp.rotation.x );
	if( m_temp.rotation.y != 0 )	
		DISPLAY->RotateY( m_temp.rotation.y );
	if( m_temp.rotation.z != 0 )	
		DISPLAY->RotateZ( m_temp.rotation.z );

	if( m_temp.quat.x != 0 ||  m_temp.quat.y != 0 ||  m_temp.quat.z != 0 || m_temp.quat.w != 1 )
	{
		RageMatrix mat;
		RageMatrixFromQuat( &mat, m_temp.quat );

		DISPLAY->MultMatrix(mat);
	}
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
		if( m_current.rotation.x > 360 )	m_current.rotation.x -= 360;
		if( m_current.rotation.y > 360 )	m_current.rotation.y -= 360;
		if( m_current.rotation.z > 360 )	m_current.rotation.z -= 360;
		if( m_current.rotation.x < -360 )	m_current.rotation.x += 360;
		if( m_current.rotation.y < -360 )	m_current.rotation.y += 360;
		if( m_current.rotation.z < -360 )	m_current.rotation.z += 360;
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
	if     (s=="left")		m_HorizAlign = align_left;
	else if(s=="center")	m_HorizAlign = align_center;
	else if(s=="right")		m_HorizAlign = align_right;
	else	ASSERT(0);
}

void Actor::SetVertAlign( CString s )
{
	s.MakeLower();
	if     (s=="top")		m_VertAlign = align_top;
	else if(s=="middle")	m_VertAlign = align_middle;
	else if(s=="bottom")	m_VertAlign = align_bottom;
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
	if( fLength==0 )
		m_bShadow = false;
	else
	{
		m_fShadowLength = fLength;
		m_bShadow = true;
	}
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
	mod.rotation.x	+= (CONTAINS("spinx")?-360:0);
	mod.rotation.y	+= (CONTAINS("spiny")?-360:0);
	mod.rotation.z	+= (CONTAINS("spinz")?-360:0);
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

static CString GetParam( const CStringArray& sParams, int iIndex, int& iMaxIndexAccessed )
{
	iMaxIndexAccessed = max( iIndex, iMaxIndexAccessed );
	if( iIndex < int(sParams.size()) )
		return sParams[iIndex];
	else
		return "";
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

void Actor::Command( CString sCommandString )
{
	// OPTIMIZATION OPPORTUNITY:  sCommandString could be parsed more efficiently.

	CStringArray asCommands;
	split( sCommandString, ";", asCommands, true );
	
	for( unsigned c=0; c<asCommands.size(); c++ )
	{
		CStringArray asTokens;
		split( asCommands[c], ",", asTokens, true );

		int iMaxIndexAccessed = 0;

#define sParam(i) (GetParam(asTokens,i,iMaxIndexAccessed))
#define fParam(i) ((float)atof(sParam(i)))
#define iParam(i) (atoi(sParam(i)))
#define bParam(i) (iParam(i)!=0)

		CString& sName = asTokens[0];
		sName.MakeLower();

		// Commands that go in the tweening queue:
		if     ( sName=="sleep" )			BeginTweening( fParam(1), TWEEN_LINEAR );
		else if( sName=="linear" )			BeginTweening( fParam(1), TWEEN_LINEAR );
		else if( sName=="accelerate" )		BeginTweening( fParam(1), TWEEN_ACCELERATE );
		else if( sName=="decelerate" )		BeginTweening( fParam(1), TWEEN_DECELERATE );
		else if( sName=="bouncebegin" )		BeginTweening( fParam(1), TWEEN_BOUNCE_BEGIN );
		else if( sName=="bounceend" )		BeginTweening( fParam(1), TWEEN_BOUNCE_END );
		else if( sName=="spring" )			BeginTweening( fParam(1), TWEEN_SPRING );
		else if( sName=="stoptweening" )	{ StopTweening(); BeginTweening( 0.0001f, TWEEN_LINEAR ); }
		else if( sName=="x" )				SetX( fParam(1) );
		else if( sName=="y" )				SetY( fParam(1) );
		else if( sName=="z" )				SetZ( fParam(1) );
		else if( sName=="addx" )			SetX( GetX()+fParam(1) );
		else if( sName=="addy" )			SetY( GetY()+fParam(1) );
		else if( sName=="addz" )			SetZ( GetZ()+fParam(1) );
		else if( sName=="zoom" )			SetZoom( fParam(1) );
		else if( sName=="zoomx" )			SetZoomX( fParam(1) );
		else if( sName=="zoomy" )			SetZoomY( fParam(1) );
//		else if( sName=="zoomz" )			SetZoomZ( fParam(1) );
		else if( sName=="zoomtowidth" )		ZoomToWidth( fParam(1) );
		else if( sName=="zoomtoheight" )	ZoomToHeight( fParam(1) );
		else if( sName=="diffuse" )			SetDiffuse( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="diffuseleftedge" )		SetDiffuseLeftEdge( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="diffuserightedge" )	SetDiffuseRightEdge( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="diffusetopedge" )		SetDiffuseTopEdge( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="diffusebottomedge" )	SetDiffuseBottomEdge( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		/* Add left/right/top/bottom for alpha if needed. */
		else if( sName=="diffusealpha" )	{ for(int i = 0; i < 4; ++i) { RageColor c = GetDiffuses( i ); c.a = fParam(1); SetDiffuses( i, c ); } }
		else if( sName=="glow" )			SetGlow( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
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
		else if( sName=="effectcolor1" )	SetEffectColor1( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="effectcolor2" )	SetEffectColor2( RageColor(fParam(1),fParam(2),fParam(3),fParam(4)) );
		else if( sName=="effectperiod" )	SetEffectPeriod( fParam(1) );
		else if( sName=="effectmagnitude" )	SetEffectMagnitude( RageVector3(fParam(1),fParam(2),fParam(3)) );
		else if( sName=="scaletocover" )	{ RectI R(iParam(1), iParam(2), iParam(3), iParam(4));  ScaleToCover(R); }
		// Commands that take effect immediately (ignoring the tweening queue):
		else if( sName=="animate" )			EnableAnimation( bParam(1) );
		else if( sName=="texturewrapping" )	EnableTextureWrapping( bParam(1) );
		else if( sName=="additiveblend" )	EnableAdditiveBlend( bParam(1) );
		else
		{
			CString sError = ssprintf( "Unrecognized command name '%s' in command string '%s'.", sName.c_str(), sCommandString.c_str() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "Actor::Command", MB_OK);
#endif
		}


		if( iMaxIndexAccessed != (int)asTokens.size()-1 )
		{
			CString sError = ssprintf( "Wrong number of parameters in command '%s'.  Expected %d but there are %d.", join(",",asTokens).c_str(), iMaxIndexAccessed+1, (int)asTokens.size() );
			LOG->Warn( sError );
#if defined(WIN32) // XXX arch?
			if( DISPLAY->IsWindowed() )
				MessageBox(NULL, sError, "Actor::Command", MB_OK);
#endif
		}
	}
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

	for( unsigned i=0; i<m_TweenInfo.size(); ++i )
		tot += m_TweenInfo[i].m_fTimeLeftInTween;

	return tot;
}

void Actor::TweenState::Init()
{
	pos	= RageVector3( 0, 0, 0 );
	rotation = RageVector3( 0, 0, 0 );
	quat = RageVector4( 0, 0, 0, 1 );
	scale = RageVector3( 1, 1, 1 );
	for(int i=0; i<4; i++) 
		diffuse[i] = RageColor( 1, 1, 1, 1 );
	glow = RageColor( 1, 1, 1, 0 );
}

void Actor::TweenState::MakeWeightedAverage( TweenState& average_out, const TweenState& ts1, const TweenState& ts2, float fPercentBetween )
{
	average_out.pos			= ts1.pos	  + (ts2.pos		- ts1.pos	  )*fPercentBetween;
	average_out.scale		= ts1.scale	  + (ts2.scale		- ts1.scale   )*fPercentBetween;
	average_out.rotation	= ts1.rotation+ (ts2.rotation	- ts1.rotation)*fPercentBetween;
	RageQuatSlerp(&average_out.quat, ts1.quat, ts2.quat, fPercentBetween);

	for(int i=0; i<4; i++) 
		average_out.diffuse[i]	= ts1.diffuse[i]+ (ts2.diffuse[i]	- ts1.diffuse[i])*fPercentBetween;
	average_out.glow			= ts1.glow      + (ts2.glow			- ts1.glow		)*fPercentBetween;
}
