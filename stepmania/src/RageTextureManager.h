#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageTextureManager

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/
#include "RageTexture.h"


//-----------------------------------------------------------------------------
// RageTextureManager Class Declarations
//-----------------------------------------------------------------------------
class RageTextureManager
{
public:
	RageTextureManager( RageDisplay* pScreen );
	~RageTextureManager();

	RageTexture* LoadTexture( CString sTexturePath, bool bForceReload = false, int iMipMaps = 4, int iAlphaBits = 4, bool bDither = false, bool bStretch = false );
	bool IsTextureLoaded( CString sTexturePath );
	void UnloadTexture( CString sTexturePath );
	void ReloadAll();

	void SetPrefs( const DWORD dwMaxSize, const DWORD dwTextureColorDepth )
	{
		ASSERT( m_dwMaxTextureSize >= 64 );
		m_dwMaxTextureSize = dwMaxSize; 
		m_dwTextureColorDepth = dwTextureColorDepth;
		ReloadAll(); 
	};
	DWORD GetMaxTextureSize() { return m_dwMaxTextureSize; };
	DWORD GetTextureColorDepth() { return m_dwTextureColorDepth; };

protected:
	RageDisplay* m_pScreen;

	DWORD m_dwMaxTextureSize;
	DWORD m_dwTextureColorDepth;

	// map from file name to a texture holder
	CTypedPtrMap<CMapStringToPtr, CString, RageTexture*> m_mapPathToTexture;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program
