#ifndef RAGE_TEXTURE_MANAGER_H
#define RAGE_TEXTURE_MANAGER_H

/*
-----------------------------------------------------------------------------
 Class: RageTextureManager

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "RageTexture.h"

#include <map>

class RageTextureManager
{
public:
	RageTextureManager();
	void Update( float fDeltaTime );
	~RageTextureManager();

	RageTexture* LoadTexture( RageTextureID ID );
	bool IsTextureRegistered( RageTextureID ID ) const;
	void RegisterTexture( RageTextureID ID, RageTexture *p );
	void CacheTexture( RageTextureID ID );
	void VolatileTexture( RageTextureID ID );
	void UnloadTexture( RageTexture *t );
	void ReloadAll();

	bool SetPrefs( int iTextureColorDepth, int iMovieColorDepth, bool bDelayedDelete, int iMaxTextureResolution );
	int GetTextureColorDepth() { return m_iTextureColorDepth; };
	int GetMovieColorDepth() { return m_iMovieColorDepth; };
	bool GetDelayedDelete() { return m_bDelayedDelete; };
	int GetMaxTextureResolution() { return m_iMaxTextureResolution; };

	// call this between Screens
	void DeleteCachedTextures()	{ GarbageCollect(screen_changed); }
	
	// call this on switch theme
	void DoDelayedDelete()	{ GarbageCollect(delayed_delete); }
	
	void InvalidateTextures();
	
	void AdjustTextureID(RageTextureID &ID) const;
	void DiagnosticOutput() const;

	void DisableOddDimensionWarning() { m_iNoWarnAboutOddDimensions++; }
	void EnableOddDimensionWarning() { m_iNoWarnAboutOddDimensions--; }
	bool GetOddDimensionWarning() const { return m_iNoWarnAboutOddDimensions == 0; }

protected:
	void DeleteTexture( RageTexture *t );
	enum GCType { screen_changed, delayed_delete };
	void GarbageCollect( GCType type );

	int m_iTextureColorDepth;
	int m_iMovieColorDepth;
	bool m_bDelayedDelete;
	int m_iMaxTextureResolution;

	RageTexture* LoadTextureInternal( RageTextureID ID );

	std::map<RageTextureID, RageTexture*> m_mapPathToTexture;
	int m_iNoWarnAboutOddDimensions;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program

#endif
