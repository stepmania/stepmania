#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageTextureManager.cpp

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageTextureManager.h"
#include "RageBitmapTexture.h"
#include "RageMovieTexture.h"
#include "RageUtil.h"
#include "RageHelper.h"
#include "ErrorCatcher/ErrorCatcher.h"

RageTextureManager*		TEXTURE		= NULL;

//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
RageTextureManager::RageTextureManager( RageScreen* pScreen )
{
	assert( pScreen != NULL );
	m_pScreen = pScreen;
	m_dwMaxTextureSize = 2048;	// infinite size
	m_dwTextureColorDepth = 16;
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
		HELPER.Log( "TEXTURE LEAK: '%s', RefCount = %d.", sPath, pTexture->m_iRefCount );
		SAFE_DELETE( pTexture );
	}

	m_mapPathToTexture.RemoveAll();
}


//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
RageTexture* RageTextureManager::LoadTexture( CString sTexturePath, const DWORD dwHints, const bool bForceReload )
{
	sTexturePath.MakeLower();

//	HELPER.Log( "RageTextureManager::LoadTexture(%s).", sTexturePath );

	// holder for the new texture
	RageTexture* pTexture;

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp" ).

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture already exists in the map
	{
		pTexture->m_iRefCount++;
		HELPER.Log( ssprintf("RageTextureManager: '%s' now has %d references.", sTexturePath, pTexture->m_iRefCount) );
	}
	else	// the texture is not already loaded
	{
		CString sDrive, sDir, sFName, sExt;
		splitpath( false, sTexturePath, sDrive, sDir, sFName, sExt );

		if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
			pTexture = (RageTexture*) new RageMovieTexture( m_pScreen, sTexturePath );
		else
			pTexture = (RageTexture*) new RageBitmapTexture( m_pScreen, sTexturePath, m_dwMaxTextureSize, m_dwTextureColorDepth, dwHints );


		HELPER.Log( "RageTextureManager: Loading '%s' (%ux%u) from disk.  It has %d references", 
			sTexturePath, 
			pTexture->GetTextureWidth(), 
			pTexture->GetTextureHeight(),
			pTexture->m_iRefCount 
			);

		m_mapPathToTexture.SetAt( sTexturePath, pTexture );
	}

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

//	HELPER.Log( "RageTextureManager::UnloadTexture(%s).", sTexturePath );

	if( sTexturePath == "" )
	{
		HELPER.Log( "RageTextureManager::UnloadTexture(): tried to Unload a blank texture." );
		return;
	}
	
	RageTexture* pTexture;

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture exists in the map
	{
		pTexture->m_iRefCount--;
		if( pTexture->m_iRefCount == 0 )		// there are no more references to this texture
		{
			HELPER.Log( ssprintf("RageTextureManager: '%s' will be deleted.  It has %d references.", sTexturePath, pTexture->m_iRefCount) );
			SAFE_DELETE( pTexture );		// free the texture
			m_mapPathToTexture.RemoveKey( sTexturePath );	// and remove the key in the map
		}
		else
		{
			HELPER.Log( ssprintf("RageTextureManager: '%s' will not be deleted.  It still has %d references.", sTexturePath, pTexture->m_iRefCount) );
		}
	}
	else // texture not found
	{
		FatalError( ssprintf("Tried to Unload texture '%s' that wasn't loaded.", sTexturePath) );
	}
	
}
