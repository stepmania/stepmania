#include "global.h"
#include "GrooveRadar.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "RageDisplay.h"
#include "RageMath.h"
#include "ThemeMetric.h"
#include "CommonMetrics.h"
#include "ActorUtil.h"

REGISTER_ACTOR_CLASS(GrooveRadar)

#define		LABEL_OFFSET_X( i )		THEME->GetMetricF("GrooveRadar",ssprintf("Label%dOffsetX",i+1))
#define 	LABEL_OFFSET_Y( i )		THEME->GetMetricF("GrooveRadar",ssprintf("Label%dOffsetY",i+1))
static const ThemeMetric<float>			LABEL_ON_DELAY				("GrooveRadar","LabelOnDelay");

static float RADAR_VALUE_ROTATION( int iValueIndex ) {	return PI/2 + PI*2 / 5.0f * iValueIndex; }

static const float RADAR_EDGE_WIDTH	= 2;
static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

GrooveRadar::GrooveRadar()
{
	m_sprRadarBase.Load( THEME->GetPathG("GrooveRadar","base") );
	m_Frame.AddChild( &m_sprRadarBase );
	m_Frame.SetName( "RadarFrame" );

	FOREACH_PlayerNumber( p )
	{
		m_GrooveRadarValueMap[p].SetRadius( m_sprRadarBase.GetZoomedWidth() );
		m_Frame.AddChild( &m_GrooveRadarValueMap[p] );
		m_GrooveRadarValueMap[p].RunCommands( CommonMetrics::PLAYER_COLOR.GetValue(p) );
	}

	this->AddChild( &m_Frame );

	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_sprRadarLabels[c].SetName( "Label" );
		m_sprRadarLabels[c].Load( THEME->GetPathG("GrooveRadar","labels 1x5") );
		m_sprRadarLabels[c].StopAnimating();
		m_sprRadarLabels[c].SetState( c );
		m_sprRadarLabels[c].SetXY( LABEL_OFFSET_X(c), LABEL_OFFSET_Y(c) );
		ActorUtil::LoadAllCommands( m_sprRadarLabels[c], "GrooveRadar" );
		this->AddChild( &m_sprRadarLabels[c] );
	}
}

void GrooveRadar::LoadFromNode( const RString& sDir, const XNode* pNode )
{
	ActorFrame::LoadFromNode( sDir, pNode );
}

void GrooveRadar::TweenOnScreen()
{
	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_sprRadarLabels[c].SetX( LABEL_OFFSET_X(c) );
		m_sprRadarLabels[c].PlayCommand( "PreDelayOn" );
		m_sprRadarLabels[c].BeginTweening( LABEL_ON_DELAY*c );	// sleep
		m_sprRadarLabels[c].PlayCommand( "PostDelayOn" );
	}

	m_Frame.SetZoom( 0.5f );
	m_Frame.SetRotationZ( 720 );
	m_Frame.BeginTweening( 0.6f );
	m_Frame.SetZoom( 1 );
	m_Frame.SetRotationZ( 0 );
}

void GrooveRadar::TweenOffScreen()
{
	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_sprRadarLabels[c].StopTweening();
		m_sprRadarLabels[c].BeginTweening( 0.2f );
		/* Make sure we undo glow.  We do this at the end of TweenIn,
		 * but we might tween off before we complete tweening in, and
		 * the glow can remain. */
		m_sprRadarLabels[c].SetGlow( RageColor(1,1,1,0) );
		m_sprRadarLabels[c].SetDiffuse( RageColor(1,1,1,0) );
	}

	m_Frame.BeginTweening( 0.6f );
	m_Frame.SetRotationZ( 180*4 );
	m_Frame.SetZoom( 0 );
}

void GrooveRadar::SetEmpty( PlayerNumber pn )
{
	SetFromSteps( pn, NULL );
}

void GrooveRadar::SetFromRadarValues( PlayerNumber pn, const RadarValues &rv )
{
	m_GrooveRadarValueMap[pn].SetFromSteps( rv );
}

void GrooveRadar::SetFromSteps( PlayerNumber pn, Steps* pSteps )	// NULL means no Song
{
	if( pSteps == NULL )
	{
		m_GrooveRadarValueMap[pn].SetEmpty();
		return;
	}

	const RadarValues &rv = pSteps->GetRadarValues( pn );
	m_GrooveRadarValueMap[pn].SetFromSteps( rv );
}


GrooveRadar::GrooveRadarValueMap::GrooveRadarValueMap()
{
	m_bValuesVisible = false;
	m_PercentTowardNew = 0;

	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_fValuesNew[c] = 0;
		m_fValuesOld[c] = 0;
	}
}

void GrooveRadar::GrooveRadarValueMap::SetEmpty()
{
	m_bValuesVisible = false;
}

