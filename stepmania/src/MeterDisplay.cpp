#include "global.h"
/*
-----------------------------------------------------------------------------
 File: MeterDisplay.h

 Desc: The song's MeterDisplay displayed in SelectSong.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

#include "MeterDisplay.h"


MeterDisplay::MeterDisplay( 
	CString sSteamPath,
	CString sFramePath,
	float fStreamWidth )
{
	m_fStreamWidth = fStreamWidth;

	m_sprStream.Load( sSteamPath );
	m_sprStream.SetZoomX( fStreamWidth / m_sprStream.GetUnzoomedWidth() );

	m_quad.SetZoomY( m_sprStream.GetUnzoomedHeight() );
	m_quad.SetX( m_fStreamWidth/2 );
	m_quad.SetHorizAlign( Actor::align_right );
	m_quad.SetDiffuse( RageColor(0,0,0,1) );

	m_sprFrame.Load( sFramePath );

	m_fTrailingPercent = 0;

	SetPercent( 0.5f );
}

MeterDisplay::~MeterDisplay()
{
}

void MeterDisplay::Update( float fDelta )
{
	fapproach( m_fTrailingPercent, m_fPercent, 0.2f*fDelta );
	m_quad.SetZoomX( m_fStreamWidth * (1-m_fTrailingPercent) );

	m_sprStream.Update( fDelta );
	m_quad.Update( fDelta );
	m_sprFrame.Update( fDelta );

	Actor::Update( fDelta );
}

void MeterDisplay::DrawPrimitives()
{
	m_sprStream.Draw();
	m_quad.Draw();
	m_sprFrame.Draw();
}

void MeterDisplay::SetPercent( float fPercent )
{
	ASSERT( fPercent >= 0 && fPercent <= 1 );

	m_fPercent = fPercent;

	m_sprFrame.Command( "glow,1,1,1,1;linear,0.5;glow,1,1,1,0" );
	m_sprStream.Command( "glow,1,1,1,1;linear,0.5;glow,1,1,1,0" );
}
