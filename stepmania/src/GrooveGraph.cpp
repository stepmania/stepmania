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
#include "Notes.h"
#include "RageDisplay.h"
#include "song.h"
#include "GameState.h"
#include "StyleDef.h"
#include <math.h>


const float GRAPH_EDGE_WIDTH	= 2;

GrooveGraph::GrooveGraph()
{
	for( int c=0; c<NUM_RADAR_CATEGORIES; c++ )
	{
		float fTotalWidth = 60 * NUM_RADAR_CATEGORIES;
		float fX = SCALE(c,0,NUM_RADAR_CATEGORIES-1, -fTotalWidth, +fTotalWidth );

		m_Mountains[c].SetXY( fX, -20 );
		this->AddChild( &m_Mountains[c] );

		m_sprLabels[c].Load( THEME->GetPathToG("GrooveGraph labels 1x5") );
		m_sprLabels[c].StopAnimating();
		m_sprLabels[c].SetState( c );
		m_sprLabels[c].SetXY( fX, +10 );
		this->AddChild( &m_sprLabels[c] );
	}
}

void GrooveGraph::SetFromSong( Song* pSong )
{
	float fValues[NUM_RADAR_CATEGORIES][NUM_DIFFICULTIES];
	for( int i=0; i<NUM_DIFFICULTIES; i++ )
	{
		Notes* pNotes = pSong->GetNotes( GAMESTATE->GetCurrentStyleDef()->m_NotesType, (Difficulty)i );
		const float* fRadarValues = pNotes ? pNotes->GetRadarValues() : NULL;
		for( int j=0; j<NUM_RADAR_CATEGORIES; j++ )
			fValues[j][i] = fRadarValues ? fRadarValues[j] : 0;
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
	m_PercentTowardNew = 0;

	for( int i=0; i<NUM_DIFFICULTIES; i++ )
	{
		m_fValuesNew[i] = 0;
		m_fValuesOld[i] = 0;
	}
}

void GrooveGraph::Mountain::SetValues( float fNewValues[NUM_DIFFICULTIES] )
{
	m_PercentTowardNew = 0;

	memcpy( m_fValuesOld, m_fValuesNew, sizeof(m_fValuesNew) );
	memcpy( m_fValuesNew, fNewValues, sizeof(fNewValues) );
}

void GrooveGraph::Mountain::Update( float fDeltaTime )
{
	ActorFrame::Update( fDeltaTime );

	m_PercentTowardNew = min( m_PercentTowardNew+4.0f*fDeltaTime, 1 );
}

const RageColor g_DifficultyColors[NUM_DIFFICULTIES] = 
{
	RageColor(0.5f,0.5f,1,1),	// light blue
	RageColor(1,1,0,1),	// yellow
	RageColor(1,0,0,1),	// red
	RageColor(0,1,0,1),	// green
	RageColor(0,0,1,1),	// blue
};

void GrooveGraph::Mountain::DrawPrimitives()
{
	ActorFrame::DrawPrimitives();

	DISPLAY->SetTexture( NULL );
	DISPLAY->SetTextureModeModulate();
	RageVertex v[3];

	for( int i=NUM_DIFFICULTIES-1; i>=0; i-- )
	{
		float fValue = m_fValuesOld[i] + (m_fValuesNew[i]-m_fValuesOld[i]) * m_PercentTowardNew;
		v[0].p = RageVector3( -30, -30,	0 );
		v[1].p = RageVector3( 30, -30,	0 );
		v[2].p = RageVector3( 0, -30+60*fValue,	0 );
		v[0].c = v[1].c = v[2].c = g_DifficultyColors[i];
	}

	DISPLAY->DrawFan( v, 3 );

	switch( PREFSMAN->m_iPolygonRadar )
	{
	case 0:		DISPLAY->DrawLoop_LinesAndPoints( v, 3, 2 );	break;
	case 1:		DISPLAY->DrawLoop_Polys( v, 3, 2 );			break;
	default:
	case -1:	DISPLAY->DrawLoop( v, 3, 2 );					break;
	}
}

