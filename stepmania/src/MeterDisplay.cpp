#include "global.h"
/*
-----------------------------------------------------------------------------
 File: MeterDisplay.h

 Desc: The song's MeterDisplay displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "MeterDisplay.h"


MeterDisplay::MeterDisplay()
{

}

void MeterDisplay::Load( CString sSteamPath, float fStreamWidth )
{
	m_fStreamWidth = fStreamWidth;

	m_sprStream.Load( sSteamPath );
	m_sprStream.SetZoomX( fStreamWidth / m_sprStream.GetUnzoomedWidth() );

	SetPercent( 0.5f );
}

void MeterDisplay::Update( float fDelta )
{
	m_sprStream.Update( fDelta );

	Actor::Update( fDelta );
}

void MeterDisplay::DrawPrimitives()
{
	m_sprStream.Draw();
}

void MeterDisplay::SetPercent( float fPercent )
{
	ASSERT( fPercent >= 0 && fPercent <= 1 );

	m_sprStream.SetCropRight( 1-fPercent );
}
