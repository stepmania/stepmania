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
#include "time.h"

RageTextureManager*		TEXTUREMAN		= NULL;

//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
RageTextureManager::RageTextureManager( int iTextureColorDepth, int iSecondsBeforeUnload )
{
	SetPrefs( iTextureColorDepth, iSecondsBeforeUnload );
}

RageTextureManager::~RageTextureManager()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		if( pTexture->m_iRefCount )
			LOG->Trace( "TEXTUREMAN LEAK: '%s', RefCount = %d.", i->first.filename.GetString(), pTexture->m_iRefCount );
		SAFE_DELETE( pTexture );
	}
}

void RageTextureManager::Update( float fDeltaTime )
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		pTexture->Update( fDeltaTime );
	}
}

//-----------------------------------------------------------------------------
// Load/Unload textures from disk
//-----------------------------------------------------------------------------
RageTexture* RageTextureManager::LoadTexture( RageTextureID ID )
{
	ID.filename.MakeLower();

	LOG->Trace( "RageTextureManager::LoadTexture(%s).", ID.filename.GetString() );

	// holder for the new texture
	RageTexture* pTexture;

	// Convert the path to lowercase so that we don't load duplicates.
	// Really, this does not solve the duplicate problem.  We could have to copies
	// of the same bitmap if there are equivalent but different paths
	// (e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp" ).

	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p != m_mapPathToTexture.end()) {
		pTexture = p->second;

		pTexture->m_iRefCount++;

//		LOG->Trace( "RageTextureManager: '%s' now has %d references.", sTexturePath.GetString(), pTexture->m_iRefCount );
	}
	else	// the texture is not already loaded
	{
		CString sDrive, sDir, sFName, sExt;
		splitpath( false, ID.filename, sDrive, sDir, sFName, sExt );

		if( sExt == "avi" || sExt == "mpg" || sExt == "mpeg" )
			pTexture = new RageMovieTexture( ID );
		else
			pTexture = new RageBitmapTexture( ID );

		LOG->Trace( "RageTextureManager: Loaded '%s'.", ID.filename.GetString() );

		m_mapPathToTexture[ID] = pTexture;
	}

//	LOG->Trace( "Display: %.2f KB video memory left",	DISPLAY->GetDevice()->GetAvailableTextureMem()/1000000.0f );

	return pTexture;
}


bool RageTextureManager::IsTextureLoaded( CString sTexturePath )
{
	sTexturePath.MakeLower();

	return m_mapPathToTexture.find(sTexturePath) != m_mapPathToTexture.end();
}	

void RageTextureManager::GCTextures()
{
	/* If m_iSecondsBeforeUnload is 0, then we'll unload textures immediately when
	 * they're no longer needed, to use as little extra memory as possible, so don't
	 * bother checking for expired textures here. */
	if(!m_iSecondsBeforeUnload) return;

	static int timeLastGarbageCollect = time(NULL);
	if( timeLastGarbageCollect+m_iSecondsBeforeUnload/2 > time(NULL) )
		return;

	// Search for old textures with refcount==0 to unload
	LOG->Trace("Performing texture garbage collection");
	timeLastGarbageCollect = time(NULL);

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); )
	{
		std::map<RageTextureID, RageTexture*>::iterator j = i;
		i++;

		CString sPath = j->first.filename;
		RageTexture* pTexture = j->second;
		if( pTexture->m_iRefCount==0  &&
			pTexture->m_iTimeOfLastUnload+m_iSecondsBeforeUnload < time(NULL) )
		{
			SAFE_DELETE( pTexture );		// free the texture
			m_mapPathToTexture.erase(j);	// and remove the key in the map
		}
	}
}


void RageTextureManager::UnloadTexture( RageTextureID ID )
{
	ID.filename.MakeLower();

	if( ID.filename == "" )
	{
		//LOG->Trace( "RageTextureManager::UnloadTexture(): tried to Unload a blank texture." );
		return;
	}
	LOG->Trace( "RageTextureManager::UnloadTexture(%s).", ID.filename.GetString() );

	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p == m_mapPathToTexture.end())
		RageException::Throw( "Tried to Unload texture '%s' that wasn't loaded.", ID.filename.GetString() );
	
	RageTexture* pTexture = p->second;
	UnloadTexture(p->second);
	//LOG->Trace( "RageTextureManager: '%s' will not be deleted.  It still has %d references.", sTexturePath.GetString(), pTexture->m_iRefCount );
}

void RageTextureManager::UnloadTexture( RageTexture *t )
{
	t->m_iRefCount--;
	t->m_iTimeOfLastUnload = time(NULL);
	ASSERT( t->m_iRefCount >= 0 );

	/* Always unload movies, so we don't waste time decoding.
	 *
	 * Actually, multiple refs to a movie won't work; they should play independently,
	 * but they'll actually share settings.  Not worth fixing, since we don't currently
	 * using movies for anything except BGAs (though we could).
	 * 
	 * Also, if texture caching is off, just remove it now instead of doing
	 * garbage collection. */
	if( t->m_iRefCount == 0  && 
		(t->IsAMovie() || !m_iSecondsBeforeUnload ))
	{
		//	LOG->Trace( "RageTextureManager: '%s' will be deleted.  It has %d references.", sTexturePath.GetString(), pTexture->m_iRefCount );
		m_mapPathToTexture.erase(t->GetID());	// and remove the key in the map
		SAFE_DELETE( t );		// free the texture
	}

	GCTextures();
}

void RageTextureManager::ReloadAll()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;

		/* A note on how this really works:
		 *
		 * The ID identifies a texture, and all of the parameters needed
		 * to produce it.  When we load a texture, iColorDepth is pulled
		 * from GetTextureColorDepth().  Now we're reloading it, probably
		 * due to a change in display settings, so we need to update that
		 * to the current setting and reload it.  This will also change
		 * the data in our map (which is OK; it's not part of the ordering)
		 * to reflect this. */

		/* Update the settings that are based on preferences. */
		RageTextureID ID = i->first;
		ID.iColorDepth = TEXTUREMAN->GetTextureColorDepth();

		pTexture->Reload( ID );
	}
}

/* In some cases, changing the display mode will reset the rendering context,
 * releasing all textures.  We don't want to reload immediately if that happens,
 * since we might be changing texture preferences too, which also may have to reload
 * textures.  Instead, tell all textures that their texture ID is invalid, so it
 * doesn't try to free it later when we really do reload (since that ID might be
 * associated with a different texture).  Ack. */
void RageTextureManager::InvalidateTextures()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		pTexture->Invalidate();
	}
}

bool RageTextureManager::SetPrefs( int iTextureColorDepth, int iSecondsBeforeUnload )
{
	bool need_reload = false;
	if(m_iSecondsBeforeUnload != iSecondsBeforeUnload ||
		m_iTextureColorDepth != iTextureColorDepth)
		need_reload = true;

	m_iSecondsBeforeUnload = iSecondsBeforeUnload;
	m_iTextureColorDepth = iTextureColorDepth;
	
	ASSERT( m_iTextureColorDepth==16 || m_iTextureColorDepth==32 );
	return need_reload;
}
