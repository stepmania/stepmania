#include "global.h"
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
#include <map>

FontManager*	FONT	= NULL;

// map from file name to a texture holder
typedef pair<CString,CString> FontName;
static map<FontName, Font*> g_mapPathToFont;

FontManager::FontManager()
{
}

FontManager::~FontManager()
{
	for( std::map<FontName, Font*>::iterator i = g_mapPathToFont.begin();
		i != g_mapPathToFont.end(); ++i)
	{
		const FontName &fn = i->first;
		Font* pFont = i->second;
		LOG->Trace( "FONT LEAK: '%s', RefCount = %d.", fn.first.c_str(), pFont->m_iRefCount );
		delete pFont;
	}
}

/* A longchar is at least 32 bits.  If c & 0xFF000000, it's a game-custom
 * character; game 0 is 0x0100nnnn, game 1 is 0x0200nnnn, and so on. */
longchar FontManager::MakeGameGlyph(wchar_t c, Game g)
{
	ASSERT(c >= 0 && c <= 0xFFFF);
	ASSERT(g >= 0 && g <= 0xFF);
	return longchar (((g+1) << 24) + c);
}

bool FontManager::ExtractGameGlyph(longchar ch, wchar_t &c, Game &g)
{
	if((ch & 0xFF000000) == 0) return false; /* not a game glyph */
	
	g = Game((ch >> 24) - 1);
	c = wchar_t(ch & 0xFFFF);

	return true;
}

void FontManager::ReloadFonts()
{
	for(map<FontName, Font*>::iterator i = g_mapPathToFont.begin();
		i != g_mapPathToFont.end(); ++i)
	{
		i->second->Reload();
	}
}

Font* FontManager::LoadFont( const CString &sFontOrTextureFilePath, CString sChars )
{
	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have two copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "graphics\blah.png" and "..\stepmania\graphics\blah.png" ).

	CHECKPOINT_M( ssprintf("FontManager::LoadFont(%s).", sFontOrTextureFilePath.c_str()) );
	const FontName NewName( sFontOrTextureFilePath, sChars );
	map<FontName, Font*>::iterator p = g_mapPathToFont.find( NewName );
	if( p != g_mapPathToFont.end() )
	{
		Font *pFont=p->second;
		pFont->m_iRefCount++;
		return pFont;
	}

	Font *f = new Font;
	f->Load(sFontOrTextureFilePath, sChars);
	g_mapPathToFont[NewName] = f;
	return f;
}


void FontManager::UnloadFont( Font *fp )
{
	CHECKPOINT_M( ssprintf("FontManager::UnloadFont(%s).", fp->path.c_str()) );

	for( std::map<FontName, Font*>::iterator i = g_mapPathToFont.begin();
		i != g_mapPathToFont.end(); ++i)
	{
		if(i->second != fp)
			continue;

		i->second->m_iRefCount--;

		if( fp->m_iRefCount == 0 )
		{
			delete i->second;		// free the texture
			g_mapPathToFont.erase( i );	// and remove the key in the map
		}
		return;
	}
	
	RageException::Throw( "Unloaded an unknown font (%p)", fp );
}
