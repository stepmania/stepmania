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

//-----------------------------------------------------------------------------
// RageTextureManager Class Declarations
//-----------------------------------------------------------------------------
class RageTextureManager
{
public:
	RageTextureManager( int iTextureColorDepth, int iSecsBeforeUnload, int iMaxTextureResolution );
	void Update( float fDeltaTime );
	~RageTextureManager();

	RageTexture* LoadTexture( RageTextureID ID );
	bool IsTextureLoaded( CString sTexturePath ); // XXX
	void UnloadTexture( RageTextureID ID );
	void UnloadTexture( RageTexture *t );
	void ReloadAll();

	bool SetPrefs( int iTextureColorDepth, int iSecsBeforeUnload, int iMaxTextureResolution );
	int GetTextureColorDepth() { return m_iTextureColorDepth; };
	int GetMaxTextureResolution() { return m_iMaxTextureResolution; };
	int GetSecsBeforeUnload() { return m_iSecondsBeforeUnload; };

	void InvalidateTextures();

protected:
	void GCTextures();

	int m_iTextureColorDepth;
	int m_iSecondsBeforeUnload;
	int m_iMaxTextureResolution;

	std::map<RageTextureID, RageTexture*> m_mapPathToTexture;
};

extern RageTextureManager*	TEXTUREMAN;	// global and accessable from anywhere in our program

#endif
