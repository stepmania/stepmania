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

#include <map>

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
		if( dwMaxSize == m_iMaxTextureSize  &&  dwTextureColorDepth == m_iTextureColorDepth )
			return; 
		ASSERT( m_iMaxTextureSize >= 64 );
		m_iMaxTextureSize = dwMaxSize; 
		m_iTextureColorDepth = dwTextureColorDepth;
		ReloadAll(); 
	};
	DWORD GetMaxTextureSize() { return m_iMaxTextureSize; };
	DWORD GetTextureColorDepth() { return m_iTextureColorDepth; };

protected:
	RageDisplay* m_pScreen;

	DWORD m_iMaxTextureSize;
	DWORD m_iTextureColorDepth;

	// map from file name to a texture holder
	std::map<CString, RageTexture*> m_mapPathToTexture;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program
