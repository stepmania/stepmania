#include "global.h"
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
#include "arch/MovieTexture/MovieTexture.h"
#include "RageUtil.h"
#include "RageLog.h"
#include "RageException.h"
#include <time.h>

RageTextureManager*		TEXTUREMAN		= NULL;

static CString GetExtension(const CString &fn)
{
	CString sDir, sFName, sExt;
	splitpath( fn, sDir, sFName, sExt );
	sExt.MakeLower();

	return sExt;
}


//-----------------------------------------------------------------------------
// constructor/destructor
//-----------------------------------------------------------------------------
RageTextureManager::RageTextureManager()
{
}

RageTextureManager::~RageTextureManager()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		if( pTexture->m_iRefCount )
			LOG->Trace( "TEXTUREMAN LEAK: '%s', RefCount = %d.", i->first.filename.c_str(), pTexture->m_iRefCount );
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

void RageTextureManager::AdjustTextureID(RageTextureID &ID) const
{
	if( ID.iColorDepth == -1 )
		ID.iColorDepth = m_iTextureColorDepth;
	ID.iMaxSize = min( ID.iMaxSize, m_iMaxTextureResolution );
}

/* If you've set up a texture yourself, register it here so it can be referenced
 * and deleted by an ID.  This takes ownership.  They're kept around until the
 * ne*/
bool RageTextureManager::IsTextureRegistered( RageTextureID ID ) const
{
	AdjustTextureID(ID);
	return m_mapPathToTexture.find(ID) != m_mapPathToTexture.end();
}

void RageTextureManager::RegisterTexture( RageTextureID ID, RageTexture *pTexture )
{
	AdjustTextureID(ID);

	/* Make sure we don't already have a texture with this ID.  If we do, the
	 * caller should have used it. */
	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p != m_mapPathToTexture.end())
		/* Oops, found the texture. */
		RageException::Throw("Custom texture \"%s\" already registered!", ID.filename.c_str());

	m_mapPathToTexture[ID] = pTexture;
}

// Load and unload textures from disk.
RageTexture* RageTextureManager::LoadTexture( RageTextureID ID )
{
	Checkpoint( ssprintf( "RageTextureManager::LoadTexture(%s).", ID.filename.c_str() ) );

	AdjustTextureID(ID);

	/* We could have two copies of the same bitmap if there are equivalent but
	 * different paths, e.g. "Bitmaps\me.bmp" and "..\Rage PC Edition\Bitmaps\me.bmp". */
	std::map<RageTextureID, RageTexture*>::iterator p = m_mapPathToTexture.find(ID);
	if(p != m_mapPathToTexture.end())
	{
		/* Found the texture.  Just increase the refcount and return it. */
		RageTexture* pTexture = p->second;
		pTexture->m_iRefCount++;
		LOG->UnmapLog( "LoadTexture" );
		return pTexture;
	}

	// The texture is not already loaded.  Load it.
	const CString sExt = GetExtension(ID.filename);

	RageTexture* pTexture;
	if( sExt == ".avi" || sExt == ".mpg" || sExt == ".mpeg" )
		pTexture = MakeRageMovieTexture( ID );
	else
		pTexture = new RageBitmapTexture( ID );

	m_mapPathToTexture[ID] = pTexture;

	LOG->UnmapLog( "LoadTexture" );
	return pTexture;
}

void RageTextureManager::CacheTexture( RageTextureID ID )
{
	RageTexture* pTexture = LoadTexture( ID );
	pTexture->m_bCacheThis |= true;
	UnloadTexture( pTexture );
}

void RageTextureManager::UnloadTexture( RageTexture *t )
{
	t->m_iRefCount--;
	ASSERT( t->m_iRefCount >= 0 );

	if( t->m_iRefCount )
		return; /* Can't unload textures that are still referenced. */

	bool bDeleteThis = false;

	/* Always unload movies, so we don't waste time decoding.
	 *
	 * Actually, multiple refs to a movie won't work; they should play independently,
	 * but they'll actually share settings.  Not worth fixing, since we don't currently
	 * using movies for anything except BGAs (though we could).
	 */
	if( t->IsAMovie() )
		bDeleteThis = true;

	/* If m_bDelayedDelete, keep all textures around until we GC.  If m_bCacheThis,
	 * keep this individual texture, even if m_bDelayedDelete is off.  (Would
	 * "m_bDelayedDelete" be clearer as "m_bCacheAll"?) */
	if( !m_bDelayedDelete && !t->m_bCacheThis )
		bDeleteThis = true;
	
	if( bDeleteThis )
		DeleteTexture( t );
}

