#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: FontManager

 Desc: Interface for loading and releasing fonts.

 Copyright (c) 2001-2003 by the person(s) listed below.  All rights reserved.
	Chris Danford
	Glenn Maynard
-----------------------------------------------------------------------------
*/


#include "FontManager.h"
#include "Font.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTimer.h"

FontManager::aliasmap FontManager::CharAliases;
map<CString,CString> FontManager::CharAliasRepl;

FontManager*	FONT	= NULL;

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
	for( std::map<CString, Font*>::iterator i = m_mapPathToFont.begin();
		i != m_mapPathToFont.end(); ++i)
	{
		Font* pFont = i->second;
		LOG->Trace( "FONT LEAK: '%s', RefCount = %d.", i->first.GetString(), pFont->m_iRefCount );
		delete pFont;
	}
}

/* A longchar is at least 32 bits.  If c & 0xFF000000, it's a game-custom
 * character; game 0 is 0x01nnnnnn, game 1 is 0x02nnnnnn, and so on. */
longchar FontManager::MakeGameGlyph(longchar c, Game g)
{
	ASSERT(g >= 0 && g <= 0xFF && c >= 0 && c <= 0xFFFF);
	return longchar (((g+1) << 24) + c);
}

bool FontManager::ExtractGameGlyph(longchar ch, int &c, Game &g)
{
	if((ch & 0xFF000000) == 0) return false; /* not a game glyph */
	
	g = Game((ch >> 24) - 1);
	c = ch & 0xFFFFFF;

	return true;
}

void FontManager::ReloadFonts()
{
	for(map<CString, Font*>::iterator i = m_mapPathToFont.begin();
		i != m_mapPathToFont.end(); ++i)
	{
		i->second->Reload();
	}
}

Font* FontManager::LoadFont( const CString &sFontOrTextureFilePath, CString sChars )
{
	InitCharAliases();

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have two copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "graphics\blah.png" and "..\stepmania\graphics\blah.png" ).

	map<CString, Font*>::iterator p = m_mapPathToFont.find(sFontOrTextureFilePath);
	if(p != m_mapPathToFont.end()) {
		Font *pFont=p->second;
//		LOG->Trace( ssprintf("FontManager: The Font '%s' now has %d references.", sFontOrTextureFilePath.GetString(), pFont->m_iRefCount) );
		pFont->m_iRefCount++;
		return pFont;
	}

	/* XXX: This will mismatch cached fonts if we load the same font with two
	 * different sChars. */
	Font *f = new Font;
	f->Load(sFontOrTextureFilePath, sChars);
	m_mapPathToFont[sFontOrTextureFilePath] = f;
	return f;
}


void FontManager::UnloadFont( Font *fp )
{
	LOG->Trace( "FontManager::UnloadTexture(%s).", fp->path.GetString() );

	for( std::map<CString, Font*>::iterator i = m_mapPathToFont.begin();
		i != m_mapPathToFont.end(); ++i)
	{
		if(i->second == fp)
		{
			i->second->m_iRefCount--;
	
			if( fp->m_iRefCount == 0 )
			{
				delete i->second;		// free the texture
				m_mapPathToFont.erase( i );	// and remove the key in the map
			}
//	LOG->Trace( "FontManager: '%s' will be deleted.  It has %d references.", sFontFilePath.GetString(), pFont->m_iRefCount );
			return;
		}

	}
	
	RageException::Throw( "Unloaded an unknown font (%p)", fp );
}


void FontManager::InitCharAliases()
{
	if(!CharAliases.empty())
		return;
	/* XXX; these should be in a data file somewhere (theme metrics?) 
	 * (The comments here are UTF-8; they won't show up in VC6.) */
	CharAliases["default"]		= Font::DEFAULT_GLYPH; /* ? */
	CharAliases["kakumei1"]		= 0x9769; /* 革 */
	CharAliases["kakumei2"]		= 0x547D; /* 命 */
	CharAliases["matsuri"]		= 0x796D; /* 祭 */
	CharAliases["oni"]			= 0x9B3C; /* 鬼 */
	CharAliases["michi"]		= 0x9053; /* 道 */
	CharAliases["futatsu"]		= 0x5F10; /* 弐 */
	CharAliases["omega"]		= 0x03a9; /* Ω */
	CharAliases["whiteheart"]	= 0x2661; /* ♡ */
	CharAliases["blackstar"]	= 0x2605; /* ★ */
	CharAliases["whitestar"]	= 0x2606; /* ☆ */

	/* These are internal-use glyphs; they don't have real Unicode codepoints. */
	CharAliases["up"]			= 0xE000;
	CharAliases["down"]			= 0xE001;
	CharAliases["left"]			= 0xE002;
	CharAliases["right"]		= 0xE003;
	CharAliases["menuup"]		= 0xE004;
	CharAliases["menudown"]		= 0xE005;
	CharAliases["menuleft"]		= 0xE006;
	CharAliases["menuright"]	= 0xE007;
	CharAliases["start"]		= 0xE008;

	CharAliases["invalid"]		= INVALID_CHAR; /* 0xFFFF */

	for(aliasmap::const_iterator i = CharAliases.begin(); i != CharAliases.end(); ++i)
	{
		CString from = ssprintf("&%s;", i->first.GetString());
		CString to = LcharToUTF8(i->second);
		from.MakeUpper();
		CharAliasRepl[from] = to;
		LOG->Trace("from '%s' to '%s'", from.GetString(), to.GetString());
	}
}

/* Replace all &markers; and &#NNNN;s with UTF-8.  I'm not really
 * sure where to put this; this is used elsewhere, too. */
void FontManager::ReplaceMarkers( CString &sText )
{
	InitCharAliases();
	ReplaceText(sText, FontManager::CharAliasRepl);
	Replace_Unicode_Markers(sText);
}

