#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: TransitionKeepAlive

 Desc: See header.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionKeepAlive.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"


const float KEEP_ALIVE_TRANSITION_TIME	=	1.0f;

TransitionKeepAlive::TransitionKeepAlive()
{
	this->SetTransitionTime( KEEP_ALIVE_TRANSITION_TIME );

	m_sprLogo.Load( THEME->GetPathTo(GRAPHIC_KEEP_ALIVE) );
	m_sprLogo.SetXY( CENTER_X, CENTER_Y );
}

void TransitionKeepAlive::Update( float fDeltaTime )
{
	// hack:  Smooth out the big hickups.
	fDeltaTime = min( fDeltaTime, 1/15.0f );

	Transition::Update( fDeltaTime );
}


void TransitionKeepAlive::RenderPrimitives()
{
	float fPercentClosed = 1 - this->GetPercentageOpen();
	fPercentClosed = min( 1, fPercentClosed*2 );
	const float fPercentColor = fPercentClosed;
	const float fPercentAlpha = min( fPercentClosed * 2, 1 );

	m_sprLogo.SetDiffuseColor( D3DXCOLOR(fPercentColor,fPercentColor,fPercentColor,fPercentAlpha) );
	m_sprLogo.SetZoomY( fPercentClosed );
	if( fPercentClosed > 0 )
		m_sprLogo.Draw();
}


