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

	void SetPrefs( int iMaxSize, int iTextureColorDepth, bool bUnloadWhenDone );
	int GetMaxTextureSize() { return m_iMaxTextureSize; };
	int GetTextureColorDepth() { return m_iTextureColorDepth; };

protected:
	RageDisplay* m_pScreen;

	int m_iMaxTextureSize;
	int m_iTextureColorDepth;
	bool  m_bUnloadWhenDone;

	// map from file name to a texture holder
	std::map<CString, RageTexture*> m_mapPathToTexture;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program
