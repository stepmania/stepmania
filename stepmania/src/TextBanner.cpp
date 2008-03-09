#include "global.h"
#include "TextBanner.h"
#include "Song.h"
#include "ActorUtil.h"
#include "ThemeManager.h"
#include "XmlFile.h"

REGISTER_ACTOR_CLASS( TextBanner )

void TextBanner::LoadFromNode( const XNode* pNode )
{
	m_bInitted = true;

	{
		const XNode *pTitleNode = pNode->GetChild( "Title" );
		if( pTitleNode == NULL )
			RageException::Throw( "%s: TextBanner: missing the Title child", ActorUtil::GetWhere(pNode).c_str() );
		m_textTitle.LoadFromNode( pTitleNode );
		this->AddChild( &m_textTitle );
	}

	{
		const XNode *pSubtitleNode = pNode->GetChild( "Subtitle" );
		if( pSubtitleNode == NULL )
			RageException::Throw( "%s: TextBanner: missing the Subtitle child", ActorUtil::GetWhere(pNode).c_str() );
		m_textSubTitle.LoadFromNode( pSubtitleNode );
		this->AddChild( &m_textSubTitle );
	}

	{
		const XNode *pArtistNode = pNode->GetChild( "Artist" );
		if( pArtistNode == NULL )
			RageException::Throw( "%s: TextBanner: missing the Artist child", ActorUtil::GetWhere(pNode).c_str() );
		m_textArtist.LoadFromNode( pArtistNode );
		this->AddChild( &m_textArtist );
	}

	pNode->GetAttrValue( "ArtistPrependString", m_sArtistPrependString );

	ActorFrame::LoadFromNode( pNode );
}

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

	AddCommand( "AfterSet", THEME->GetMetricA(sType,"AfterSetCommand") );
	m_sArtistPrependString = THEME->GetMetric(sType,"ArtistPrependString");

	ActorUtil::LoadAllCommandsAndOnCommand( m_textTitle, sType );
	ActorUtil::LoadAllCommandsAndOnCommand( m_textSubTitle, sType );
	ActorUtil::LoadAllCommandsAndOnCommand( m_textArtist, sType );
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
	m_sArtistPrependString( cpy.m_sArtistPrependString )
{
	this->AddChild( &m_textTitle );
	this->AddChild( &m_textSubTitle );
	this->AddChild( &m_textArtist );
}

void TextBanner::SetFromString( 
	const RString &sDisplayTitle, const RString &sTranslitTitle, 
	const RString &sDisplaySubTitle, const RString &sTranslitSubTitle, 
	const RString &sDisplayArtist, const RString &sTranslitArtist )
{
	ASSERT( m_bInitted );

	m_textTitle.SetText( sDisplayTitle, sTranslitTitle );
	m_textSubTitle.SetText( sDisplaySubTitle, sTranslitSubTitle );
	m_textArtist.SetText( sDisplayArtist, sTranslitArtist );

	Message msg("AfterSet");
	this->PlayCommandNoRecurse( msg );
}

void TextBanner::SetFromSong( const Song *pSong )
{
	if( pSong == NULL )
	{
		SetFromString( "", "", "", "", "", "" );
		return;
	}
	SetFromString( pSong->GetDisplayMainTitle(),				pSong->GetTranslitMainTitle(),
			pSong->GetDisplaySubTitle(),				pSong->GetTranslitSubTitle(),
			m_sArtistPrependString + pSong->GetDisplayArtist(),	m_sArtistPrependString + pSong->GetTranslitArtist() );
}

// lua start
#include "LuaBinding.h"

class LunaTextBanner: public Luna<TextBanner>
{
public:
	static int SetFromSong( T* p, lua_State *L )
	{
		Song *pSong = Luna<Song>::check(L,1);
  		p->SetFromSong( pSong );
		return 0;
	}

	static int SetFromString( T* p, lua_State *L )
	{
		RString sDisplayTitle = SArg(1);
		RString sTranslitTitle = SArg(2);
		RString sDisplaySubTitle = SArg(3);
		RString sTranslitSubTitle = SArg(4);
		RString sDisplayArtist = SArg(5);
		RString sTranslitArtist = SArg(6);
  		p->SetFromString( sDisplayTitle, sTranslitTitle, sDisplaySubTitle, sTranslitSubTitle, sDisplayArtist, sTranslitArtist );
		return 0;
	}

	LunaTextBanner()
	{
		ADD_METHOD( SetFromSong );
		ADD_METHOD( SetFromString );
	}
};

LUA_REGISTER_DERIVED_CLASS( TextBanner, ActorFrame )
// lua end

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
