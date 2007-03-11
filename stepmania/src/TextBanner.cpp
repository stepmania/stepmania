#include "global.h"
#include "TextBanner.h"
#include "RageUtil.h"
#include "song.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "RageTextureManager.h"
#include "ActorUtil.h"
#include "ThemeMetric.h"


void TextBanner::Load( RString sType )
{
	m_bInitted = true;

	m_textTitle.SetName( "Title" );
	m_textTitle.LoadFromFont( THEME->GetPathF(sType,"text") );
	this->AddChild( &m_textTitle );

	m_textSubTitle.SetName( "Subtitle" );
	m_textSubTitle.LoadFromFont( THEME->GetPathF(sType,"text") );
	this->AddChild( &m_textSubTitle );

	m_textArtist.SetName( "Artist" );
	m_textArtist.LoadFromFont( THEME->GetPathF(sType,"text") );
	this->AddChild( &m_textArtist );

	AddCommand( "Set", THEME->GetMetricA(sType,"SetCommand") );
	ARTIST_PREPEND_STRING			.Load(sType,"ArtistPrependString");

	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textTitle, sType );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textSubTitle, sType );
	ActorUtil::LoadAllCommandsAndSetXYAndOnCommand( m_textArtist, sType );
}

TextBanner::TextBanner()
{
	m_bInitted = false;
}

TextBanner::TextBanner( const TextBanner &cpy ):
	ActorFrame( cpy ),
	m_bInitted( cpy.m_bInitted ),
	m_textTitle( cpy.m_textTitle ),
	m_textSubTitle( cpy.m_textSubTitle ),
	m_textArtist( cpy.m_textArtist ),
	ARTIST_PREPEND_STRING( cpy.ARTIST_PREPEND_STRING )
{
	this->AddChild( &m_textTitle );
	this->AddChild( &m_textSubTitle );
	this->AddChild( &m_textArtist );
}

void TextBanner::LoadFromString( 
	const RString &sDisplayTitle, const RString &sTranslitTitle, 
	const RString &sDisplaySubTitle, const RString &sTranslitSubTitle, 
	const RString &sDisplayArtist, const RString &sTranslitArtist )
{
	LoadInternal( 
		NULL,
		sDisplayTitle, sTranslitTitle, 
		sDisplaySubTitle, sTranslitSubTitle, 
		sDisplayArtist, sTranslitArtist );
}

void TextBanner::LoadInternal( 
	Song *pSong,
	const RString &sDisplayTitle, const RString &sTranslitTitle, 
	const RString &sDisplaySubTitle, const RString &sTranslitSubTitle, 
	const RString &sDisplayArtist, const RString &sTranslitArtist )
{
	ASSERT( m_bInitted );

	m_textTitle.SetText( sDisplayTitle, sTranslitTitle );
	m_textSubTitle.SetText( sDisplaySubTitle, sTranslitSubTitle );
	m_textArtist.SetText( sDisplayArtist, sTranslitArtist );

	Message msg( "Set" );
	msg.SetParam( "Song", pSong );
	this->HandleMessage( msg );
}

void TextBanner::LoadFromSong( Song* pSong )
{
	ASSERT( m_bInitted );

	RString sDisplayTitle		= pSong ? pSong->GetDisplayMainTitle() : RString("");
	RString sTranslitTitle		= pSong ? pSong->GetTranslitMainTitle() : RString("");
	RString sDisplaySubTitle	= pSong ? pSong->GetDisplaySubTitle() : RString("");
	RString sTranslitSubTitle	= pSong ? pSong->GetTranslitSubTitle() : RString("");
	RString sDisplayArtist		= pSong ? (RString)ARTIST_PREPEND_STRING + pSong->GetDisplayArtist() : RString("");
	RString sTranslitArtist		= pSong ? (RString)ARTIST_PREPEND_STRING + pSong->GetTranslitArtist() : RString("");

	LoadInternal(
		pSong,
		sDisplayTitle, sTranslitTitle, 
		sDisplaySubTitle, sTranslitSubTitle, 
		sDisplayArtist, sTranslitArtist );
}

/*
 * (c) 2001-2004 Chris Danford
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
