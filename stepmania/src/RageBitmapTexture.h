/*
-----------------------------------------------------------------------------
 File: RageBitmapTexture.h

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001 Chris Danford.  All rights reserved.
-----------------------------------------------------------------------------
*/


class RageBitmapTexture;
typedef RageBitmapTexture* LPRageBitmapTexture;




#ifndef _RageBitmapTexture_H_
#define _RageBitmapTexture_H_


#include "RageScreen.h"
#include <d3dx8.h>
#include <assert.h>
//#include <d3d8types.h>


//-----------------------------------------------------------------------------
// RageBitmapTexture Class Declarations
//-----------------------------------------------------------------------------
class RageBitmapTexture : public RageTexture
{
public:
	RageBitmapTexture( LPRageScreen pScreen, CString sFilePath );
	~RageBitmapTexture();

	virtual LPDIRECT3DTEXTURE8 GetD3DTexture();

protected:
	virtual VOID Create();

	LPDIRECT3DTEXTURE8  m_pd3dTexture;
};


#endif
