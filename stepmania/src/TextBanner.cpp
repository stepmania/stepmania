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
#include "ActorUtil.h"


CachedThemeMetric  ARTIST_PREPEND_STRING		("TextBanner","ArtistPrependString");
CachedThemeMetric TWO_LINES_TITLE_COMMAND		("TextBanner","TwoLinesTitleCommand");
CachedThemeMetric TWO_LINES_SUBTITLE_COMMAND	("TextBanner","TwoLinesSubtitleCommand");
CachedThemeMetric TWO_LINES_ARTIST_COMMAND		("TextBanner","TwoLinesArtistCommand");
CachedThemeMetric THREE_LINES_TITLE_COMMAND		("TextBanner","ThreeLinesTitleCommand");
CachedThemeMetric THREE_LINES_SUBTITLE_COMMAND	("TextBanner","ThreeLinesSubtitleCommand");
CachedThemeMetric THREE_LINES_ARTIST_COMMAND	("TextBanner","ThreeLinesArtistCommand");

TextBanner::TextBanner()
{
	ARTIST_PREPEND_STRING.Refresh();
	TWO_LINES_TITLE_COMMAND.Refresh();
	TWO_LINES_SUBTITLE_COMMAND.Refresh();
	TWO_LINES_ARTIST_COMMAND.Refresh();
	THREE_LINES_TITLE_COMMAND.Refresh();
	THREE_LINES_SUBTITLE_COMMAND.Refresh();
	THREE_LINES_ARTIST_COMMAND.Refresh();

	m_textTitle.SetName( "Title" );
	m_textTitle.LoadFromFont( THEME->GetPathToF("TextBanner") );
	UtilSetXYAndOnCommand( m_textTitle, "TextBanner" );
	this->AddChild( &m_textTitle );

	m_textSubTitle.SetName( "Subtitle" );
	m_textSubTitle.LoadFromFont( THEME->GetPathToF("TextBanner") );
	UtilSetXYAndOnCommand( m_textSubTitle, "TextBanner" );
	this->AddChild( &m_textSubTitle );

	m_textArtist.SetName( "Artist" );
	m_textArtist.LoadFromFont( THEME->GetPathToF("TextBanner") );
	UtilSetXYAndOnCommand( m_textArtist, "TextBanner" );
	this->AddChild( &m_textArtist );
}


void TextBanner::LoadFromString( 
	CString sDisplayTitle, CString sTranslitTitle, 
	CString sDisplaySubTitle, CString sTranslitSubTitle, 
	CString sDisplayArtist, CString sTranslitArtist )
{
	bool bTwoLines = sDisplaySubTitle.size() == 0;

	if( bTwoLines )
	{
		m_textTitle.Command( TWO_LINES_TITLE_COMMAND );
		m_textSubTitle.Command( TWO_LINES_SUBTITLE_COMMAND );
		m_textArtist.Command( TWO_LINES_ARTIST_COMMAND );
	}
	else
	{
		m_textTitle.Command( THREE_LINES_TITLE_COMMAND );
		m_textSubTitle.Command( THREE_LINES_SUBTITLE_COMMAND );
		m_textArtist.Command( THREE_LINES_ARTIST_COMMAND );
	}

	m_textTitle.SetText( sDisplayTitle, sTranslitTitle );
	m_textSubTitle.SetText( sDisplaySubTitle, sTranslitSubTitle );
	m_textArtist.SetText( sDisplayArtist, sTranslitArtist );
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

