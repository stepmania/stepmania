#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: TransitionStarWipe.cpp

 Desc: Black bands (horizontal window blinds) gradually close.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "RageUtil.h"

#include "TransitionKeepAlive.h"
#include "ScreenDimensions.h"
#include "ThemeManager.h"


TransitionKeepAlive::TransitionKeepAlive()
{
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
	const float fPercentClosed = 1 - this->GetPercentageOpen();
	m_sprLogo.SetDiffuseColor( D3DXCOLOR(1,1,1,fPercentClosed) );
	if( fPercentClosed > 0 )
		m_sprLogo.Draw();
}


