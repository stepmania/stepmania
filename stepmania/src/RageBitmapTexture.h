/*
-----------------------------------------------------------------------------
 File: RageBitmapTexture.h

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the persons listed below.  All rights reserved.
-----------------------------------------------------------------------------
*/


class RageBitmapTexture;



#ifndef _RageBitmapTexture_H_
#define _RageBitmapTexture_H_


#include "RageScreen.h"
#include <d3dx8.h>
#include <assert.h>
//#include <d3d8types.h>


const DWORD TEXTURE_HINT_NOMIPMAPS	=	1 << 0;
const DWORD TEXTURE_HINT_DITHER		=	1 << 1;


//-----------------------------------------------------------------------------
// RageBitmapTexture Class Declarations
//-----------------------------------------------------------------------------
class RageBitmapTexture : public RageTexture
{
public:
	RageBitmapTexture( 
		RageScreen* pScreen, 
		const CString &sFilePath, 
		const DWORD dwMaxSize = 2048, 
		const DWORD dwTextureColorDepth = 16, 
		const DWORD dwHints = 0 
		);
	~RageBitmapTexture();

	virtual void Reload( 
		const DWORD dwMaxSize, 
		const DWORD dwTextureColorDepth,
		const DWORD dwHints );

	virtual LPDIRECT3DTEXTURE8 GetD3DTexture();

protected:
	virtual void Create( DWORD dwMaxSize, DWORD dwTextureColorDepth, DWORD dwHints );

	LPDIRECT3DTEXTURE8  m_pd3dTexture;
};


#endif
