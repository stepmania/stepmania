#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: GrooveGraph

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GrooveGraph.h"
#include "RageUtil.h"
#include "PrefsManager.h"
#include "RageBitmapTexture.h"
#include "GameConstantsAndTypes.h"
#include "ThemeManager.h"
#include "Steps.h"
#include "RageDisplay.h"
#include "song.h"
#include "GameState.h"
#include "StyleDef.h"
#include <math.h>


const float GRAPH_EDGE_WIDTH	= 2;

#define EDGE_WIDTH				THEME->GetMetricF("GrooveGraph","EdgeWidth")
#define DIFFICULTY_COLORS(dc)	THEME->GetMetricC("GrooveGraph",Capitalize(DifficultyToString(dc))+"Color")
#define SHOW_CATEGORY(cat)		THEME->GetMetricB("GrooveGraph","Show"+Capitalize(CategoryToString(cat)))
#define CATEGORY_X(cat)			THEME->GetMetricF("GrooveGraph",Capitalize(CategoryToString(cat))+"X")
#define MOUNTAINS_BASE_Y		THEME->GetMetricF("GrooveGraph","MountainsBaseY")
CachedThemeMetricF MOUNTAIN_WIDTH	("GrooveGraph","MountainWidth");
CachedThemeMetricF MOUNTAIN_HEIGHT	("GrooveGraph","MountainHeight");

RageColor	g_DifficultyColorsCache[NUM_DIFFICULTIES];

GrooveGraph::GrooveGraph()
{
	for( int i=0; i<NUM_DIFFICULTIES; i++ )
		g_DifficultyColorsCache[i] = DIFFICULTY_COLORS((Difficulty)i);
	MOUNTAIN_WIDTH.Refresh();
	MOUNTAIN_HEIGHT.Refresh();

	m_sprBase.Load( THEME->GetPathToG("GrooveGraph base") );
	this->AddChild( &m_sprBase );

	for( int c=0; c<NUM_RADAR_CATEGORIES; c++ )
	{
		if( !SHOW_CATEGORY((RadarCategory)c) )
			continue;

		m_Mountains[c].SetXY( CATEGORY_X((RadarCategory)c), MOUNTAINS_BASE_Y );
		this->AddChild( &m_Mountains[c] );
	}
}

void GrooveGraph::SetFromSong( Song* pSong )
{
	float fValues[NUM_RADAR_CATEGORIES][NUM_DIFFICULTIES];
	ZERO( fValues );

	if( pSong )
	{
		for( int i=0; i<NUM_DIFFICULTIES; i++ )
		{
			Steps* pNotes = pSong->GetStepsByDifficulty( GAMESTATE->GetCurrentStyleDef()->m_StepsType, (Difficulty)i );
			const float* fRadarValues = pNotes ? pNotes->GetRadarValues() : NULL;
			for( int j=0; j<NUM_RADAR_CATEGORIES; j++ )
				fValues[j][i] = fRadarValues ? fRadarValues[j] : 0;
		}
	}

	for( int j=0; j<NUM_RADAR_CATEGORIES; j++ )
	{
		m_Mountains[j].SetValues( fValues[j] );	
	}
}

void GrooveGraph::TweenOnScreen()
{
}

void GrooveGraph::TweenOffScreen()
{
}

GrooveGraph::Mountain::Mountain()
{
	m_fPercentTowardNew = 0;

	for( int i=0; i<NUM_DIFFICULTIES; i++ )
	{
		m_fValuesNew[i] = 0;
		m_fValuesOld[i] = 0;
	}
}

void GrooveGraph::Mountain::SetValues( float fNewValues[NUM_DIFFICULTIES] )
{
	m_fPercentTowardNew = 0;

	for( int i=0; i<NUM_DIFFICULTIES; i++ )
	{
		m_fValuesOld[i] = m_fValuesNew[i];
		m_fValuesNew[i] = fNewValues[i];
	}
}

void GrooveGraph::Mountain::Update( float fDeltaTime )
{
	Actor::Update( fDeltaTime );

	m_fPercentTowardNew += 4.0f*fDeltaTime;
	CLAMP( m_fPercentTowardNew, 0, 1 );
}

void GrooveGraph::Mountain::DrawPrimitives()
{
	DISPLAY->SetTexture( NULL );
	DISPLAY->SetTextureModeModulate();
	RageSpriteVertex v[4];

	for( int i=NUM_DIFFICULTIES-1; i>=0; i-- )
	{
		float fValue = SCALE(m_fPercentTowardNew, 0.f, 1.f, m_fValuesOld[i], m_fValuesNew[i] );
		float fHeight = MOUNTAIN_HEIGHT*fValue;
		v[0].p = v[3].p = RageVector3( -MOUNTAIN_WIDTH/2, 0,	0 );
		v[1].p = RageVector3( MOUNTAIN_WIDTH/2, 0,	0 );
		v[2].p = RageVector3( 0, -fHeight, 0 );
		v[0].c = v[1].c = v[2].c = v[3].c = g_DifficultyColorsCache[i];
		DISPLAY->DrawFan( v, 3 );
	}

	// TODO: Add this back in
	// This shouldn't be needed anymore. -glenn
//	switch( PREFSMAN->m_iPolygonRadar )
//	{
//	case 0:		DISPLAY->DrawLoop_LinesAndPoints( v, 3, 2 );	break;
//	case 1:		DISPLAY->DrawLoop_Polys( v, 3, 2 );				break;
//	default:
//	case -1:
	DISPLAY->DrawLineStrip( v, 4, 2 );
//	break;
//	}
}

