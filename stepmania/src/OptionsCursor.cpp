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
	CString sPath = THEME->GetPathTo( "Graphics", sFileName );

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
	TweenBarWidth( iWidth, 0.001f );
}

void OptionsCursor::TweenBarWidth( int iNewWidth )
{
	TweenBarWidth( iNewWidth, 0.2f );	
}

void OptionsCursor::TweenBarWidth( int iNewWidth, float fTweenTime )
{
	if( iNewWidth%2 == 1 )
		iNewWidth++;	// round up to nearest even number
	float fFrameWidth = m_sprLeft.GetUnzoomedWidth();

	m_sprLeft.StopTweening();
	m_sprMiddle.StopTweening();
	m_sprRight.StopTweening();

	m_sprLeft.BeginTweening( fTweenTime );
	m_sprMiddle.BeginTweening( fTweenTime );
	m_sprRight.BeginTweening( fTweenTime );

	m_sprMiddle.SetZoomX( iNewWidth/(float)fFrameWidth );

	m_sprLeft.SetX( -iNewWidth/2 - fFrameWidth/2 );
	m_sprRight.SetX( +iNewWidth/2 + fFrameWidth/2 );
}
