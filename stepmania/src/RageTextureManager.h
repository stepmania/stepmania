/*
 * RageTextureManager - Interface for loading and releasing textures.
 */

#ifndef RAGE_TEXTURE_MANAGER_H
#define RAGE_TEXTURE_MANAGER_H

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

	RageTextureID::TexPolicy GetDefaultTexturePolicy() const { return m_TexturePolicy; }
	void SetDefaultTexturePolicy( RageTextureID::TexPolicy p ) { m_TexturePolicy = p; }

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
	RageTextureID::TexPolicy m_TexturePolicy;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program

#endif

/*
 * Copyright (c) 2001-2004 Chris Danford, Glenn Maynard
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
