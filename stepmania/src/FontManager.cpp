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
		SAFE_DELETE( pFont );
	}
}


Font* FontManager::LoadFont( CString sFontOrTextureFilePath, CString sChars )
{
	sFontOrTextureFilePath.MakeLower();

//	LOG->Trace( "FontManager::LoadFont(%s).", sFontFilePath.GetString() );


	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "graphics\blah.png" and "..\stepmania\graphics\blah.png" ).

	map<CString, Font*>::iterator p = m_mapPathToFont.find(sFontOrTextureFilePath);
	if(p != m_mapPathToFont.end()) {
//		LOG->Trace( ssprintf("FontManager: The Font '%s' now has %d references.", sFontFilePath.GetString(), pFont->m_iRefCount) );
		Font *pFont=p->second;
		pFont->m_iRefCount++;
		return pFont;
	}
	
	// the texture is not already loaded
	CString sDrive, sDir, sFName, sExt;
	splitpath( false, sFontOrTextureFilePath, sDrive, sDir, sFName, sExt );

	Font* pFont = new Font;


	FontPage *fp = new FontPage;
	IniFile ini;

	int expect;
	if( sChars == "" )
	{
		/* Default to 0..255. */
		for( int i=0; i<256; i++ )
			fp->m_iCharToGlyphNo[i] = i;

		// Find .ini widths path from texture path
		CString sDir, sFileName, sExtension;
		splitrelpath( sFontOrTextureFilePath, sDir, sFileName, sExtension );
		const CString sIniPath = sDir + sFileName + ".ini";

		ini.SetPath( sIniPath );
		ini.ReadFile();
		ini.RenameKey("Char Widths", "main");
		expect = 256;
	}
	else
	{
		/* Map characters to frames; we don't actually have an INI. */
		for( int i=0; i<sChars.GetLength(); i++ )
		{
			char c = sChars[i];
			fp->m_iCharToGlyphNo[c] = i;
		}
		expect = sChars.GetLength();
	}

	fp->Load(sFontOrTextureFilePath, ini);
	if( fp->m_pTexture->GetNumFrames() != expect )
		RageException::Throw( "The font '%s' has %d frames; expected %i frames.",
			fp->m_pTexture->GetNumFrames(), expect );

//		LOG->Trace( "FontManager: Loading '%s' from disk.", sFontFilePath.GetString());

	pFont->AddPage(fp);
	m_mapPathToFont[sFontOrTextureFilePath] = pFont;

	pFont->LoadINI(ini);

	return pFont;
}


void FontManager::UnloadFont( Font *fp )
{
//	LOG->Trace( "FontManager::UnloadTexture(%s).", sFontFilePath.GetString() );

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

CString FontManager::GetPageNameFromFileName(const CString &fn)
{
	unsigned begin = fn.find_first_of('[');
	if(begin == fn.npos) return "main";
	unsigned end = fn.find_first_of(']', begin);
	if(end == fn.npos) return "main";
	begin++; end--;
	if(end == begin) return "main";
	return fn.substr(begin+1, end-begin+1);
}
