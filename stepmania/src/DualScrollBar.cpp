#include "global.h"
#include "DualScrollBar.h"
#include "ThemeManager.h"
#include "RageUtil.h"

DualScrollBar::DualScrollBar()
{
	m_fBarHeight = 100;
	m_fBarTime = 1;
}

void DualScrollBar::Load()
{
	int pn;

	for( pn=0; pn < NUM_PLAYERS; ++pn )
	{
		m_sprScrollThumbUnderHalf[pn].SetName( ssprintf("ThumbP%i", pn+1) );
		m_sprScrollThumbUnderHalf[pn].Load( THEME->GetPathToG( ssprintf("%s thumb p%i", m_sName.c_str(), pn+1) ) );
		this->AddChild( &m_sprScrollThumbUnderHalf[pn] );
	}

	for( pn=0; pn < NUM_PLAYERS; ++pn )
	{
		m_sprScrollThumbOverHalf[pn].SetName( ssprintf("ThumbP%i", pn+1) );
		m_sprScrollThumbOverHalf[pn].Load( THEME->GetPathToG( ssprintf("%s thumb p%i", m_sName.c_str(), pn+1) ) );
		this->AddChild( &m_sprScrollThumbOverHalf[pn] );
	}

	m_sprScrollThumbUnderHalf[0].SetCropLeft( .5f );
	m_sprScrollThumbUnderHalf[1].SetCropRight( .5f );

	m_sprScrollThumbOverHalf[0].SetCropRight( .5f );
	m_sprScrollThumbOverHalf[1].SetCropLeft( .5f );

	for( pn=0; pn < NUM_PLAYERS; ++pn )
		SetPercentage( (PlayerNumber) pn, 0 );

	FinishTweening();
}

void DualScrollBar::EnablePlayer( PlayerNumber pn, bool on )
{
	m_sprScrollThumbUnderHalf[pn].SetHidden( !on );
	m_sprScrollThumbOverHalf[pn].SetHidden( !on );
}

void DualScrollBar::SetPercentage( PlayerNumber pn, float fPercent )
{
	const float bottom = m_fBarHeight/2 - m_sprScrollThumbUnderHalf[pn].GetZoomedHeight()/2;
	const float top = -bottom;

	/* Position both thumbs. */
	m_sprScrollThumbUnderHalf[pn].StopTweening();
	m_sprScrollThumbUnderHalf[pn].BeginTweening( m_fBarTime );
	m_sprScrollThumbUnderHalf[pn].SetY( SCALE( fPercent, 0, 1, top, bottom ) );

	m_sprScrollThumbOverHalf[pn].StopTweening();
	m_sprScrollThumbOverHalf[pn].BeginTweening( m_fBarTime );
	m_sprScrollThumbOverHalf[pn].SetY( SCALE( fPercent, 0, 1, top, bottom ) );
}

/*
 * (c) 2001-2004 Glenn Maynard, Chris Danford
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
