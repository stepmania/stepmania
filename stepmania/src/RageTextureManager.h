/*
-----------------------------------------------------------------------------
 File: RageTextureManager.h

 Desc: Interface for loading and releasing textures.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/

class RageTextureManager;



#ifndef _RAGETEXTUREMANAGER_H_
#define _RAGETEXTUREMANAGER_H_


#include "RageTexture.h"
//#include <d3dx8.h>
//#include <d3d8types.h>


//-----------------------------------------------------------------------------
// RageTextureManager Class Declarations
//-----------------------------------------------------------------------------
class RageTextureManager
{
public:
	RageTextureManager( RageScreen* pScreen );
	~RageTextureManager();

	RageTexture* LoadTexture( CString sTexturePath, const DWORD dwHints = 0, const bool bForceReload = false );
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
	RageScreen* m_pScreen;

	DWORD m_dwMaxTextureSize;
	DWORD m_dwTextureColorDepth;

	// map from file name to a texture holder
	CTypedPtrMap<CMapStringToPtr, CString, RageTexture*> m_mapPathToTexture;
};

extern RageTextureManager*	TEXTURE;	// global and accessable from anywhere in our program

#endif
