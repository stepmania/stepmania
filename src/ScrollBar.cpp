#include "global.h"
#include "ScrollBar.h"
#include "ThemeManager.h"
#include "RageUtil.h"

#include <cmath>


ScrollBar::ScrollBar()
{
	RString sMetricsGroup = "ScrollBar";

	m_sprMiddle.Load( THEME->GetPathG(sMetricsGroup,"middle") );
	this->AddChild( m_sprMiddle );

	m_sprTop.Load( THEME->GetPathG(sMetricsGroup,"top") );
	m_sprTop->SetVertAlign( VertAlign_Bottom );
	this->AddChild( m_sprTop );

	m_sprBottom.Load( THEME->GetPathG(sMetricsGroup,"bottom") );
	m_sprBottom->SetVertAlign( VertAlign_Top );
	this->AddChild( m_sprBottom );

	m_sprScrollTickThumb.Load( THEME->GetPathG(sMetricsGroup,"TickThumb") );
	this->AddChild( m_sprScrollTickThumb );

	for( unsigned i=0; i<ARRAYLEN(m_sprScrollStretchThumb); i++ )
	{
		m_sprScrollStretchThumb[i].Load( THEME->GetPathG(sMetricsGroup,"StretchThumb") );
		this->AddChild( m_sprScrollStretchThumb[i] );
	}

	SetBarHeight( 100 );
}

void ScrollBar::SetBarHeight( int iHeight )
{
	m_iBarHeight = iHeight;
	m_sprMiddle->SetZoomY( m_iBarHeight/m_sprMiddle->GetUnzoomedHeight() );
	m_sprTop->SetY( -m_iBarHeight/2.0f );
	m_sprBottom->SetY( +m_iBarHeight/2.0f );
	m_sprScrollTickThumb->SetY( 0 );
	for( unsigned i=0; i<ARRAYLEN(m_sprScrollStretchThumb); i++ )
		m_sprScrollStretchThumb[i]->SetY( 0 );
}

void ScrollBar::SetPercentage( float fCenterPercent, float fSizePercent )
{
	wrap( fCenterPercent, 1.0f );

	const int iBarContentHeight = static_cast<int>(m_sprMiddle->GetZoomedHeight());
	ASSERT( iBarContentHeight != 0 );

	/* Set tick thumb */
	{
		float fY = SCALE( fCenterPercent, 0.0f, 1.0f, -iBarContentHeight/2.0f, iBarContentHeight/2.0f );
		fY = std::round( fY );
		m_sprScrollTickThumb->SetY( fY );
	}

	/* Set stretch thumb */
	float fStartPercent = fCenterPercent - fSizePercent;
	float fEndPercent = fCenterPercent + fSizePercent;

	// make sure the percent numbers are between 0 and 1
	fStartPercent	= std::fmod( fStartPercent+1, 1 );
	fEndPercent	= std::fmod( fEndPercent+1, 1 );

	CHECKPOINT_M("Percentages set.");
	float fPartTopY[2], fPartBottomY[2];

	if( fStartPercent < fEndPercent )	// we only need to one 1 stretch thumb part
	{
		fPartTopY[0]	= SCALE( fStartPercent,0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
		fPartBottomY[0]	= SCALE( fEndPercent,  0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
		fPartTopY[1]	= 0;
		fPartBottomY[1]	= 0;
	}
	else	// we need two stretch thumb parts
	{
		fPartTopY[0]	= SCALE( 0.0f,		0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
		fPartBottomY[0]	= SCALE( fEndPercent,	0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
		fPartTopY[1]	= SCALE( fStartPercent,	0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
		fPartBottomY[1]	= SCALE( 1.0f,		0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f );
	}

	for( unsigned i=0; i<ARRAYLEN(m_sprScrollStretchThumb); i++ )
	{
		RectF rect(
			-m_sprScrollStretchThumb[i]->GetUnzoomedWidth()/2,
			fPartTopY[i],
			+m_sprScrollStretchThumb[i]->GetUnzoomedWidth()/2,
			fPartBottomY[i]
			);
		m_sprScrollStretchThumb[i]->StretchTo( rect );
	}
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
