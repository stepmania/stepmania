#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: GhostArrowBright

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Ben Nordstrom
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "GhostArrowBright.h"
#include "PrefsManager.h"


#define POP_UP_SECONDS		THEME->GetMetricF("GhostArrowBright","PopUpSeconds")
#define ZOOM_START			THEME->GetMetricF("GhostArrowBright","ZoomStart")
#define ZOOM_END			THEME->GetMetricF("GhostArrowBright","ZoomEnd")
#define COLOR_PERFECT		THEME->GetMetricC("GhostArrowBright","ColorPerfect")
#define COLOR_GREAT			THEME->GetMetricC("GhostArrowBright","ColorGreat")
#define COLOR_GOOD			THEME->GetMetricC("GhostArrowBright","ColorGood")
#define COLOR_BOO			THEME->GetMetricC("GhostArrowBright","ColorBoo")


GhostArrowBright::GhostArrowBright()
{
	m_fPopUpSeconds = POP_UP_SECONDS;
	m_fZoomStart = ZOOM_START;
	m_fZoomEnd = ZOOM_END;
	m_colorPerfect = COLOR_PERFECT;
	m_colorGreat = COLOR_GREAT;
	m_colorGood = COLOR_GOOD;
	m_colorBoo = COLOR_BOO;
	SetDiffuseColor( D3DXCOLOR(1,1,1,0) );
	TurnShadowOff();
}

void GhostArrowBright::Update( float fDeltaTime )
{
	Sprite::Update( fDeltaTime );
}

void GhostArrowBright::Step( TapNoteScore score )
{
	D3DXCOLOR color;
	switch( score )
	{
	case TNS_PERFECT:	color = m_colorPerfect;	break;
	case TNS_GREAT:		color = m_colorGreat;	break;
	case TNS_GOOD:		color = m_colorGood;	break;
	case TNS_BOO:		color = m_colorBoo;		break;
	case TNS_MISS:		// miss should never be passed in here
	default:
		ASSERT(0);
	}

	StopTweening();
	SetDiffuseColor( color );
	SetState( 0 );
	SetZoom( m_fZoomStart );
	BeginTweening( m_fPopUpSeconds );
	SetTweenZoom( m_fZoomEnd );
	color.a = 0;
	SetTweenDiffuseColor( color );
}
