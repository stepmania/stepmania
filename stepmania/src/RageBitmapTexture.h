#pragma once
/*
-----------------------------------------------------------------------------
 Class: RageBitmapTexture

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageDisplay.h"
#include "RageTexture.h"
#include <d3dx8.h>
#include <assert.h>


//-----------------------------------------------------------------------------
// RageBitmapTexture Class Declarations
//-----------------------------------------------------------------------------
class RageBitmapTexture : public RageTexture
{
public:
	RageBitmapTexture( 
		RageDisplay* pScreen, 
		const CString &sFilePath, 
		DWORD dwMaxSize = 2048, 
		DWORD dwTextureColorDepth = 16, 
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false, 
		bool bStretch = false 
		);
	~RageBitmapTexture();

	virtual void Reload( 
		DWORD dwMaxSize, 
		DWORD dwTextureColorDepth,
		int iMipMaps = 4,
		int iAlphaBits = 4,
		bool bDither = false,
		bool bStretch = false
		);
	virtual LPDIRECT3DTEXTURE8 GetD3DTexture();

	virtual bool IsAMovie() { return true; };

protected:

	virtual void Create( 
		DWORD dwMaxSize, 
		DWORD dwTextureColorDepth, 
		int iMipMaps,
		int iAlphaBits,
		bool bDither,
		bool bStretch
		);


	LPDIRECT3DTEXTURE8  m_pd3dTexture;
};
