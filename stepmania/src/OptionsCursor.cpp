#include "global.h"
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

/*
 * (c) 2001-2003 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
