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
	// delete all textures
	POSITION pos = m_mapPathToTexture.GetStartPosition();
	CString sPath;
	RageTexture* pTexture;

	while( pos != NULL )  // iterate over all k/v pairs in map
	{
		m_mapPathToTexture.GetNextAssoc( pos, sPath, pTexture );
		LOG->WriteLine( "TEXTUREMAN LEAK: '%s', RefCount = %d.", sPath, pTexture->m_iRefCount );
		SAFE_DELETE( pTexture );
	}

	m_mapPathToTexture.RemoveAll();
}


//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
RageTexture* RageTextureManager::LoadTexture( CString sTexturePath, bool bForceReload, int iMipMaps, int iAlphaBits, bool bDither, bool bStretch )
{
	sTexturePath.MakeLower();

//	LOG->WriteLine( "RageTextureManager::LoadTexture(%s).", sTexturePath );

	// holder for the new texture
	RageTexture* pTexture;

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp" ).

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture already exists in the map
	{
		pTexture->m_iRefCount++;
		if( bForceReload )
			pTexture->Reload( m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither );

		LOG->WriteLine( "RageTextureManager: '%s' now has %d references.", sTexturePath, pTexture->m_iRefCount );
	}
	else	// the texture is not already loaded
	{
		CString sDrive, sDir, sFName, sExt;
		splitpath( false, sTexturePath, sDrive, sDir, sFName, sExt );

		if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
			pTexture = (RageTexture*) new RageMovieTexture( m_pScreen, sTexturePath, m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );
		else
			pTexture = (RageTexture*) new RageBitmapTexture( m_pScreen, sTexturePath, m_iMaxTextureSize, m_iTextureColorDepth, iMipMaps, iAlphaBits, bDither, bStretch );


		LOG->WriteLine( "RageTextureManager: Finished loading '%s' - %d references.", sTexturePath, pTexture->m_iRefCount );


		m_mapPathToTexture.SetAt( sTexturePath, pTexture );
	}

	LOG->WriteLine( "Display: %u MB video memory left",	DISPLAY->GetDevice()->GetAvailableTextureMem() );

	return pTexture;
}


bool RageTextureManager::IsTextureLoaded( CString sTexturePath )
{
	sTexturePath.MakeLower();

	RageTexture* pTexture;

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture exists in the map
		return true;
	else
		return false;
}	

void RageTextureManager::UnloadTexture( CString sTexturePath )
{
	sTexturePath.MakeLower();

//	LOG->WriteLine( "RageTextureManager::UnloadTexture(%s).", sTexturePath );

	if( sTexturePath == "" )
	{
		//LOG->WriteLine( "RageTextureManager::UnloadTexture(): tried to Unload a blank texture." );
		return;
	}
	
	RageTexture* pTexture;

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture exists in the map
	{
		pTexture->m_iRefCount--;
		if( pTexture->m_iRefCount == 0 )		// there are no more references to this texture
		{
			LOG->WriteLine( "RageTextureManager: '%s' will be deleted.  It has %d references.", sTexturePath, pTexture->m_iRefCount );
			SAFE_DELETE( pTexture );		// free the texture
			m_mapPathToTexture.RemoveKey( sTexturePath );	// and remove the key in the map
		}
		else
		{
			LOG->WriteLine( "RageTextureManager: '%s' will not be deleted.  It still has %d references.", sTexturePath, pTexture->m_iRefCount );
		}
	}
	else // texture not found
	{
		throw RageException( "Tried to Unload texture '%s' that wasn't loaded.", sTexturePath );
	}
	
}

void RageTextureManager::ReloadAll()
{
	for( POSITION pos = m_mapPathToTexture.GetStartPosition(); pos != NULL;  )
	{
		CString sPath;
		RageTexture* pTexture;

		m_mapPathToTexture.GetNextAssoc( pos, sPath, pTexture );
		
		pTexture->Reload( m_iMaxTextureSize, m_iTextureColorDepth, 0 );	// this not entirely correct.  Hints are lost!
	}
}