#include "global.h"
#include "FontManager.h"
#include "Font.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include <map>
#include "GameManager.h"

FontManager*	FONT	= NULL;	// global and accessable from anywhere in our program

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

Font *FontManager::CopyFont( Font *pFont )
{
	++pFont->m_iRefCount;
	return pFont;
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

/*
 * (c) 2001-2003 Chris Danford, Glenn Maynard
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
