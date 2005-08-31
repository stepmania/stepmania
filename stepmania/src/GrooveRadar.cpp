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

#define		LABEL_OFFSET_X( i )		THEME->GetMetricF("GrooveRadar",ssprintf("Label%dOffsetX",i+1))
#define 	LABEL_OFFSET_Y( i )		THEME->GetMetricF("GrooveRadar",ssprintf("Label%dOffsetY",i+1))
static const ThemeMetric<apActorCommands>	LABEL_ON_COMMAND			("GrooveRadar","LabelOnCommand");
static const ThemeMetric<float>			LABEL_ON_DELAY				("GrooveRadar","LabelOnDelay");
static const ThemeMetric<apActorCommands>	LABEL_ON_COMMAND_POST_DELAY ("GrooveRadar","LabelOnCommandPostDelay");

float RADAR_VALUE_ROTATION( int iValueIndex ) {	return PI/2 + PI*2 / 5.0f * iValueIndex; }

const float RADAR_EDGE_WIDTH	= 2;
static const int NUM_SHOWN_RADAR_CATEGORIES = 5;

GrooveRadar::GrooveRadar()
{
	this->AddChild( &m_GrooveRadarValueMap );

	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_sprRadarLabels[c].Load( THEME->GetPathG("GrooveRadar","labels 1x5") );
		m_sprRadarLabels[c].StopAnimating();
		m_sprRadarLabels[c].SetState( c );
		m_sprRadarLabels[c].SetXY( LABEL_OFFSET_X(c), LABEL_OFFSET_Y(c) );
		this->AddChild( &m_sprRadarLabels[c] );
	}
}

void GrooveRadar::TweenOnScreen()
{
	for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
	{
		m_sprRadarLabels[c].SetX( LABEL_OFFSET_X(c) );
		m_sprRadarLabels[c].RunCommands( LABEL_ON_COMMAND );
		m_sprRadarLabels[c].BeginTweening( LABEL_ON_DELAY*c );	// sleep
		m_sprRadarLabels[c].RunCommands( LABEL_ON_COMMAND_POST_DELAY );
	}
	m_GrooveRadarValueMap.TweenOnScreen();
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
	m_GrooveRadarValueMap.TweenOffScreen();
}

GrooveRadar::GrooveRadarValueMap::GrooveRadarValueMap()
{
	m_sprRadarBase.Load( THEME->GetPathG("GrooveRadar","base") );
	this->AddChild( &m_sprRadarBase );

	FOREACH_PlayerNumber( p )
	{
		m_bValuesVisible[p] = false;
		m_PercentTowardNew[p] = 0;

		for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
		{
			m_fValuesNew[p][c] = 0;
			m_fValuesOld[p][c] = 0;
		}
	}
}

void GrooveRadar::GrooveRadarValueMap::SetFromSteps( PlayerNumber pn, Steps* pSteps )		// NULL means no song
{
	if( pSteps != NULL )
	{
		for( int c=0; c<NUM_SHOWN_RADAR_CATEGORIES; c++ )
		{
			const float fValueCurrent = m_fValuesOld[pn][c] * (1-m_PercentTowardNew[pn]) + m_fValuesNew[pn][c] * m_PercentTowardNew[pn];
			m_fValuesOld[pn][c] = fValueCurrent;
			m_fValuesNew[pn][c] = pSteps->GetRadarValues()[c];
		}	

		if( !m_bValuesVisible[pn] )	// the values WERE invisible
			m_PercentTowardNew[pn] = 1;
		else
			m_PercentTowardNew[pn] = 0;	

		m_bValuesVisible[pn] = true;
	}
	else	// pSteps == NULL
	{
		m_bValuesVisible[pn] = false;
	}
}

void GrooveRadar::GrooveRadarValueMap::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	FOREACH_PlayerNumber( p )
	{
		m_PercentTowardNew[p] = min( m_PercentTowardNew[p]+4.0f*fDeltaTime, 1 );
	}
}

void GrooveRadar::GrooveRadarValueMap::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	// draw radar filling
	const float fRadius = m_sprRadarBase.GetZoomedHeight()/2.0f*1.1f;

	DISPLAY->ClearAllTextures();
	DISPLAY->SetTextureModeModulate();
	RageSpriteVertex v[12];	// needed to draw 5 fan primitives and 10 strip primitives

	FOREACH_PlayerNumber( p )
	{
		if( !m_bValuesVisible[p] )
			continue;

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
				( m_fValuesOld[p][c] * (1-m_PercentTowardNew[p]) + m_fValuesNew[p][c] * m_PercentTowardNew[p] + 0.07f ) * fRadius;
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
				( m_fValuesOld[p][c] * (1-m_PercentTowardNew[p]) + m_fValuesNew[p][c] * m_PercentTowardNew[p] + 0.07f ) * fRadius;
			const float fRotation = RADAR_VALUE_ROTATION(i);
			const float fX = RageFastCos(fRotation) * fDistFromCenter;
			const float fY = -RageFastSin(fRotation) * fDistFromCenter;

			v[i].p = RageVector3( fX, fY, 0 );
			v[i].c = this->m_pTempState->diffuse[0];
		}

		// TODO: Add this back in.  -Chris
//		switch( PREFSMAN->m_iPolygonRadar )
//		{
//		case 0:		DISPLAY->DrawLoop_LinesAndPoints( v, NUM_SHOWN_RADAR_CATEGORIES, RADAR_EDGE_WIDTH );	break;
//		case 1:		DISPLAY->DrawLoop_Polys( v, NUM_SHOWN_RADAR_CATEGORIES, RADAR_EDGE_WIDTH );			break;
//		default:
//		case -1:
		DISPLAY->DrawLineStrip( v, NUM_SHOWN_RADAR_CATEGORIES+1, RADAR_EDGE_WIDTH );
//		break;
//		}
	}
}

void GrooveRadar::GrooveRadarValueMap::TweenOnScreen()
{
	SetZoom( 0.5f );
	SetRotationZ( 720 );
	BeginTweening( 0.6f );
	SetZoom( 1 );
	SetRotationZ( 0 );
}

void GrooveRadar::GrooveRadarValueMap::TweenOffScreen()
{
	BeginTweening( 0.6f );
	SetRotationZ( 180*4 );
	SetZoom( 0 );
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
