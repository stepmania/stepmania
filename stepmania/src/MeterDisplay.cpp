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

void MeterDisplay::Load( CString sStreamPath, float fStreamWidth, CString sTipPath )
{
	m_fStreamWidth = fStreamWidth;

	m_sprStream.Load( sStreamPath );
	m_sprStream.SetZoomX( fStreamWidth / m_sprStream.GetUnzoomedWidth() );

	m_sprTip.Load( sTipPath );

	this->AddChild( &m_sprStream );
	this->AddChild( m_sprTip );

	SetPercent( 0.5f );
}

void MeterDisplay::SetPercent( float fPercent )
{
	ASSERT( fPercent >= 0 && fPercent <= 1 );

	m_sprStream.SetCropRight( 1-fPercent );

	m_sprTip->SetX( SCALE(fPercent, 0.f, 1.f, -m_fStreamWidth/2, m_fStreamWidth/2) );
}
