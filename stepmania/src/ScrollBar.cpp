#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScrollBar

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScrollBar.h"
#include "ThemeManager.h"
#include "GameConstantsAndTypes.h"


const float BAR_HEIGHT = 376;

ScrollBar::ScrollBar()
{
	m_sprBackground.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SCROLLBAR) );
	m_sprBackground.StopAnimating();
	m_sprBackground.SetState( 2 );
	m_sprBackground.SetZoomY( BAR_HEIGHT/m_sprBackground.GetUnzoomedHeight() );
	this->AddActor( &m_sprBackground );

	m_sprTopButton.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SCROLLBAR) );
	m_sprTopButton.StopAnimating();
	m_sprTopButton.SetState( 0 );
	m_sprTopButton.SetY( -BAR_HEIGHT/2 );
	this->AddActor( &m_sprTopButton );

	m_sprBottomButton.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SCROLLBAR) );
	m_sprBottomButton.StopAnimating();
	m_sprBottomButton.SetState( 3 );
	m_sprBottomButton.SetY( BAR_HEIGHT/2 );
	this->AddActor( &m_sprBottomButton );

	m_sprScrollThumb.Load( THEME->GetPathTo(GRAPHIC_SELECT_MUSIC_SCROLLBAR) );
	m_sprScrollThumb.StopAnimating();
	m_sprScrollThumb.SetState( 1 );
	m_sprScrollThumb.SetY( CENTER_Y );
	this->AddActor( &m_sprScrollThumb );
}

void ScrollBar::SetPercentage( float fPercentageFromTop )
{
	if( fPercentageFromTop < 0 )	fPercentageFromTop += 1;
	fPercentageFromTop = fmodf( fPercentageFromTop, 1 );
	m_sprScrollThumb.SetY( -BAR_HEIGHT/2+16 + fPercentageFromTop*(BAR_HEIGHT-32) );
}
