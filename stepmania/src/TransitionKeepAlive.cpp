#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionKeepAlive

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


#include "TransitionKeepAlive.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "RageMusic.h"


#define STOP_MUSIC_ON_NEXT		THEME->GetMetricB("TransitionKeepAlive","StopMusicOnNext")
#define KEEP_ALIVE_TYPE			THEME->GetMetricI("TransitionKeepAlive","KeepAliveType")
enum KeepAliveType		// for use with the metric above
{
	KEEP_ALIVE_TYPE_MAX = 0,
	KEEP_ALIVE_TYPE_5TH,
};


KeepAliveType g_KeepAliveType;		// hack:  This is just a cache so we don't have to read the ThemeMetric on every update


const float KEEP_ALIVE_FORWARD_TRANSITION_TIME	=	0.8f;
const float KEEP_ALIVE_BACKWARD_TRANSITION_TIME	=	0.4f;


TransitionKeepAlive::TransitionKeepAlive()
{
	g_KeepAliveType = (KeepAliveType)KEEP_ALIVE_TYPE;

	m_sprLogo.Load( THEME->GetPathTo("Graphics","keep alive") );
	m_sprLogo.SetXY( CENTER_X, CENTER_Y );
}

void TransitionKeepAlive::Update( float fDeltaTime )
{
	// hack:  Smooth out the big hickups.
	fDeltaTime = min( fDeltaTime, 1/15.0f );

	Transition::Update( fDeltaTime );
}


void TransitionKeepAlive::DrawPrimitives()
{
	// draw keep alive graphic
	switch( g_KeepAliveType )
	{
	case KEEP_ALIVE_TYPE_MAX:
		{
			float fPercentClosed = 1 - this->GetPercentageOpen();
			fPercentClosed = min( 1, fPercentClosed*2 );
			const float fPercentColor = fPercentClosed;
			const float fPercentAlpha = min( fPercentClosed * 2, 1 );

			m_sprLogo.SetDiffuse( RageColor(fPercentColor,fPercentColor,fPercentColor,fPercentAlpha) );
			m_sprLogo.SetZoomY( fPercentClosed );
			if( fPercentClosed > 0 )
				m_sprLogo.Draw();
		}
		break;
	case KEEP_ALIVE_TYPE_5TH:
		{
			float fPercentClosed = 1 - this->GetPercentageOpen();
			m_sprLogo.SetDiffuse( RageColor(1,1,1,fPercentClosed) );
			m_sprLogo.Draw();
		}
		break;
	default:
		ASSERT(0);
		break;
	}

}


void TransitionKeepAlive::OpenWipingRight( ScreenMessage send_when_done )
{
	this->SetTransitionTime( KEEP_ALIVE_FORWARD_TRANSITION_TIME );

	Transition::OpenWipingRight( send_when_done );
}

void TransitionKeepAlive::OpenWipingLeft(  ScreenMessage send_when_done )
{
	if( STOP_MUSIC_ON_NEXT )
		MUSIC->Stop();

	this->SetTransitionTime( KEEP_ALIVE_BACKWARD_TRANSITION_TIME );

	Transition::OpenWipingLeft( send_when_done );
}

void TransitionKeepAlive::CloseWipingRight(ScreenMessage send_when_done )
{
	this->SetTransitionTime( KEEP_ALIVE_FORWARD_TRANSITION_TIME );

	Transition::CloseWipingRight( send_when_done );
}

void TransitionKeepAlive::CloseWipingLeft( ScreenMessage send_when_done )
{
	this->SetTransitionTime( KEEP_ALIVE_BACKWARD_TRANSITION_TIME );

	Transition::CloseWipingLeft( send_when_done );
}