void GrooveRadar::GrooveRadarValueMap::SetFromSteps( const RadarValues &rv )
{
	m_bValuesVisible = true;
	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		const float fValueCurrent = m_fValuesOld[c] * (1-m_PercentTowardNew) + m_fValuesNew[c] * m_PercentTowardNew;
		m_fValuesOld[c] = fValueCurrent;
		m_fValuesNew[c] = rv[c];
	}	

	if( !m_bValuesVisible )	// the values WERE invisible
		m_PercentTowardNew = 1;
	else
		m_PercentTowardNew = 0;	
}

void GrooveRadar::GrooveRadarValueMap::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_PercentTowardNew = min( m_PercentTowardNew+4.0f*fDeltaTime, 1 );
}

void GrooveRadar::GrooveRadarValueMap::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	// draw radar filling
	const float fRadius = GetUnzoomedWidth()/2.0f*1.1f;

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTextureModeModulate();
	RageSpriteVertex v[12];	// needed to draw 5 fan primitives and 10 strip primitives

	if( !m_bValuesVisible )
		return;

	//
	// use a fan to draw the volume
	//
	RageColor color = this->m_pTempState->diffuse[0];
	color.a = 0.5f;
	v[0].p = RageVector3( 0, 0, 0 );
	v[0].c = color;

	for( int i=0; i<NUM_SHOWN_RADAR_CATEGORIES+1; i++ )	// do one extra to close the fan
	{
		const int c = i%NUM_SHOWN_RADAR_CATEGORIES;
		const float fDistFromCenter = 
			( m_fValuesOld[c] * (1-m_PercentTowardNew) + m_fValuesNew[c] * m_PercentTowardNew + 0.07f ) * fRadius;
		const float fRotation = RADAR_VALUE_ROTATION(i);
		const float fX = RageFastCos(fRotation) * fDistFromCenter;
		const float fY = -RageFastSin(fRotation) * fDistFromCenter;

		v[1+i].p = RageVector3( fX, fY,	0 );
		v[1+i].c = v[0].c;
	}

	DISPLAY->DrawFan( v, NUM_SHOWN_RADAR_CATEGORIES+2 );

	//
	// use a line loop to draw the thick line
	//
	for( int i=0; i<=NUM_SHOWN_RADAR_CATEGORIES; i++ )
	{
		const int c = i%NUM_SHOWN_RADAR_CATEGORIES;
		const float fDistFromCenter = 
			( m_fValuesOld[c] * (1-m_PercentTowardNew) + m_fValuesNew[c] * m_PercentTowardNew + 0.07f ) * fRadius;
		const float fRotation = RADAR_VALUE_ROTATION(i);
		const float fX = RageFastCos(fRotation) * fDistFromCenter;
		const float fY = -RageFastSin(fRotation) * fDistFromCenter;

		v[i].p = RageVector3( fX, fY, 0 );
		v[i].c = this->m_pTempState->diffuse[0];
	}

	// TODO: Add this back in.  -Chris
//	switch( PREFSMAN->m_iPolygonRadar )
//	{
//	case 0:		DISPLAY->DrawLoop_LinesAndPoints( v, NUM_SHOWN_RADAR_CATEGORIES, RADAR_EDGE_WIDTH );	break;
//	case 1:		DISPLAY->DrawLoop_Polys( v, NUM_SHOWN_RADAR_CATEGORIES, RADAR_EDGE_WIDTH );			break;
//	default:
//	case -1:
	DISPLAY->DrawLineStrip( v, NUM_SHOWN_RADAR_CATEGORIES+1, RADAR_EDGE_WIDTH );
//	break;
//	}
}

// lua start
#include "LuaBinding.h"

class LunaGrooveRadar: public Luna<GrooveRadar>
{
public:
	static int SetFromRadarValues( T* p, lua_State *L )
	{ 
		PlayerNumber pn = Enum::Check<PlayerNumber>(L, 1);
		if( lua_isnil(L,2) )
		{
			p->SetEmpty( pn );
		}
		else
		{
			RadarValues *pRV = Luna<RadarValues>::check(L,2);
			p->SetFromRadarValues( pn, *pRV );
		}
		return 0;
	}
	static int SetEmpty( T* p, lua_State *L )		{ p->SetEmpty( Enum::Check<PlayerNumber>(L, 1) ); return 0; }
	static int tweenonscreen( T* p, lua_State *L )		{ p->TweenOnScreen(); return 0; }
	static int tweenoffscreen( T* p, lua_State *L )		{ p->TweenOffScreen(); return 0; }

	LunaGrooveRadar()
	{
		ADD_METHOD( SetFromRadarValues );
		ADD_METHOD( SetEmpty );
		ADD_METHOD( tweenonscreen );
		ADD_METHOD( tweenoffscreen );
	}
};

LUA_REGISTER_DERIVED_CLASS( GrooveRadar, ActorFrame )
// lua end

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
