#include "global.h"
#include "OptionsCursor.h"
#include "RageUtil.h"
#include "ThemeManager.h"
#include "ActorUtil.h"

#include <cmath>

OptionsCursor::OptionsCursor()
{
	m_iOriginalLeftX = 0;
	m_iOriginalRightX = 0;
	m_iOriginalCanGoLeftX = 0;
	m_iOriginalCanGoRightX = 0;
}

OptionsCursor::OptionsCursor( const OptionsCursor &cpy ):
	ActorFrame( cpy ),
	m_sprMiddle( cpy.m_sprMiddle ),
	m_sprLeft( cpy.m_sprLeft ),
	m_sprRight( cpy.m_sprRight ),
	m_sprCanGoLeft( cpy.m_sprCanGoLeft ),
	m_sprCanGoRight( cpy.m_sprCanGoRight ),
	m_iOriginalLeftX( cpy.m_iOriginalLeftX ),
	m_iOriginalRightX( cpy.m_iOriginalRightX ),
	m_iOriginalCanGoLeftX( cpy.m_iOriginalCanGoLeftX ),
	m_iOriginalCanGoRightX( cpy.m_iOriginalCanGoRightX )
{
	// Re-add children, or m_SubActors will point to cpy's children and not our own.
	m_SubActors.clear();
	this->AddChild( m_sprMiddle );
	this->AddChild( m_sprLeft );
	this->AddChild( m_sprRight );
	if( m_sprCanGoLeft )
		this->AddChild( m_sprCanGoLeft );
	if( m_sprCanGoRight )
		this->AddChild( m_sprCanGoRight );
}

void OptionsCursor::Load( const RString &sMetricsGroup, bool bLoadCanGos )
{
#define LOAD_SPR( spr, name ) \
	spr.Load( THEME->GetPathG(sMetricsGroup,name) ); \
	spr->SetName( name ); \
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( spr, sMetricsGroup ); \
	this->AddChild( spr );

	LOAD_SPR( m_sprMiddle, "Middle" );
	LOAD_SPR( m_sprLeft, "Left" );
	LOAD_SPR( m_sprRight, "Right" );
	if( bLoadCanGos )
	{
		LOAD_SPR( m_sprCanGoLeft, "CanGoLeft" );
		LOAD_SPR( m_sprCanGoRight, "CanGoRight" );
	}
#undef LOAD_SPR

	m_iOriginalLeftX = static_cast<int>(m_sprLeft->GetX());
	m_iOriginalRightX = static_cast<int>(m_sprRight->GetX());
	if( bLoadCanGos )
	{
		m_iOriginalCanGoLeftX = static_cast<int>(m_sprCanGoLeft->GetX());
		m_iOriginalCanGoRightX = static_cast<int>(m_sprCanGoRight->GetX());
	}

	SetCanGo( false, false );
}

void OptionsCursor::SetCanGo( bool bCanGoLeft, bool bCanGoRight )
{
	if( m_sprCanGoLeft )
	{
		m_sprCanGoLeft->EnableAnimation( bCanGoLeft );
		m_sprCanGoRight->EnableAnimation( bCanGoRight );

		m_sprCanGoLeft->SetDiffuse( bCanGoLeft ? RageColor(1,1,1,1) : RageColor(1,1,1,0) );
		m_sprCanGoRight->SetDiffuse( bCanGoRight ? RageColor(1,1,1,1) : RageColor(1,1,1,0) );
	}
}

void OptionsCursor::StopTweening()
{
	ActorFrame::StopTweening();

	m_sprMiddle->StopTweening();
	m_sprLeft->StopTweening();
	m_sprRight->StopTweening();

	if( m_sprCanGoLeft )
	{
		m_sprCanGoLeft->StopTweening();
		m_sprCanGoRight->StopTweening();
	}
}

void OptionsCursor::BeginTweening( float fSecs, TweenType tt )
{
	ActorFrame::BeginTweening( fSecs );

	m_sprMiddle->BeginTweening( fSecs );
	m_sprLeft->BeginTweening( fSecs );
	m_sprRight->BeginTweening( fSecs );

	if( m_sprCanGoLeft )
	{
		m_sprCanGoLeft->BeginTweening( fSecs );
		m_sprCanGoRight->BeginTweening( fSecs );
	}
}

void OptionsCursor::SetBarWidth( int iWidth )
{
	float fWidth = std::ceil(iWidth/2.0f)*2.0f; // round up to nearest even number

	m_sprMiddle->ZoomToWidth( fWidth );

	m_sprLeft->SetX( m_iOriginalLeftX - fWidth/2 );
	m_sprRight->SetX( m_iOriginalRightX + fWidth/2 );
	if( m_sprCanGoLeft )
	{
		m_sprCanGoLeft->SetX( m_iOriginalCanGoLeftX - fWidth/2 );
		m_sprCanGoRight->SetX( m_iOriginalCanGoRightX + fWidth/2 );
	}
}

int OptionsCursor::GetBarWidth() const
{
	float fWidth = m_sprMiddle->GetZoomX() * m_sprMiddle->GetUnzoomedWidth();
	return (int)fWidth;
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
