#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 File: RageTextureManager.cpp

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "RageTextureManager.h"
#include "RageBitmapTexture.h"
#include "RageMovieTexture.h"
#include "RageUtil.h"
#include <assert.h>

LPRageTextureManager	TM		= NULL;

//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
RageTextureManager::RageTextureManager( LPRageScreen pScreen )
{
	assert( pScreen != NULL );
	m_pScreen = pScreen;

}

RageTextureManager::~RageTextureManager()
{
	// delete all textures
	POSITION pos = m_mapPathToTexture.GetStartPosition();
	CString key;
	LPRageTexture value;

	while( pos != NULL )  // iterate over all k/v pairs in map
	{
		m_mapPathToTexture.GetNextAssoc( pos, key, value );
		SAFE_DELETE( value );
	}

	m_mapPathToTexture.RemoveAll();
}


//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
LPRageTexture RageTextureManager::LoadTexture( CString sTexturePath )
{
//	RageLog( "RageTextureManager::LoadTexture(%s).", sTexturePath );

	// holder for the new texture
	LPRageTexture pTexture;

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp" ).
	sTexturePath.MakeLower();

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture already exists in the map
	{
//		RageLog( ssprintf("Found that '%s' is already loaded.  Using the loaded copy.", sTexturePath) );
		pTexture->m_iRefCount++;
	}
	else
	{
		RageLog( ssprintf("RageTextureManager: allocating space for '%s'.", sTexturePath) );
		
		CString sDrive, sDir, sFName, sExt;
		splitpath( FALSE, sTexturePath, sDrive, sDir, sFName, sExt );

		if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
			pTexture = (LPRageTexture) new RageMovieTexture( m_pScreen, sTexturePath );
		else
			pTexture = (LPRageTexture) new RageBitmapTexture( m_pScreen, sTexturePath );

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
//	RageLog( "RageTextureManager::UnloadTexture(%s).", sTexturePath );

	if( sTexturePath == "" )
	{
		RageLog( "RageTextureManager::UnloadTexture(): tried to Unload a blank" );
		return;
	}
	
	if( !IsTextureLoaded( sTexturePath ) )
		RageError( ssprintf("Tried to Unload a texture that wasn't loaded. '%s'", sTexturePath) );

	sTexturePath.MakeLower();
	LPRageTexture pTexture;

	if( m_mapPathToTexture.Lookup( sTexturePath, pTexture ) )	// if the texture exists in the map
	{
		pTexture->m_iRefCount--;
		if( pTexture->m_iRefCount == 0 )		// there are no more references to this texture
		{
			RageLog( ssprintf("RageTextureManager: deallocating '%s'.", sTexturePath) );
			SAFE_DELETE( pTexture );		// free the texture
			m_mapPathToTexture.RemoveKey( sTexturePath );	// and remove the key in the map
		}
//		else
//			RageLog( ssprintf("The texture '%s' has %d more references.  It will not be deleted.", 
//					 sTexturePath, pTexture->m_iRefCount) );

	}
	
}
