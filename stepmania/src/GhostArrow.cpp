#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrow

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrow.h"
#include "PrefsManager.h"


#define SHOW_SECONDS		THEME->GetMetricF("GhostArrow","ShowSeconds")
#define ZOOM_START			THEME->GetMetricF("GhostArrow","ZoomStart")
#define ZOOM_END			THEME->GetMetricF("GhostArrow","ZoomEnd")
#define COLOR_PERFECT_START	THEME->GetMetricC("GhostArrow","ColorPerfectStart")
#define COLOR_PERFECT_END	THEME->GetMetricC("GhostArrow","ColorPerfectEnd")
#define COLOR_GREAT_START	THEME->GetMetricC("GhostArrow","ColorGreatStart")
#define COLOR_GREAT_END		THEME->GetMetricC("GhostArrow","ColorGreatEnd")
#define COLOR_GOOD_START	THEME->GetMetricC("GhostArrow","ColorGoodStart")
#define COLOR_GOOD_END		THEME->GetMetricC("GhostArrow","ColorGoodEnd")
#define COLOR_BOO_START		THEME->GetMetricC("GhostArrow","ColorBooStart")
#define COLOR_BOO_END		THEME->GetMetricC("GhostArrow","ColorBooEnd")

float g_fShowSeconds;
float g_fZoomStart, g_fZoomEnd;
D3DXCOLOR 
	g_colorPerfectStart, g_colorPerfectEnd, 
	g_colorGreatStart, g_colorGreatEnd, 
	g_colorGoodStart, g_colorGoodEnd, 
	g_colorBooStart, g_colorBooEnd;

GhostArrow::GhostArrow()
{
	g_fShowSeconds		= SHOW_SECONDS;
	g_fZoomStart		= ZOOM_START;
	g_fZoomEnd			= ZOOM_END;
	g_colorPerfectStart = COLOR_PERFECT_START;
	g_colorPerfectEnd	= COLOR_PERFECT_END;
	g_colorGreatStart	= COLOR_GREAT_START;
	g_colorGreatEnd		= COLOR_GREAT_END;
	g_colorGoodStart	= COLOR_GOOD_START;
	g_colorGoodEnd		= COLOR_GOOD_END;
	g_colorBooStart		= COLOR_BOO_START;
	g_colorBooEnd		= COLOR_BOO_END;


	SetDiffuse( D3DXCOLOR(1,1,1,0) );
}

void GhostArrow::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrow::Step( TapNoteScore score )
{
	// HACK: set the length of each frame so the animation plays in exactly 1 pop up time.
	//    We can't do this in the constructor because the image hasn't been loaded yet
	for( int i=0; i<Sprite::GetNumStates(); i++ )
		Sprite::m_fDelay[i] = g_fShowSeconds / (float)Sprite::GetNumStates();

	D3DXCOLOR colorStart, colorEnd;
	switch( score )
	{
	case TNS_PERFECT:	colorStart = g_colorPerfectStart;	colorEnd = g_colorPerfectEnd;	break;
	case TNS_GREAT:		colorStart = g_colorGreatStart;		colorEnd = g_colorGreatEnd;		break;
	case TNS_GOOD:		colorStart = g_colorGoodStart;		colorEnd = g_colorGoodEnd;		break;
	case TNS_BOO:		colorStart = g_colorBooStart;		colorEnd = g_colorBooEnd;		break;
	case TNS_MISS:		// miss should never be passed in here
	default:
		ASSERT(0);
	}

	StopTweening();
	SetDiffuse( colorStart );
	SetState( 0 );
	SetZoom( g_fZoomStart );
	
	BeginTweening( g_fShowSeconds );
	SetTweenZoom( g_fZoomEnd );
	SetTweenDiffuse( colorEnd );

	BeginTweening( 0.0001f );		// snap to invisible
	SetTweenDiffuse( D3DXCOLOR(1,1,1,0) );
}
