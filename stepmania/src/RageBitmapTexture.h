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
	RageBitmapTexture( const CString &sFilePath );
	virtual ~RageBitmapTexture();

protected:
	virtual void Load( const CString &sFilePath );
	virtual void Reload();

	virtual unsigned int GetGLTextureID();

	unsigned int	m_uGLTextureID;
	CString			m_sFilePath;
};
