#include "global.h"
/*
-----------------------------------------------------------------------------
 Class: TextBanner

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "TextBanner.h"
#include "RageUtil.h"
#include "Song.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "SongManager.h"


#define WIDTH						THEME->GetMetricF("TextBanner","Width")
#define HEIGHT						THEME->GetMetricF("TextBanner","Height")
#define HORIZ_ALIGN					THEME->GetMetricI("TextBanner","HorizAlign")
#define ARTIST_PREPEND_STRING		THEME->GetMetric( "TextBanner","ArtistPrependString")
#define TWO_LINES_TITLE_ZOOM		THEME->GetMetricF("TextBanner","TwoLinesTitleZoom")
#define TWO_LINES_ARTIST_ZOOM		THEME->GetMetricF("TextBanner","TwoLinesArtistZoom")
#define THREE_LINES_TITLE_ZOOM		THEME->GetMetricF("TextBanner","ThreeLinesTitleZoom")
#define THREE_LINES_SUB_TITLE_ZOOM	THEME->GetMetricF("TextBanner","ThreeLinesSubTitleZoom")
#define THREE_LINES_ARTIST_ZOOM		THEME->GetMetricF("TextBanner","ThreeLinesArtistZoom")
#define TWO_LINES_TITLE_Y			THEME->GetMetricF("TextBanner","TwoLinesTitleY")
#define TWO_LINES_ARTIST_Y			THEME->GetMetricF("TextBanner","TwoLinesArtistY")
#define THREE_LINES_TITLE_Y			THEME->GetMetricF("TextBanner","ThreeLinesTitleY")
#define THREE_LINES_SUB_TITLE_Y		THEME->GetMetricF("TextBanner","ThreeLinesSubTitleY")
#define THREE_LINES_ARTIST_Y		THEME->GetMetricF("TextBanner","ThreeLinesArtistY")

// metrics cache
float g_fWidth;
float g_fHeight;
int g_iHorizAlign;
CString g_sArtistPrependString;
float g_fTwoLinesTitleZoom;
float g_fTwoLinesArtistZoom;
float g_fThreeLinesTitleZoom;
float g_fThreeLinesSubTitleZoom;
float g_fThreeLinesArtistZoom;
float g_fTwoLinesTitleY;
float g_fTwoLinesArtistY;
float g_fThreeLinesTitleY;
float g_fThreeLinesSubTitleY;
float g_fThreeLinesArtistY;


TextBanner::TextBanner()
{
	g_fWidth = WIDTH;
	g_fHeight = HEIGHT;
	g_iHorizAlign = HORIZ_ALIGN;
	g_sArtistPrependString = ARTIST_PREPEND_STRING;
	g_fTwoLinesTitleZoom = TWO_LINES_TITLE_ZOOM;
	g_fTwoLinesArtistZoom = TWO_LINES_ARTIST_ZOOM;
	g_fThreeLinesTitleZoom = THREE_LINES_TITLE_ZOOM;
	g_fThreeLinesSubTitleZoom = THREE_LINES_SUB_TITLE_ZOOM;
	g_fThreeLinesArtistZoom = THREE_LINES_ARTIST_ZOOM;
	g_fTwoLinesTitleY = TWO_LINES_TITLE_Y;
	g_fTwoLinesArtistY = TWO_LINES_ARTIST_Y;
	g_fThreeLinesTitleY = THREE_LINES_TITLE_Y;
	g_fThreeLinesSubTitleY = THREE_LINES_SUB_TITLE_Y;
	g_fThreeLinesArtistY = THREE_LINES_ARTIST_Y;
	
	m_textTitle.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel text banner") );
	m_textSubTitle.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel text banner") );
	m_textArtist.LoadFromFont( THEME->GetPathTo("Fonts","musicwheel text banner") );

	m_textTitle.SetX( -g_fWidth/2 );
	m_textSubTitle.SetX( -g_fWidth/2 );
	m_textArtist.SetX( -g_fWidth/2 );

	m_textTitle.SetHorizAlign( (Actor::HorizAlign)g_iHorizAlign );
	m_textSubTitle.SetHorizAlign( (Actor::HorizAlign)g_iHorizAlign );
	m_textArtist.SetHorizAlign( (Actor::HorizAlign)g_iHorizAlign );

	m_textTitle.TurnShadowOff();
	m_textSubTitle.TurnShadowOff();
	m_textArtist.TurnShadowOff();

	this->AddChild( &m_textTitle );
	this->AddChild( &m_textSubTitle );
	this->AddChild( &m_textArtist );
}


bool TextBanner::LoadFromSong( const Song* pSong )
{
	if( pSong == NULL )
	{
		m_textTitle.SetText( "" );
		m_textSubTitle.SetText( "" );
		m_textArtist.SetText( "" );
		return true;
	}

	m_textTitle.SetText( pSong->GetDisplayMainTitle(), pSong->GetTranslitMainTitle() );
	m_textSubTitle.SetText( pSong->GetDisplaySubTitle(), pSong->GetTranslitSubTitle() );
	m_textArtist.SetText( g_sArtistPrependString + pSong->GetDisplayArtist(),
						  g_sArtistPrependString + pSong->GetTranslitArtist() );

	bool bTwoLines = m_textSubTitle.GetText().size() == 0;

	float fTitleZoom	= bTwoLines ? g_fTwoLinesTitleZoom	: g_fThreeLinesTitleZoom;
	float fSubTitleZoom = bTwoLines ? 0						: g_fThreeLinesSubTitleZoom;
	float fArtistZoom	= bTwoLines ? g_fTwoLinesArtistZoom : g_fThreeLinesArtistZoom;
	
	m_textTitle.SetZoomY( fTitleZoom );
	m_textSubTitle.SetZoomY( fSubTitleZoom );
	m_textArtist.SetZoomY( fArtistZoom );

	fTitleZoom		= min( fTitleZoom,		g_fWidth/m_textTitle.GetWidestLineWidthInSourcePixels() );
	fSubTitleZoom	= min( fSubTitleZoom,	g_fWidth/m_textSubTitle.GetWidestLineWidthInSourcePixels() );
	fArtistZoom		= min( fArtistZoom,		g_fWidth/m_textArtist.GetWidestLineWidthInSourcePixels() );

	m_textTitle.SetZoomX( fTitleZoom );
	m_textSubTitle.SetZoomX( fSubTitleZoom );
	m_textArtist.SetZoomX( fArtistZoom );

	float fTitleY		= bTwoLines ? g_fTwoLinesTitleY		: g_fThreeLinesTitleY;
	float fSubTitleY	= bTwoLines ? 0						: g_fThreeLinesSubTitleY;
	float fArtistY		= bTwoLines ? g_fTwoLinesArtistY	: g_fThreeLinesArtistY;

	m_textTitle.SetY( fTitleY );
	m_textSubTitle.SetY( fSubTitleY );
	m_textArtist.SetY( fArtistY );


	return true;
}
