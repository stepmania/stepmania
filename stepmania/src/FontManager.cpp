#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: FontManager

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "FontManager.h"
#include "Font.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include "RageTimer.h"


FontManager*	FONT	= NULL;

//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
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


//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
Font* FontManager::LoadFont( CString sFontOrTextureFilePath, CString sChars )
{
	sFontOrTextureFilePath.MakeLower();

//	LOG->Trace( "FontManager::LoadFont(%s).", sFontFilePath.GetString() );


	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "graphics\blah.png" and "..\stepmania\graphics\blah.png" ).

	Font* pFont = NULL;

	std::map<CString, Font*>::iterator p = m_mapPathToFont.find(sFontOrTextureFilePath);
	if(p != m_mapPathToFont.end()) {
//		LOG->Trace( ssprintf("FontManager: The Font '%s' now has %d references.", sFontFilePath.GetString(), pFont->m_iRefCount) );
		pFont=p->second;
		pFont->m_iRefCount++;
	}
	else	// the texture is not already loaded
	{
		CString sDrive, sDir, sFName, sExt;
		splitpath( false, sFontOrTextureFilePath, sDrive, sDir, sFName, sExt );

		if( sChars == "" )
			pFont = (Font*) new Font( sFontOrTextureFilePath );
		else
			pFont = (Font*) new Font( sFontOrTextureFilePath, sChars );

//		LOG->Trace( "FontManager: Loading '%s' from disk.", sFontFilePath.GetString());

		m_mapPathToFont[sFontOrTextureFilePath] = pFont;
	}

	return pFont;
}


bool FontManager::IsFontLoaded( CString sFontFilePath )
{
	sFontFilePath.MakeLower();

	return m_mapPathToFont.find(sFontFilePath) != m_mapPathToFont.end();
}	

void FontManager::UnloadFont( CString sFontFilePath )
{
	sFontFilePath.MakeLower();

//	LOG->Trace( "FontManager::UnloadTexture(%s).", sFontFilePath.GetString() );

	if( sFontFilePath == "" )
	{
//		LOG->Trace( "FontManager::UnloadTexture(): tried to Unload a blank" );
		return;
	}
	
	Font* pFont;
	std::map<CString, Font*>::iterator p = m_mapPathToFont.find(sFontFilePath);
	if(p == m_mapPathToFont.end())
		RageException::Throw( "Tried to Unload a font that wasn't loaded. '%s'", sFontFilePath.GetString() );

	pFont=p->second;
	pFont->m_iRefCount--;
	if( pFont->m_iRefCount != 0 )
	{
//		LOG->Trace( "FontManager: '%s' will not be deleted.  It still has %d references.", sFontFilePath.GetString(), pFont->m_iRefCount );
		return;
	}
	
	// There are no more references to this texture.
//	LOG->Trace( "FontManager: '%s' will be deleted.  It has %d references.", sFontFilePath.GetString(), pFont->m_iRefCount );
	SAFE_DELETE( pFont );		// free the texture
	m_mapPathToFont.erase( p );	// and remove the key in the map
}
