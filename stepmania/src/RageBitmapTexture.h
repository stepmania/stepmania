#ifndef RAGEBITMAPTEXTURE_H
#define RAGEBITMAPTEXTURE_H

/*
-----------------------------------------------------------------------------
 Class: RageBitmapTexture

 Desc: Holder for a static texture with metadata.  Can load just about any image format.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "RageTexture.h"
struct SDL_Surface;

class RageBitmapTexture : public RageTexture
{
public:
	RageBitmapTexture( RageTextureID name );
	virtual ~RageBitmapTexture();
	/* only called by RageTextureManager::InvalidateTextures */
	virtual void Invalidate() { m_uTexHandle = 0; /* don't Destroy() */}
	virtual void Reload();
	virtual unsigned GetTexHandle() { return m_uTexHandle; };	// accessed by RageDisplay

private:
	void Create();	// called by constructor and Reload
	void Destroy();
	unsigned m_uTexHandle;	// treat as unsigned in OpenGL, ID3D8Texture* for D3D

	SDL_Surface *CreateImg(int &pixfmt);
};

#endif
