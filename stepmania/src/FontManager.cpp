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
#include "RageTimer.h"

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
		LOG->Trace( "FONT LEAK: '%s', RefCount = %d.", i->first.c_str(), pFont->m_iRefCount );
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
	for(map<CString, Font*>::iterator i = m_mapPathToFont.begin();
		i != m_mapPathToFont.end(); ++i)
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

	map<CString, Font*>::iterator p = m_mapPathToFont.find(sFontOrTextureFilePath);
	if(p != m_mapPathToFont.end()) {
		Font *pFont=p->second;
//		LOG->Trace( ssprintf("FontManager: The Font '%s' now has %d references.", sFontOrTextureFilePath.c_str(), pFont->m_iRefCount) );
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
	Checkpoint( ssprintf("FontManager::UnloadFont(%s).", fp->path.c_str()) );

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
//	LOG->Trace( "FontManager: '%s' will be deleted.  It has %d references.", sFontFilePath.c_str(), pFont->m_iRefCount );
			return;
		}

	}
	
	RageException::Throw( "Unloaded an unknown font (%p)", fp );
}