void RageTextureManager::DeleteTexture( RageTexture *t )
{
	ASSERT( t->m_iRefCount==0 );
	LOG->Trace( "RageTextureManager: deleting '%s'.", t->GetID().filename.c_str() );

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); i++ )
	{
		if( i->second == t )
		{
			m_mapPathToTexture.erase( i );	// remove map entry
			SAFE_DELETE( t );	// free the texture
			return;
		}
	}

	ASSERT(0);	// we tried to delete a texture that wasn't loaded.
}

void RageTextureManager::GarbageCollect( GCType type )
{
	// Search for old textures with refcount==0 to unload
	LOG->Trace("Performing texture garbage collection.");

	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); )
	{
		std::map<RageTextureID, RageTexture*>::iterator j = i;
		i++;

		CString sPath = j->first.filename;
		RageTexture* t = j->second;

		if( t->m_iRefCount )
			continue; /* Can't unload textures that are still referenced. */

		bool bDeleteThis = false;
		if( type==cached_textures && !m_bDelayedDelete )
			bDeleteThis = true;
		if( type==delayed_delete )
			bDeleteThis = true;
			
		if( bDeleteThis )
			DeleteTexture( t );
	}
}


void RageTextureManager::ReloadAll()
{
	for( std::map<RageTextureID, RageTexture*>::iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i)
	{
		i->second->Reload();
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
	std::map<RageTextureID, RageTexture*>::iterator i;
	for( i = m_mapPathToTexture.begin(); i != m_mapPathToTexture.end(); ++i)
	{
		RageTexture* pTexture = i->second;
		pTexture->Invalidate();
	}

	/* We're going to have to reload all loaded textures.  Let's get rid
	 * of all unreferenced textures, so we don't reload a ton of cached
	 * data that we're not necessarily going to use. 
	 *
	 * This must be done *after* we Invalidate above, to let the texture
	 * know its OpenGL texture number is invalid.  If we don't do that,
	 * it'll try to free it. */
	for( i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); )
	{
		std::map<RageTextureID, RageTexture*>::iterator j = i;
		i++;

		CString sPath = j->first.filename;
		RageTexture* pTexture = j->second;
		if( pTexture->m_iRefCount==0 )
		{
			SAFE_DELETE( pTexture );		// free the texture
			m_mapPathToTexture.erase(j);	// and remove the key in the map
		}
	}
}

bool RageTextureManager::SetPrefs( int iTextureColorDepth, bool bDelayedDelete, int iMaxTextureResolution )
{
	bool need_reload = false;
	if( m_bDelayedDelete != bDelayedDelete ||
		m_iTextureColorDepth != iTextureColorDepth ||
		m_iMaxTextureResolution != iMaxTextureResolution )
		need_reload = true;

	m_bDelayedDelete = bDelayedDelete;
	m_iTextureColorDepth = iTextureColorDepth;
	m_iMaxTextureResolution = iMaxTextureResolution;
	
	ASSERT( m_iTextureColorDepth==16 || m_iTextureColorDepth==32 );
	return need_reload;
}

void RageTextureManager::DiagnosticOutput() const
{
	unsigned cnt = distance(m_mapPathToTexture.begin(), m_mapPathToTexture.end());
	LOG->Trace("%u textures loaded:", cnt);
	for( std::map<RageTextureID, RageTexture*>::const_iterator i = m_mapPathToTexture.begin();
		i != m_mapPathToTexture.end(); ++i )
	{
		const RageTextureID &ID = i->first;
		const RageTexture *tex = i->second;

		/* This could output much more, but I only need resolution now and I don't
		 * want it to be more than one line. */
		LOG->Trace(" %3ix%3i (%2i) %s",
			tex->GetTextureHeight(), tex->GetTextureWidth(),
			tex->m_iRefCount, Basename(ID.filename).c_str() );		
	}
}
