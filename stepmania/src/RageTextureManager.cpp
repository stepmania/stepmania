#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: RageTextureManager

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageTextureManager.h"
#include "RageBitmapTexture.h"
#include "RageMovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"

RageTextureManager*		TEXTUREMAN		= NULL;

//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
RageTextureManager::RageTextureManager( RageDisplay* pScreen )
{
	assert( pScreen != NULL );
	m_pScreen = pScreen;
	m_iMaxTextureSize = 2048;	// infinite size
	m_iTextureColorDepth = 16;
}

RageTextureManager::~RageTextureManager()
{
	for( std::map<CString, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		LOG->Trace( "TEXTUREMAN LEAK: '%s', RefCount = %d.", i->first, pTexture->m_iRefCount );
		SAFE_DELETE( pTexture );
	}
}


//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
RageTexture* RageTextureManager::LoadTexture( CString sTexturePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	sTexturePath.MakeLower();

//	LOG->Trace( "RageTextureManager::LoadTexture(%s).", sTexturePath );

	// holder for the new texture
	RageTexture* pTexture;

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp" ).

	std::map<CString, RageTexture*>::iterator p = m_mapPathToTexture.find(sTexturePath);
	if(p != m_mapPathToTexture.end()) {
		pTexture = p->second;

		pTexture->m_iRefCount++;
		if( bForceReload )
			pTexture->Reload( m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );

//		LOG->Trace( "RageTextureManager: '%s' now has %d references.", sTexturePath, pTexture->m_iRefCount );
	}
	else	// the texture is not already loaded
	{
		CString sDrive, sDir, sFName, sExt;
		splitpath( false, sTexturePath, sDrive, sDir, sFName, sExt );

		if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
			pTexture = (RageTexture*) new RageMovieTexture( m_pScreen, sTexturePath, m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
		else
			pTexture = (RageTexture*) new RageBitmapTexture( m_pScreen, sTexturePath, m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );


		LOG->Trace( "RageTextureManager: Finished loading '%s'.", sTexturePath );


		m_mapPathToTexture[sTexturePath] = pTexture;
	}

//	LOG->Trace( "Display: %.2f KB video memory left",	DISPLAY->GetDevice()->GetAvailableTextureMem()/1000000.0f );

	return pTexture;
}


bool RageTextureManager::IsTextureLoaded( CString sTexturePath )
{
	sTexturePath.MakeLower();

	return m_mapPathToTexture.find(sTexturePath) != m_mapPathToTexture.end();
}	

void RageTextureManager::UnloadTexture( CString sTexturePath )
{
	sTexturePath.MakeLower();

//	LOG->Trace( "RageTextureManager::UnloadTexture(%s).", sTexturePath );

	if( sTexturePath == "" )
	{
		//LOG->Trace( "RageTextureManager::UnloadTexture(): tried to Unload a blank texture." );
		return;
	}
	
	RageTexture* pTexture;

	std::map<CString, RageTexture*>::iterator p = m_mapPathToTexture.find(sTexturePath);
	if(p == m_mapPathToTexture.end())
		throw RageException( "Tried to Unload texture '%s' that wasn't loaded.", sTexturePath );
	
	pTexture = p->second;
	pTexture->m_iRefCount--;
	if( pTexture->m_iRefCount != 0 )
	{
//		LOG->Trace( "RageTextureManager: '%s' will not be deleted.  It still has %d references.", sTexturePath, pTexture->m_iRefCount );
		return;
	}

	// There are no more references to this texture.
//	LOG->Trace( "RageTextureManager: '%s' will be deleted.  It has %d references.", sTexturePath, pTexture->m_iRefCount );
	SAFE_DELETE( pTexture );		// free the texture
	m_mapPathToTexture.erase(p);	// and remove the key in the map
}

void RageTextureManager::ReloadAll()
{
	for( std::map<CString, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;

		// this is not entirely correct.  Hints are lost!
		pTexture->Reload( m_iMaxTextureSize, m_iTextureColorDepth, 0 );
	}
}

void RageTextureManager::SetPrefs( DWORD dwMaxSize, DWORD dwTextureColorDepth )
{
	if( dwMaxSize == m_iMaxTextureSize  &&  dwTextureColorDepth == m_iTextureColorDepth )
		return; 
	ASSERT( m_iMaxTextureSize >= 64 );
	m_iMaxTextureSize = dwMaxSize; 
	m_iTextureColorDepth = dwTextureColorDepth;
	ReloadAll(); 
}
