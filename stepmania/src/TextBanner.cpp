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
#include "song.h"
#include "PrefsManager.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "RageTextureManager.h"


CachedThemeMetricF TITLE_X						("TextBanner","TitleHorizX");
CachedThemeMetricF SUB_TITLE_X					("TextBanner","SubTitleHorizX");
CachedThemeMetricF ARTIST_X						("TextBanner","ArtistHorizX");
CachedThemeMetricF MAX_TITLE_WIDTH				("TextBanner","MaxTitleWidth");
CachedThemeMetricF MAX_SUB_TITLE_WIDTH			("TextBanner","MaxSubTitleWidth");
CachedThemeMetricF MAX_ARTIST_WIDTH				("TextBanner","MaxArtistWidth");
CachedThemeMetricI TITLE_HORIZ_ALIGN			("TextBanner","TitleHorizAlign");
CachedThemeMetricI SUB_TITLE_HORIZ_ALIGN		("TextBanner","SubTitleHorizAlign");
CachedThemeMetricI ARTIST_HORIZ_ALIGN			("TextBanner","ArtistHorizAlign");
CachedThemeMetric  ARTIST_PREPEND_STRING		("TextBanner","ArtistPrependString");
CachedThemeMetricF TWO_LINES_TITLE_ZOOM			("TextBanner","TwoLinesTitleZoom");
CachedThemeMetricF TWO_LINES_ARTIST_ZOOM		("TextBanner","TwoLinesArtistZoom");
CachedThemeMetricF THREE_LINES_TITLE_ZOOM		("TextBanner","ThreeLinesTitleZoom");
CachedThemeMetricF THREE_LINES_SUB_TITLE_ZOOM	("TextBanner","ThreeLinesSubTitleZoom");
CachedThemeMetricF THREE_LINES_ARTIST_ZOOM		("TextBanner","ThreeLinesArtistZoom");
CachedThemeMetricF TWO_LINES_TITLE_Y			("TextBanner","TwoLinesTitleY");
CachedThemeMetricF TWO_LINES_ARTIST_Y			("TextBanner","TwoLinesArtistY");
CachedThemeMetricF THREE_LINES_TITLE_Y			("TextBanner","ThreeLinesTitleY");
CachedThemeMetricF THREE_LINES_SUB_TITLE_Y		("TextBanner","ThreeLinesSubTitleY");
CachedThemeMetricF THREE_LINES_ARTIST_Y			("TextBanner","ThreeLinesArtistY");


TextBanner::TextBanner()
{
	TITLE_X.Refresh();
	SUB_TITLE_X.Refresh();
	ARTIST_X.Refresh();
	MAX_TITLE_WIDTH.Refresh();
	MAX_SUB_TITLE_WIDTH.Refresh();
	MAX_ARTIST_WIDTH.Refresh();
	TITLE_HORIZ_ALIGN.Refresh();
	SUB_TITLE_HORIZ_ALIGN.Refresh();
	ARTIST_HORIZ_ALIGN.Refresh();
	ARTIST_PREPEND_STRING.Refresh();
	TWO_LINES_TITLE_ZOOM.Refresh();
	TWO_LINES_ARTIST_ZOOM.Refresh();
	THREE_LINES_TITLE_ZOOM.Refresh();
	THREE_LINES_SUB_TITLE_ZOOM.Refresh();
	THREE_LINES_ARTIST_ZOOM.Refresh();
	TWO_LINES_TITLE_Y.Refresh();
	TWO_LINES_ARTIST_Y.Refresh();
	THREE_LINES_TITLE_Y.Refresh();
	THREE_LINES_SUB_TITLE_Y.Refresh();
	THREE_LINES_ARTIST_Y.Refresh();
	
	m_textTitle.LoadFromFont( THEME->GetPathToF("TextBanner") );
	m_textSubTitle.LoadFromFont( THEME->GetPathToF("TextBanner") );
	m_textArtist.LoadFromFont( THEME->GetPathToF("TextBanner") );

	m_textTitle.SetX( TITLE_X );
	m_textSubTitle.SetX( SUB_TITLE_X );
	m_textArtist.SetX( ARTIST_X );

	m_textTitle.SetHorizAlign( (Actor::HorizAlign)(int)TITLE_HORIZ_ALIGN );
	m_textSubTitle.SetHorizAlign( (Actor::HorizAlign)(int)SUB_TITLE_HORIZ_ALIGN );
	m_textArtist.SetHorizAlign( (Actor::HorizAlign)(int)ARTIST_HORIZ_ALIGN );

	m_textTitle.EnableShadow( false );
	m_textSubTitle.EnableShadow( false );
	m_textArtist.EnableShadow( false );

	this->AddChild( &m_textTitle );
	this->AddChild( &m_textSubTitle );
	this->AddChild( &m_textArtist );
}


void TextBanner::LoadFromString( 
	CString sDisplayTitle, CString sTranslitTitle, 
	CString sDisplaySubTitle, CString sTranslitSubTitle, 
	CString sDisplayArtist, CString sTranslitArtist )
{
	bool bTwoLines = sDisplaySubTitle.size() == 0;

	const float fTitleZoom		= bTwoLines ? TWO_LINES_TITLE_ZOOM	: THREE_LINES_TITLE_ZOOM;
	const float fSubTitleZoom	= bTwoLines ? 0.0f					: THREE_LINES_SUB_TITLE_ZOOM;
	const float fArtistZoom		= bTwoLines ? TWO_LINES_ARTIST_ZOOM : THREE_LINES_ARTIST_ZOOM;
	
	m_textTitle.SetZoom( fTitleZoom );
	m_textSubTitle.SetZoom( fSubTitleZoom );
	m_textArtist.SetZoom( fArtistZoom );

	m_textTitle.SetTextMaxWidth( MAX_TITLE_WIDTH, sDisplayTitle, sTranslitTitle );
	m_textSubTitle.SetTextMaxWidth( MAX_SUB_TITLE_WIDTH, sDisplaySubTitle, sTranslitSubTitle );
	m_textArtist.SetTextMaxWidth( MAX_ARTIST_WIDTH, sDisplayArtist, sTranslitArtist );

	const float fTitleY		= bTwoLines ? TWO_LINES_TITLE_Y		: THREE_LINES_TITLE_Y;
	const float fSubTitleY	= bTwoLines ? 0.0f					: THREE_LINES_SUB_TITLE_Y;
	const float fArtistY	= bTwoLines ? TWO_LINES_ARTIST_Y	: THREE_LINES_ARTIST_Y;

	m_textTitle.SetY( fTitleY );
	m_textSubTitle.SetY( fSubTitleY );
	m_textArtist.SetY( fArtistY );
}

void TextBanner::LoadFromSong( const Song* pSong )
{
	CString sDisplayTitle = pSong ? pSong->GetDisplayMainTitle() : "";
	CString sTranslitTitle = pSong ? pSong->GetTranslitMainTitle() : "";
	CString sDisplaySubTitle = pSong ? pSong->GetDisplaySubTitle() : "";
	CString sTranslitSubTitle = pSong ? pSong->GetTranslitSubTitle() : "";
	CString sDisplayArtist = pSong ? (CString)ARTIST_PREPEND_STRING + pSong->GetDisplayArtist() : "";
	CString sTranslitArtist = pSong ? (CString)ARTIST_PREPEND_STRING + pSong->GetTranslitArtist() : "";

	LoadFromString( 
		sDisplayTitle, sTranslitTitle, 
		sDisplaySubTitle, sTranslitSubTitle, 
		sDisplayArtist, sTranslitArtist );
}

