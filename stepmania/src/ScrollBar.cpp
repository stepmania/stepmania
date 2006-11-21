#include "global.h"
#include "ScrollBar.h"
#include "ThemeManager.h"
#include "RageUtil.h"


ScrollBar::ScrollBar()
{
	m_sprBackground.Load( THEME->GetPathG("ScrollBar","parts 1x3") );
	m_sprBackground->StopAnimating();
	m_sprBackground->SetState( 1 );
	this->AddChild( m_sprBackground );

	m_sprScrollThumbPart1.Load( THEME->GetPathG("ScrollBar","thumb") );
	m_sprScrollThumbPart1->StopAnimating();
	this->AddChild( m_sprScrollThumbPart1 );

	m_sprScrollThumbPart2.Load( THEME->GetPathG("ScrollBar","thumb") );
	m_sprScrollThumbPart2->StopAnimating();
	this->AddChild( m_sprScrollThumbPart2 );

	m_sprTopButton.Load( THEME->GetPathG("ScrollBar","parts 1x3") );
	m_sprTopButton->StopAnimating();
	m_sprTopButton->SetState( 0 );
	this->AddChild( m_sprTopButton );

	m_sprBottomButton.Load( THEME->GetPathG("ScrollBar","parts 1x3") );
	m_sprBottomButton->StopAnimating();
	m_sprBottomButton->SetState( 2 );
	this->AddChild( m_sprBottomButton );

	SetBarHeight( 100 );
}

void ScrollBar::SetBarHeight( int iHeight )
{
	m_iBarHeight = iHeight;
	m_sprBackground->SetZoomY( m_iBarHeight/m_sprBackground->GetUnzoomedHeight() );
	m_sprTopButton->SetY( -m_iBarHeight/2.0f );
	m_sprBottomButton->SetY( +m_iBarHeight/2.0f );
	m_sprScrollThumbPart1->SetY( 0 );
	m_sprScrollThumbPart2->SetY( 0 );
}

void ScrollBar::SetPercentage( float fStartPercent, float fEndPercent )
{
	CHECKPOINT;
	const int iBarContentHeight = m_iBarHeight - int( m_sprTopButton->GetUnzoomedHeight()/2 + m_sprBottomButton->GetUnzoomedHeight()/2 );
	ASSERT( iBarContentHeight != 0 );
	CHECKPOINT;

	// make sure the percent numbers are between 0 and 1
	fStartPercent	= fmodf( fStartPercent+1, 1 );
	fEndPercent		= fmodf( fEndPercent+1, 1 );

	CHECKPOINT;
	float fPart1TopY, fPart1BottomY, fPart2TopY, fPart2BottomY;

	if( fStartPercent < fEndPercent )	// we only need to one 1 thumb part
	{
		fPart1TopY		= SCALE( fStartPercent,0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
		fPart1BottomY	= SCALE( fEndPercent,  0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
		fPart2TopY		= 0; 
		fPart2BottomY	= 0; 
	}
	else	// we need two thumb parts
	{
		fPart1TopY		= SCALE( 0.0f,			0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
		fPart1BottomY	= SCALE( fEndPercent,	0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
		fPart2TopY		= SCALE( fStartPercent,0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
		fPart2BottomY	= SCALE( 1.0f,			0.0f, 1.0f, -iBarContentHeight/2.0f, +iBarContentHeight/2.0f ); 
	}

	CHECKPOINT;
		
	m_sprScrollThumbPart1->StretchTo( RectF(
		-m_sprScrollThumbPart1->GetUnzoomedWidth()/2,
		fPart1TopY,
		+m_sprScrollThumbPart1->GetUnzoomedWidth()/2,
		fPart1BottomY
		) );

	CHECKPOINT;
	m_sprScrollThumbPart2->StretchTo( RectF(
		-m_sprScrollThumbPart2->GetUnzoomedWidth()/2,
		fPart2TopY,
		+m_sprScrollThumbPart2->GetUnzoomedWidth()/2,
		fPart2BottomY
		) );
	CHECKPOINT;
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
