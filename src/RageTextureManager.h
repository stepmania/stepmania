/* RageTextureManager - Interface for loading textures. */

#ifndef RAGE_TEXTURE_MANAGER_H
#define RAGE_TEXTURE_MANAGER_H

#include "RageTexture.h"
#include "RageSurface.h"

struct RageTextureManagerPrefs
{
	int m_iTextureColorDepth;
	int m_iMovieColorDepth;
	bool m_bDelayedDelete;
	int m_iMaxTextureResolution;
	bool m_bHighResolutionTextures;
	bool m_bMipMaps;
	
	RageTextureManagerPrefs(): m_iTextureColorDepth(16),
		m_iMovieColorDepth(16), m_bDelayedDelete(false),
		m_iMaxTextureResolution(1024),
		m_bHighResolutionTextures(true), m_bMipMaps(false) {}
	RageTextureManagerPrefs( 
		int iTextureColorDepth,
		int iMovieColorDepth,
		bool bDelayedDelete,
		int iMaxTextureResolution,
		bool bHighResolutionTextures,
		bool bMipMaps ):
		m_iTextureColorDepth(iTextureColorDepth),
		m_iMovieColorDepth(iMovieColorDepth),
		m_bDelayedDelete(bDelayedDelete),
		m_iMaxTextureResolution(iMaxTextureResolution),
		m_bHighResolutionTextures(bHighResolutionTextures),
		m_bMipMaps(bMipMaps) {}

	bool operator!=( const RageTextureManagerPrefs& rhs ) const
	{
		return 
			m_iTextureColorDepth != rhs.m_iTextureColorDepth ||
			m_iMovieColorDepth != rhs.m_iMovieColorDepth ||
			m_bDelayedDelete != rhs.m_bDelayedDelete ||
			m_iMaxTextureResolution != rhs.m_iMaxTextureResolution ||
			m_bHighResolutionTextures != rhs.m_bHighResolutionTextures ||
			m_bMipMaps != rhs.m_bMipMaps;
	}
};

class RageTextureManager
{
public:
	RageTextureManager();
	~RageTextureManager();
	void Update( float fDeltaTime );

	RageTexture* LoadTexture( RageTextureID ID );
	RageTexture* CopyTexture( RageTexture *pCopy ); // returns a ref to the same texture, not a deep copy
	bool IsTextureRegistered( RageTextureID ID ) const;
	void RegisterTexture( RageTextureID ID, RageTexture *p );
	void VolatileTexture( RageTextureID ID );
	void UnloadTexture( RageTexture *t );
	void ReloadAll();

	void RegisterTextureForUpdating(RageTextureID id, RageTexture* tex);

	bool SetPrefs( RageTextureManagerPrefs prefs );
	RageTextureManagerPrefs GetPrefs() { return m_Prefs; };

	RageTextureID::TexPolicy GetDefaultTexturePolicy() const { return m_TexturePolicy; }
	void SetDefaultTexturePolicy( RageTextureID::TexPolicy p ) { m_TexturePolicy = p; }

	// call this between Screens
	void DeleteCachedTextures()	{ GarbageCollect( screen_changed ); }

	// call this on switch theme
	void DoDelayedDelete()	{ GarbageCollect( delayed_delete ); }

	void InvalidateTextures();

	void AdjustTextureID( RageTextureID &ID ) const;
	void DiagnosticOutput() const;

	void DisableOddDimensionWarning() { m_iNoWarnAboutOddDimensions++; }
	void EnableOddDimensionWarning() { m_iNoWarnAboutOddDimensions--; }
	bool GetOddDimensionWarning() const { return m_iNoWarnAboutOddDimensions == 0; }

	RageTextureID GetDefaultTextureID();
	RageTextureID GetScreenTextureID();
	RageSurface* GetScreenSurface();

private:
	void DeleteTexture( RageTexture *t );
	enum GCType { screen_changed, delayed_delete };
	void GarbageCollect( GCType type );
	RageTexture* LoadTextureInternal( RageTextureID ID );

	RageTextureManagerPrefs m_Prefs;
	int m_iNoWarnAboutOddDimensions;
	RageTextureID::TexPolicy m_TexturePolicy;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessible from anywhere in our program

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
 * 
 * (c) 2016- Electromuis, Anton Grootes
 * This branch of https://github.com/stepmania/stepmania
 * will from here on out be released as GPL v3 (wich converts from the previous MIT license)
 */
