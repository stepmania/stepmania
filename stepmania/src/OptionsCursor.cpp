#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: OptionsCursor

 Desc: A graphic displayed in the OptionsCursor during Dancing.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "OptionsCursor.h"
#include "RageUtil.h"
#include "GameConstantsAndTypes.h"
#include "PrefsManager.h"
#include "ThemeManager.h"


OptionsCursor::OptionsCursor()
{
	this->AddChild( &m_sprMiddle );
	this->AddChild( &m_sprLeft );
	this->AddChild( &m_sprRight );
}

void OptionsCursor::Load( PlayerNumber pn, bool bUnderline )
{
	CString sFileName = bUnderline ? "OptionsCursor underline" : "OptionsCursor cursor";
	CString sPath = THEME->GetPathToG(sFileName);

	m_sprLeft.Load( sPath );
	m_sprMiddle.Load( sPath );
	m_sprRight.Load( sPath );

	m_sprLeft.StopAnimating();
	m_sprMiddle.StopAnimating();
	m_sprRight.StopAnimating();

	int iBaseFrameNo;
	switch( pn )
	{
	case PLAYER_1:		iBaseFrameNo = 0;	break;
	case PLAYER_2:		iBaseFrameNo = 3;	break;
	default:			ASSERT(0);			return;
	}
	m_sprLeft.SetState(   iBaseFrameNo+0 );
	m_sprMiddle.SetState( iBaseFrameNo+1 );
	m_sprRight.SetState(  iBaseFrameNo+2 );
}

void OptionsCursor::SetBarWidth( int iWidth )
{
	if( iWidth%2 == 1 )
		iWidth++;	// round up to nearest even number
	float fFrameWidth = m_sprLeft.GetUnzoomedWidth();

	m_sprMiddle.SetZoomX( iWidth/(float)fFrameWidth );

	m_sprLeft.SetX( -iWidth/2 - fFrameWidth/2 );
	m_sprRight.SetX( +iWidth/2 + fFrameWidth/2 );
}

void OptionsCursor::TweenBarWidth( int iNewWidth )
{
	m_sprLeft.StopTweening();
	m_sprMiddle.StopTweening();
	m_sprRight.StopTweening();

	m_sprLeft.BeginTweening( 0.2f );
	m_sprMiddle.BeginTweening( 0.2f );
	m_sprRight.BeginTweening( 0.2f );

	SetBarWidth( iNewWidth );	
}
